/**
 * \file        Simulation.cpp
 * \date        Dec 15, 2010
 * \version     v0.8.1
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * The Simulation class represents a simulation of pedestrians
 * based on a certain model in a specific scenario. A simulation is defined by
 * various parameters and functions.
 *
 *
 **/

#include "Simulation.h"

#include "IO/Trajectories.h"
#include "general/Filesystem.h"
#include "general/Logger.h"
#include "general/OpenMP.h"
#include "geometry/GoalManager.h"
#include "geometry/WaitingArea.h"
#include "geometry/Wall.h"
#include "math/GCFMModel.h"
#include "pedestrian/AgentsQueue.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "routing/ff_router/ffRouter.h"

// TODO: add these variables to class simulation
std::map<std::string, std::shared_ptr<TrainType>> TrainTypes;
std::map<int, std::shared_ptr<TrainTimeTable>> TrainTimeTables;
std::map<int, double> trainOutflow;

Simulation::Simulation(Configuration * args) : _config(args)
{
    _countTraj               = 0;
    _nPeds                   = 0;
    _seed                    = 8091983;
    _deltaT                  = 0;
    _building                = nullptr;
    _operationalModel        = nullptr;
    _fps                     = 1;
    _em                      = nullptr;
    _gotSources              = false;
    _fps                     = 1;
    _em                      = nullptr;
    _gotSources              = false;
    _trainConstraints        = false;
    _maxSimTime              = 100;
    _currentTrajectoriesFile = _config->GetTrajectoriesFile();
}

Simulation::~Simulation()
{
    delete _em;

    if(_iod) {
        _iod.reset();
    }
}

long Simulation::GetPedsNumber() const
{
    return _nPeds;
}

bool Simulation::InitArgs()
{
    if(!_config->GetTrajectoriesFile().empty()) {
        switch(_config->GetFileFormat()) {
            case FileFormat::XML:
                _iod = std::make_unique<TrajectoriesXML>(TrajectoriesXML());
                break;

            case FileFormat::TXT:
                _iod = std::make_unique<TrajectoriesTXT>(TrajectoriesTXT());
                break;

            default:
                break;
        }
    }

    const fs::path & trajPath(_config->GetTrajectoriesFile());
    const fs::path trajParentPath = trajPath.parent_path();
    if(!trajParentPath.empty()) {
        fs::create_directories(trajParentPath);
    }
    auto file = std::make_shared<FileHandler>(trajPath.c_str());
    _iod->SetOutputHandler(file);
    _iod->SetOptionalOutput(_config->GetOptionalOutputOptions());


    _operationalModel = _config->GetModel();
    _deltaT           = _config->Getdt();
    _maxSimTime       = _config->GetTmax();
    _periodic         = _config->IsPeriodic();
    _fps              = _config->GetFps();

    _routingEngine   = _config->GetRoutingEngine();
    auto distributor = std::make_unique<PedDistributor>(PedDistributor(_config));
    // IMPORTANT: do not change the order in the following..
    _building = std::shared_ptr<Building>(new Building(_config, *distributor));

    // Initialize the agents sources that have been collected in the pedestrians distributor
    _agentSrcManager.SetBuilding(_building.get());
    _agentSrcManager.SetMaxSimTime(GetMaxSimTime());
    _gotSources =
        !distributor->GetAgentsSources().empty(); // did we have any sources? false if no sources

    for(const auto & src : distributor->GetAgentsSources()) {
        _agentSrcManager.AddSource(src);
        src->Dump();
    }

    //perform customs initialisation, like computing the phi for the gcfm
    //this should be called after the routing engine has been initialised
    // because a direction is needed for this initialisation.
    LOG_INFO("Init Operational Model starting ...");
    if(!_operationalModel->Init(_building.get())) {
        return false;
    }
    LOG_INFO("Init Operational Model done.");
    LOG_INFO("Got {} Train Types.", _building->GetTrainTypes().size());

    for(auto && TT : _building->GetTrainTypes()) {
        LOG_INFO("Type {}", TT.second->type);
        LOG_INFO("Max {}", TT.second->nmax);
        LOG_INFO("Number of doors {}", TT.second->doors.size());
    }

    LOG_INFO("Got {} Train Time Tables", _building->GetTrainTimeTables().size());

    for(auto && TT : _building->GetTrainTimeTables()) {
        LOG_INFO("id           : {}", TT.second->id);
        LOG_INFO("type         : {}", TT.second->type.c_str());
        LOG_INFO("room id      : {}", TT.second->rid);
        LOG_INFO("tin          : {:.2f}", TT.second->tin);
        LOG_INFO("tout         : {:.2f}", TT.second->tout);
        LOG_INFO("track start  : ({:.2f}, {:.2f})", TT.second->pstart._x, TT.second->pstart._y);
        LOG_INFO("track end    : ({:.2f}, {:.2f})", TT.second->pend._x, TT.second->pend._y);
        LOG_INFO("train start  : ({:.2f}, {:.2f})", TT.second->tstart._x, TT.second->tstart._y);
        LOG_INFO("train end    : ({:.2f}, {:.2f})", TT.second->tend._x, TT.second->tend._y);
    }

    //TODO: these variables are global
    TrainTypes        = _building->GetTrainTypes();
    TrainTimeTables   = _building->GetTrainTimeTables();
    _trainConstraints = !TrainTimeTables.empty();

    // Give the DirectionStrategy the chance to perform some initialization.
    // This should be done after the initialization of the operationalModel
    // because then, invalid pedestrians have been deleted and FindExit()
    // has been called.

    //other initializations
    for(auto && ped : _building->GetAllPedestrians()) {
        ped->Setdt(_deltaT);
    }
    _nPeds = _building->GetAllPedestrians().size();
    LOG_INFO("Number of peds received: {}", _nPeds);
    _seed = _config->GetSeed();

    if(_config->GetDistEffMaxPed() > _config->GetLinkedCellSize()) {
        LOG_ERROR(
            "The linked-cell size [{}] should be larger than the force range [{}]",
            _config->GetLinkedCellSize(),
            _config->GetDistEffMaxPed());
        return false;
    }

    _em = new EventManager(_config, _building.get(), _config->GetSeed());
    if(!_em->ReadEventsXml()) {
        LOG_WARNING("Could not initialize events handling");
    }
    if(!_em->ReadSchedule()) {
        LOG_WARNING("Could not initialize schedule handling");
    }

    _em->ListEvents();

    _goalManager.SetBuilding(_building.get());
    return true;
}

double Simulation::RunStandardSimulation(double maxSimTime)
{
    RunHeader(_nPeds + _agentSrcManager.GetMaxAgentNumber());
    double t = RunBody(maxSimTime);
    RunFooter();
    return t;
}

void Simulation::UpdateRoutesAndLocations()
{
    //pedestrians to be deleted
    //you should better create this in the constructor and allocate it once.
    std::set<Pedestrian *> pedsToRemove;

    // collect all pedestrians in the simulation.
    const std::vector<Pedestrian *> & allPeds = _building->GetAllPedestrians();
    const std::map<int, Goal *> & goals       = _building->GetAllGoals();
    auto allRooms                             = _building->GetAllRooms();

#pragma omp parallel for shared(pedsToRemove, allRooms)
    for(size_t p = 0; p < allPeds.size(); ++p) {
        auto ped       = allPeds[p];
        Room * room    = _building->GetRoom(ped->GetRoomID());
        SubRoom * sub0 = room->GetSubRoom(ped->GetSubRoomID());

        //set the new room if needed
        if((ped->GetFinalDestination() == FINAL_DEST_OUT) &&
           (room->GetCaption() == "outside")) { //TODO Hier aendern fuer inside goals?
#pragma omp critical(Simulation_Update_pedsToRemove)
            pedsToRemove.insert(ped);
        } else if(
            (ped->GetFinalDestination() != FINAL_DEST_OUT) &&
            (goals.at(ped->GetFinalDestination())->Contains(ped->GetPos())) &&
            (goals.at(ped->GetFinalDestination())->GetIsFinalGoal())) {
#pragma omp critical(Simulation_Update_pedsToRemove)
            pedsToRemove.insert(ped);
        }

        // reposition in the case the pedestrians "accidentally left the room" not via the intended exit.
        // That may happen if the forces are too high for instance
        // the ped is removed from the simulation, if it could not be reassigned
        else if(!sub0->IsInSubRoom(ped)) {
            bool assigned = false;
            std::function<void(const Pedestrian &)> f =
                std::bind(&Simulation::UpdateFlowAtDoors, this, std::placeholders::_1);
            assigned = ped->Relocate(f);
            //this will delete agents, that are pushed outside (maybe even if inside obstacles??)

            if(!assigned) {
#pragma omp critical(Simulation_Update_pedsToRemove)
                pedsToRemove.insert(ped); //the agent left the old room
                                          //actualize the eggress time for that room
#pragma omp critical(SetEgressTime)
                allRooms.at(ped->GetRoomID())->SetEgressTime(Pedestrian::GetGlobalTime());
            }
        }

        // set ped waiting, if no target is found
        int target = ped->FindRoute();

        if(target == -1) {
            ped->StartWaiting();
        } else {
            if(ped->IsWaiting() && !ped->IsInsideWaitingAreaWaiting()) {
                ped->EndWaiting();
            }
        }

        // actualize routes for sources
        if(_gotSources)
            target = ped->FindRoute();
        //finally actualize the route
        if(!_gotSources && ped->FindRoute() == -1 && !_trainConstraints && !ped->IsWaiting()) {
            //a destination could not be found for that pedestrian
            LOG_ERROR(
                "Could not find a route for pedestrian {} in room {} and subroom {}",
                ped->GetID(),
                ped->GetRoomID(),
                ped->GetSubRoomID());
            std::function<void(const Pedestrian &)> f =
                std::bind(&Simulation::UpdateFlowAtDoors, this, std::placeholders::_1);
            ped->Relocate(f);
#pragma omp critical(Simulation_Update_pedsToRemove)
            {
                pedsToRemove.insert(ped);
                // TODO KKZ track deleted peds
            }
        }

        // Set pedestrian waiting when find route temp_close
        int goal = ped->FindRoute();
        if(goal != FINAL_DEST_OUT) {
            const Hline * target = _building->GetTransOrCrossByUID(goal);
            int roomID           = ped->GetRoomID();
            int subRoomID        = ped->GetSubRoomID();

            if(auto cross = dynamic_cast<const Crossing *>(target)) {
                if(cross->IsInRoom(roomID) && cross->IsInSubRoom(subRoomID)) {
                    if(!ped->IsWaiting() && cross->IsTempClose()) {
                        ped->StartWaiting();
                    }

                    if(ped->IsWaiting() && cross->IsOpen() && !ped->IsInsideWaitingAreaWaiting()) {
                        ped->EndWaiting();
                    }
                }
            }
        }

        // Get new goal for pedestrians who are inside waiting area and wait time is over
        // Check if current position is already waiting area
        // yes: set next goal and return findExit(p)
        _goalManager.ProcessPedPosition(ped);
    }


    _goalManager.ProcessWaitingAreas(Pedestrian::GetGlobalTime());

#ifdef _USE_PROTOCOL_BUFFER
    if(_hybridSimManager) {
        AgentsQueueOut::Add(pedsToRemove); //this should be critical region (and it is)
    } else
#endif
    {
        // remove the pedestrians that have left the building
        for(auto p : pedsToRemove) {
            UpdateFlowAtDoors(*p);
            _building->DeletePedestrian(p);
        }
        pedsToRemove.clear();
    }
}

void Simulation::PrintStatistics(double simTime)
{
    LOG_INFO("Rooms Egress. Simulation Time: {:.2f}", simTime);
    LOG_INFO("id\tcaption\tegress time (s)");

    for(const auto & it : _building->GetAllRooms()) {
        auto && room = it.second;
        if(room->GetCaption() != "outside")
            LOG_INFO("{:d}\t{}\t{:.2f}", room->GetID(), room->GetCaption(), room->GetEgressTime());
    }

    LOG_INFO("Usage of Exits");
    for(const auto & itr : _building->GetAllTransitions()) {
        Transition * goal = itr.second;
        if(goal->GetDoorUsage()) {
            LOG_INFO(
                "Exit ID [{}] used by [{}] pedestrians. Last passing time [{:.2f}] s",
                goal->GetID(),
                goal->GetDoorUsage(),
                goal->GetLastPassingTime());

            fs::path statsfile{"flow_exit_id_" + std::to_string(goal->GetID()) + "_"};
            if(goal->GetOutflowRate() < std::numeric_limits<double>::max()) {
                statsfile += "rate_";
                std::stringstream buffer;
                buffer << std::setprecision(2) << std::fixed << goal->GetOutflowRate();
                statsfile += buffer.str();
                statsfile += '_';
            }
            statsfile += _config->GetOriginalTrajectoriesFile().filename().replace_extension("txt");
            LOG_INFO("More Information in the file: {}", statsfile.string());
            {
                FileHandler statOutput(statsfile);
                statOutput.Write("#Simulation time: %.2f", simTime);
                statOutput.Write(
                    "#Flow at exit " + goal->GetCaption() + "( ID " +
                    std::to_string(goal->GetID()) + " )");
                statOutput.Write("#Time (s)  cummulative number of agents \n");
                statOutput.Write(goal->GetFlowCurve());
            }
        }
    }

    LOG_INFO("Usage of Crossings");
    for(const auto & itr : _building->GetAllCrossings()) {
        Crossing * goal = itr.second;
        if(goal->GetDoorUsage()) {
            LOG_INFO(
                "Crossing ID [{}] in Room ID [{}] used by [{}] pedestrians. Last passing "
                "time [{:.2f}] s",
                goal->GetID(),
                itr.first / 1000,
                goal->GetDoorUsage(),
                goal->GetLastPassingTime());

            fs::path statsfile = "flow_crossing_id_" + std::to_string(itr.first / 1000) + "_" +
                                 std::to_string(itr.first % 1000) + ".dat";
            LOG_INFO("More Information in the file: {}", statsfile.string());
            FileHandler output(statsfile);
            output.Write("#Simulation time: %.2f", simTime);
            output.Write(
                "#Flow at crossing " + goal->GetCaption() + "( ID " +
                std::to_string(goal->GetID()) + " ) in Room ( ID " +
                std::to_string(itr.first / 1000) + " )");
            output.Write("#Time (s)  cummulative number of agents \n");
            output.Write(goal->GetFlowCurve());
        }
    }
}

void Simulation::RunHeader(long nPed)
{
    // writing the header
    if(nPed == -1)
        nPed = _nPeds;
    _iod->WriteHeader(nPed, _fps, _building.get(), _seed, 0); // first trajectory
                                                              // count = 0
    _iod->WriteGeometry(_building.get());
    if(_gotSources)
        _iod->WriteSources(GetAgentSrcManager().GetSources());

    int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
    writeInterval     = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
    int firstframe    = (Pedestrian::GetGlobalTime() / _deltaT) / writeInterval;

    _iod->WriteFrame(firstframe, _building.get());
    //first initialisation needed by the linked-cells
    UpdateRoutesAndLocations();
    ProcessAgentsQueue();
}

double Simulation::RunBody(double maxSimTime)
{
    double t = Pedestrian::GetGlobalTime();

    //frame number. This function can be called many times,
    static int frameNr = (int) (1 + t / _deltaT); // Frame Number

    //##########
    //PROBLEMATIC: time when frame should be printed out
    // possibly skipped when using the following lines
    // TODO NEEDS TO BE FIXED!
    int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
    writeInterval     = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
    // ##########

    //process the queue for incoming pedestrians
    //important since the number of peds is used
    //to break the main simulation loop
    ProcessAgentsQueue();
    _nPeds = _building->GetAllPedestrians().size();
    std::cout << "\n";
    std::string description = "Evacutation ";
    int initialnPeds        = _nPeds;
    // main program loop
    while((_nPeds || (!_agentSrcManager.IsCompleted() && _gotSources)) && t < maxSimTime) {
        t = 0 + (frameNr - 1) * _deltaT;
        // Handle train traffic: coorect geometry
        bool geometryChanged = TrainTraffic();

        //process the queue for incoming pedestrians
        ProcessAgentsQueue();

        if(t > Pedestrian::GetMinPremovementTime()) {
            //update the linked cells
            _building->UpdateGrid();

            // update the positions
            _operationalModel->ComputeNextTimeStep(t, _deltaT, _building.get(), _periodic);

            //update the events
            _em->ProcessEvent();

            //here we could place router-tasks (calc new maps) that can use multiple cores AND we have 't'
            //update quickestRouter
            if(geometryChanged) {
                // debug
                const std::string prefix = "tmp_" + std::to_string(t) + "_";
                auto changedGeometryFile =
                    add_prefix_to_filename(prefix, _config->GetGeometryFile());
                std::cout << "\nUpdate geometry. New  geometry --> " << changedGeometryFile << "\n";

                std::cout << "Enter correctGeometry: Building Has "
                          << _building->GetAllTransitions().size() << " Transitions\n";
                _building->SaveGeometry(changedGeometryFile);
                _building->GetConfig()->GetDirectionManager()->GetDirectionStrategy()->Init(
                    _building.get());
            } else { // quickest needs update even if NeedsUpdate() is false
                auto * ffrouter =
                    dynamic_cast<FFRouter *>(_routingEngine->GetRouter(ROUTING_FF_QUICKEST));
                if(ffrouter != nullptr)
                    if(ffrouter->MustReInit()) {
                        ffrouter->ReInit();
                        ffrouter->SetRecalc(t);
                    }
            }

            // here the used routers are update, when needed due to external changes
            if(_routingEngine->NeedsUpdate()) {
                LOG_INFO("Update router during simulation.");
                _routingEngine->UpdateRouter();
            }

            //update the routes and locations
            UpdateRoutesAndLocations();

            //other updates
            //someone might have left the building
            _nPeds = _building->GetAllPedestrians().size();
        }

        // update the global time
        Pedestrian::SetGlobalTime(t);

        // write the trajectories
        if(0 == frameNr % writeInterval) {
            _iod->WriteFrame(frameNr / writeInterval, _building.get());
            RotateOutputFile();
        }

        if((!_gotSources) &&
           ((frameNr < 100 && frameNr % 10 == 0) || (frameNr > 100 && frameNr % 100 == 0))) {
            LOG_INFO(
                "time: {:6.2f} ({:4.0f}) | Agents: {:6d} / {:d} [{:4.1f}%]",
                t,
                maxSimTime,
                _nPeds,
                initialnPeds,
                (double) (initialnPeds - _nPeds) / initialnPeds * 100.);
        }

        ++frameNr;

        //Trigger JPSfire Toxicity Analysis
        //only executed every 3 seconds
        if(fmod(Pedestrian::GetGlobalTime(), 3) == 0) {
            for(auto && ped : _building->GetAllPedestrians()) {
                ped->ConductToxicityAnalysis();
            }
        }

        //init train trainOutfloww
        for(auto tab : TrainTimeTables) {
            trainOutflow[tab.first] = 0;
        }
        // regulate flow
        for(auto & itr : _building->GetAllTransitions()) {
            Transition * Trans = itr.second;

            Trans->UpdateTemporaryState(_deltaT);
            // regulate train doorusage
            std::string transType = Trans->GetType();
            if(Trans->IsOpen() && transType.rfind("Train", 0) == 0) {
                std::vector<std::string> strs;
                boost::split(strs, transType, boost::is_any_of("_"), boost::token_compress_on);
                int id           = atoi(strs[1].c_str());
                std::string type = Trans->GetCaption();
                trainOutflow[id] += Trans->GetDoorUsage();
                if(trainOutflow[id] >= TrainTypes[type]->nmax) {
                    std::cout << "INFO:\tclosing train door " << transType.c_str() << " at "
                              << Pedestrian::GetGlobalTime() << " capacity "
                              << TrainTypes[type]->nmax << "\n";
                    LOG_INFO(
                        "Closing train door {} at t={:.2f}. Flow = {:.2f} (Train Capacity {})",
                        transType,
                        Pedestrian::GetGlobalTime(),
                        trainOutflow[id],
                        TrainTypes[type]->nmax);
                    Trans->Close();
                }
            }
        } // Transitions
        if(frameNr % 1000 == 0) {
            if(_config->ShowStatistics()) {
                LOG_INFO("Update door statistics at t={:.2f}", t);
                PrintStatistics(t);
            }
        }
    } // while time
    return t;
}

void Simulation::RotateOutputFile()
{
    // FIXME ??????
    if(_config->GetFileFormat() != FileFormat::TXT) {
        return;
    }
    static const fs::path & p       = _config->GetTrajectoriesFile();
    static const fs::path stem      = p.stem();
    static const fs::path extension = p.extension();
    static const fs::path parent    = p.parent_path();

    if(fs::file_size(_currentTrajectoriesFile) > _maxFileSize) {
        incrementCountTraj();
        _currentTrajectoriesFile =
            parent / fs::path(fmt::format(
                         FMT_STRING("{}_{}{}"), stem.string(), _countTraj, extension.string()));
        LOG_INFO("New trajectory file <{}>", _currentTrajectoriesFile.string());
        auto file = std::make_shared<FileHandler>(_currentTrajectoriesFile);
        _iod->SetOutputHandler(file);
        _iod->WriteHeader(_nPeds, _fps, _building.get(), _seed, _countTraj);
    }
}

//      |             |
//      *-------------* <---door
//      |             |
//      |             |
//      |             |
//      |             |
//      |             |
//*-----x-------------x--------* <- wall
//      |             |


/*
 * in this function this happens:
* remove walls
* add walls
* add doors
* set _routingEngine->setNeedUpdate(true);
*/
bool Simulation::correctGeometry(
    std::shared_ptr<Building> building,
    std::shared_ptr<TrainTimeTable> tab)
{
    int trainId           = tab->id;
    std::string trainType = tab->type;
    Point TrackStart      = tab->pstart;
    Point TrainStart      = tab->tstart;
    Point TrackEnd        = tab->pend;
    SubRoom * subroom;
    int room_id, subroom_id;
    auto mytrack = building->GetTrackWalls(TrackStart, TrackEnd, room_id, subroom_id);
    Room * room  = building->GetRoom(room_id);
    subroom      = room->GetSubRoom(subroom_id);
    if(subroom == nullptr) {
        LOG_ERROR(
            "Simulation::correctGeometry got wrong room_id|subroom_id ({}|{}). TrainId {}",
            room_id,
            subroom_id,
            trainId);
        exit(EXIT_FAILURE);
    }
    static int transition_id = 10000; // randomly high number

    std::cout << "enter with train " << trainType.c_str() << "\n";
    std::cout << "Enter correctGeometry: Building Has " << building->GetAllTransitions().size()
              << " Transitions\n";
    std::cout << "room: " << room_id << " subroom_id " << subroom_id << "\n";

    if(mytrack.empty() || subroom == nullptr)
        return false;


    auto train = building->GetTrainTypes().at(trainType);
    auto doors = train->doors;
    for(auto && d : doors) {
        auto newX = d.GetPoint1()._x + TrainStart._x + TrackStart._x;
        auto newY = d.GetPoint1()._y + TrainStart._y + TrackStart._y;
        d.SetPoint1(Point(newX, newY));
        newX = d.GetPoint2()._x + TrainStart._x + TrackStart._x;
        newY = d.GetPoint2()._y + TrainStart._y + TrackStart._y;
        d.SetPoint2(Point(newX, newY));
    }
    for(auto d : doors) {
        LOG_INFO(
            "Train {} {}. Transformed coordinates of doors: {} -- {}",
            trainType,
            trainId,
            d.GetPoint1().toString(),
            d.GetPoint2().toString());
    }

    auto pws = building->GetIntersectionPoints(doors, mytrack);
    if(pws.empty())
        std::cout << "simulation::correctGeometry: pws are empty. Something went south with train "
                     "doors\n";

    auto walls = subroom->GetAllWalls();
    //---
    for(auto pw : pws) {
        auto pw1 = pw.first;
        auto pw2 = pw.second;
        auto p1  = pw1.first;
        auto w1  = pw1.second;
        auto p2  = pw2.first;
        auto w2  = pw2.second;
        // case 1
        Point P;
        if(w1 == w2) {
            std::cout << "EQUAL\n";
            Transition * e = new Transition();
            e->SetID(transition_id++);
            e->SetCaption(trainType);
            e->SetPoint1(p1);
            e->SetPoint2(p2);
            std::string transType = "Train_" + std::to_string(tab->id) + "_" +
                                    std::to_string(tab->tin) + "_" + std::to_string(tab->tout);
            e->SetType(transType);
            room->AddTransitionID(e->GetUniqueID()); // danger area
            e->SetRoom1(room);
            e->SetSubRoom1(subroom);
            subroom->AddTransition(e);  // danger area
            building->AddTransition(e); // danger area

            double dist_pt1 = (w1.GetPoint1() - e->GetPoint1()).NormSquare();
            double dist_pt2 = (w1.GetPoint1() - e->GetPoint2()).NormSquare();
            Point A, B;

            if(dist_pt1 < dist_pt2) {
                A = e->GetPoint1();
                B = e->GetPoint2();
            } else {
                A = e->GetPoint2();
                B = e->GetPoint1();
            }

            Wall NewWall(w1.GetPoint1(), A);
            Wall NewWall1(w1.GetPoint2(), B);
            NewWall.SetType(w1.GetType());
            NewWall1.SetType(w1.GetType());

            // add new lines to be controled against overlap with exits
            if(NewWall.GetLength() > J_EPS_DIST) {
                building->TempAddedWalls[trainId].push_back(NewWall);
                subroom->AddWall(NewWall);
            }

            else
                std::cout << ">> WALL did not add: " << NewWall.toString() << "\n";

            if(NewWall1.GetLength() > J_EPS_DIST) {
                building->TempAddedWalls[trainId].push_back(NewWall1);
                subroom->AddWall(NewWall1);
            } else
                std::cout << ">> WALL did not add: " << NewWall1.toString() << "\n";

            building->TempAddedDoors[trainId].push_back(*e);
            building->TempRemovedWalls[trainId].push_back(w1);
            subroom->RemoveWall(w1);

        } else if(w1.ShareCommonPointWith(w2, P)) {
            std::cout << "ONE POINT COMON\n";
            //------------ transition --------
            Transition * e = new Transition();
            e->SetID(transition_id++);
            e->SetCaption(trainType);
            e->SetPoint1(p1);
            e->SetPoint2(p2);
            std::string transType = "Train_" + std::to_string(tab->id) + "_" +
                                    std::to_string(tab->tin) + "_" + std::to_string(tab->tout);
            e->SetType(transType);
            room->AddTransitionID(e->GetUniqueID()); // danger area
            e->SetRoom1(room);
            e->SetSubRoom1(subroom);

            subroom->AddTransition(e);  // danger area
            building->AddTransition(e); // danger area
            //--------------------------------
            Point N, M;
            if(w1.GetPoint1() == P)
                N = w1.GetPoint2();
            else
                N = w1.GetPoint1();

            if(w2.GetPoint1() == P)
                M = w2.GetPoint2();
            else
                M = w2.GetPoint1();

            Wall NewWall(N, p1);
            Wall NewWall1(M, p2);
            NewWall.SetType(w1.GetType());
            NewWall1.SetType(w2.GetType());
            // changes to building
            building->TempAddedWalls[trainId].push_back(NewWall);
            building->TempAddedWalls[trainId].push_back(NewWall1);
            building->TempAddedDoors[trainId].push_back(*e);
            building->TempRemovedWalls[trainId].push_back(w1);
            building->TempRemovedWalls[trainId].push_back(w2);
            subroom->AddWall(NewWall);
            subroom->AddWall(NewWall1);
            subroom->RemoveWall(w1);
            subroom->RemoveWall(w2);
        } else // disjoint
        {
            std::cout << "DISJOINT\n";
            //------------ transition --------
            Transition * e = new Transition();
            e->SetID(transition_id++);
            e->SetCaption(trainType);
            e->SetPoint1(p1);
            e->SetPoint2(p2);
            std::string transType = "Train_" + std::to_string(tab->id) + "_" +
                                    std::to_string(tab->tin) + "_" + std::to_string(tab->tout);
            e->SetType(transType);
            room->AddTransitionID(e->GetUniqueID()); // danger area
            e->SetRoom1(room);
            e->SetSubRoom1(subroom);

            subroom->AddTransition(e);  // danger area
            building->AddTransition(e); // danger area
            //--------------------------------
            // find points on w1 and w2 between p1 and p2
            // (A, B)
            Point A, B;
            if(e->isBetween(w1.GetPoint1()))
                A = w1.GetPoint2();
            else
                A = w1.GetPoint1();

            if(e->isBetween(w2.GetPoint1()))
                B = w2.GetPoint2();
            else
                B = w2.GetPoint1();

            Wall NewWall(A, p1);
            Wall NewWall1(B, p2);
            NewWall.SetType(w1.GetType());
            NewWall1.SetType(w2.GetType());
            // remove walls between
            for(auto wall : mytrack) {
                if(e->isBetween(wall.GetPoint1()) || e->isBetween(wall.GetPoint2())) {
                    building->TempRemovedWalls[trainId].push_back(wall);
                    subroom->RemoveWall(wall);
                }
            }
            // changes to building
            building->TempAddedWalls[trainId].push_back(NewWall);
            building->TempAddedWalls[trainId].push_back(NewWall1);
            building->TempAddedDoors[trainId].push_back(*e);
            subroom->AddWall(NewWall);
            subroom->AddWall(NewWall1);

            // remove walls w1 and w2
        }
    }
    _routingEngine->setNeedUpdate(true);
    return true;
}

void Simulation::RunFooter()
{
    _iod->WriteFooter();
}

void Simulation::ProcessAgentsQueue()
{
    //incoming pedestrians
    std::vector<Pedestrian *> peds;

    AgentsQueueIn::GetandClear(peds);

    for(auto && ped : peds) {
        _building->AddPedestrian(ped);
    }
    _building->UpdateGrid();
}

void Simulation::UpdateDoorticks() const {
    //TODO KKZ If possible get rind of this function
};

void Simulation::UpdateFlowAtDoors(const Pedestrian & ped) const
{
    Transition * trans = _building->GetTransitionByUID(ped.GetExitIndex());
    if(!trans)
        return;
    DoorState state = trans->GetState();

    bool regulateFlow = trans->GetOutflowRate() < (std::numeric_limits<double>::max)() ||
                        trans->GetMaxDoorUsage() < std::numeric_limits<double>::max();
    // flow of trans does not need regulation
    // and we don't want to have statistics
    if(!(regulateFlow || _config->ShowStatistics()))
        return;
    if(auto new_trans =
           correctDoorStatistics(ped, trans->DistTo(ped.GetPos()), trans->GetUniqueID());
       new_trans)
        trans = new_trans;

    trans->IncreaseDoorUsage(1, ped.GetGlobalTime());
    trans->IncreasePartialDoorUsage(1);
    if(regulateFlow) {
        // when <dn> agents pass <trans>, we start evaluating the flow
        // .. and maybe close the <trans>
        if(trans->GetPartialDoorUsage() == trans->GetDN()) {
            trans->RegulateFlow(Pedestrian::GetGlobalTime());
            trans->ResetPartialDoorUsage();
        }
    }
    // no flow regulation for crossings
    Crossing * cross = _building->GetCrossingByUID(ped.GetExitIndex());
    if(cross) {
        cross->IncreaseDoorUsage(1, ped.GetGlobalTime());
    }

    if(state != trans->GetState()) {
        _routingEngine->setNeedUpdate(true);
    }
}

void Simulation::incrementCountTraj()
{
    _countTraj++;
}

AgentsSourcesManager & Simulation::GetAgentSrcManager()
{
    return _agentSrcManager;
}

Building * Simulation::GetBuilding()
{
    return _building.get();
}

int Simulation::GetMaxSimTime() const
{
    return _maxSimTime;
}
// return true is changes are made to the geometry
bool Simulation::TrainTraffic()
{
    bool trainHere        = false;
    bool trainLeave       = false;
    std::string trainType = "";
    Point trackStart, trackEnd;
    int trainId = 0;
    auto now    = Pedestrian::GetGlobalTime();
    for(auto && tab : TrainTimeTables) {
        trainType  = tab.second->type;
        trainId    = tab.second->id;
        trackStart = tab.second->pstart;
        trackEnd   = tab.second->pend;
        if(!tab.second->arrival && (now >= tab.second->tin) && (now <= tab.second->tout)) {
            trainHere                            = true;
            TrainTimeTables.at(trainId)->arrival = true;
            std::cout << "Arrival: TRAIN " << trainType << " at time: " << now << "\n";
            correctGeometry(_building, tab.second);

        } else if(tab.second->arrival && now >= tab.second->tout) {
            std::cout << "Departure: TRAIN " << trainType << " at time: " << now << "\n";
            _building->resetGeometry(tab.second);
            trainLeave                           = true;
            TrainTimeTables.at(trainId)->arrival = false;
        }
    }
    if(trainHere || trainLeave) {
        return true;
    }

    return false;
}

Transition *
Simulation::correctDoorStatistics(const Pedestrian & ped, double distance, int trans_id) const
{
    if(distance <= 0.5)
        return nullptr;
    Transition * trans       = nullptr;
    double smallest_distance = 0.3;
    bool success             = false;
    LOG_WARNING(
        "Pedestrian [{}] left room/subroom [{}/{}] in an unusual way. Please check",
        ped.GetID(),
        ped.GetRoomID(),
        ped.GetSubRoomID());
    LOG_INFO(
        "Distance to last door ({} | {}) is {}. That should be smaller.",
        trans_id,
        ped.GetExitIndex(),
        distance);
    LOG_INFO("Correcting the door statistics");
    //checking the history and picking the nearest previous destination
    for(const auto & dest : ped.GetLastDestinations()) {
        if(dest == -1)
            continue;
        Transition * trans_tmp = _building->GetTransitionByUID(dest);
        if(!trans_tmp)
            continue;
        if(auto tmp_distance = trans_tmp->DistTo(ped.GetPos()); tmp_distance < smallest_distance) {
            smallest_distance = tmp_distance;
            trans             = trans_tmp;
            LOG_INFO("Best match found at door {}", dest);
            success = true; //at least one door was found
        }
    }
    if(!success) {
        LOG_WARNING("Correcting the door statistics failed!");
        //TODO we need to check if the ped is in a subroom neighboring the target. If so, no problems!
    }
    return trans;
}
