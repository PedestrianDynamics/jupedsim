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

#include "general/Filesystem.h"
#include "general/Logger.h"
#include "general/OpenMP.h"
#include "geometry/WaitingArea.h"
#include "IO/progress_bar.h"
#include "router/ff_router/ffRouter.h"
#include "math/GCFMModel.h"
#include "math/GompertzModel.h"
#include "math/GradientModel.h"
#include "pedestrian/AgentsQueue.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "geometry/WaitingArea.h"
#include "general/Filesystem.h"
#include "general/OpenMP.h"
#include "geometry/GoalManager.h"
#include "direction/DirectionManager.h"
#include "direction/walking/DirectionLocalFloorfield.h"
#include "router/ff_router/ffRouter.h"

#include <fmt/format.h>

OutputHandler* Log;
Trajectories* outputTXT;
// todo: add these variables to class simulation
std::map<std::string, std::shared_ptr<TrainType> > TrainTypes;
std::map<int, std::shared_ptr<TrainTimeTable> >  TrainTimeTables;
std::map<int, double> trainOutflow;
//--------
Simulation::Simulation(Configuration* args)
        :_config(args)
{
     _countTraj = 0;
     _maxFileSize = 10; // MB
    _nPeds = 0;
    _seed = 8091983;
    _deltaT = 0;
    _building = nullptr;
    //_direction = NULL;
    _operationalModel = nullptr;
    _solver = nullptr;
    _iod = new IODispatcher();
    _fps = 1;
    _em = nullptr;
    _gotSources = false;
    _trainConstraints = false;
    _maxSimTime = 100;
//     _config = args;
     _goalManager = new GoalManager();
}

Simulation::~Simulation()
{
    delete _solver;
    delete _iod;
    delete _em;
    delete _goalManager;
}

long Simulation::GetPedsNumber() const
{
    return _nPeds;
}

bool Simulation::InitArgs()
{
    char tmp[CLENGTH];
    std::string s = "Parameter:\n";

    switch (_config->GetLog()) {
    case 0: {

        break;
    }
    case 1: {
        delete Log;
        Log = new STDIOHandler();
        break;
    }
    case 2: {
        delete Log;
        Log = new FileHandler(_config->GetErrorLogFile());
    }
        break;
    default:
        printf("Wrong option for Logfile!\n\n");
        return false;
    }

    if (_config->GetPort()!=-1) {
        switch (_config->GetFileFormat()) {
        case FORMAT_XML_PLAIN_WITH_MESH:
        case FORMAT_XML_PLAIN: {
            auto travisto = std::make_shared<SocketHandler>(_config->GetHostname(),
                    _config->GetPort());
            Trajectories* output = new TrajectoriesJPSV05();
            output->SetOutputHandler(travisto);
            _iod->AddIO(output);
            break;
        }
        case FORMAT_XML_BIN: {
            Logging::Warning("Format xml-bin not yet supported in streaming");
            return false;
        }
        case FORMAT_PLAIN: {
            Logging::Warning("Format plain not yet supported in streaming");
            return false;
        }
        case FORMAT_VTK: {
            Logging::Warning("Format vtk not yet supported in streaming");
            return false;
        }
        default: {
            return false;
        }
        }

        s.append("\tonline streaming enabled \n");
    }

    if (!_config->GetTrajectoriesFile().empty()) {
         const fs::path& trajPath(_config->GetTrajectoriesFile());
         fs::create_directories(trajPath.parent_path());

        switch (_config->GetFileFormat()) {
        case FORMAT_XML_PLAIN: {
            auto tofile = std::make_shared<FileHandler>(
                    trajPath.c_str());
            Trajectories* output = new TrajectoriesJPSV05();
            output->SetOutputHandler(tofile);
            _iod->AddIO(output);
            break;
        }
        case FORMAT_PLAIN: {
            auto file = std::make_shared<FileHandler>(trajPath);
            outputTXT = new TrajectoriesFLAT();
            outputTXT->SetOutputHandler(file);
            _iod->AddIO(outputTXT);
            break;
        }
        case FORMAT_VTK: {
            Logging::Warning("Format vtk not yet supported");
            auto vtkTraj = trajPath;
            vtkTraj.replace_extension(".vtk");
            auto file = std::make_shared<FileHandler>(vtkTraj);
            Trajectories* output = new TrajectoriesVTK();
            output->SetOutputHandler(file);
            _iod->AddIO(output);
            break;
        }

        case FORMAT_XML_PLAIN_WITH_MESH: {
            //OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
            //if(_iod) delete _iod;
            //_iod = new TrajectoriesXML_MESH();
            //_iod->AddIO(tofile);
            break;
        }
        case FORMAT_XML_BIN: {
            // OutputHandler* travisto = new SocketHandler(args->GetHostname(), args->GetPort());
            // Trajectories* output= new TrajectoriesJPSV06();
            // output->SetOutputHandler(travisto);
            // _iod->AddIO(output);
            break;
        }
        default: {
            break;
        }
        }
    }

    _operationalModel = _config->GetModel();
    s.append(_operationalModel->GetDescription());

    // ODE solver which is never used!
    auto solver = _config->GetSolver();
    sprintf(tmp, "\tODE Solver: %d\n", solver);
    s.append(tmp);

    sprintf(tmp, "\tnCPU: %d\n", _config->GetMaxOpenMPThreads());
    s.append(tmp);
    sprintf(tmp, "\tt_max: %f\n", _config->GetTmax());
    s.append(tmp);
    _deltaT = _config->Getdt();
    _maxSimTime = _config->GetTmax();
    sprintf(tmp, "\tdt: %f\n", _deltaT);
    _periodic = _config->IsPeriodic();
    sprintf(tmp, "\t periodic: %d\n", _periodic);
    s.append(tmp);

    _fps = _config->GetFps();
    sprintf(tmp, "\tfps: %f\n", _fps);
    s.append(tmp);

    _routingEngine = _config->GetRoutingEngine();
    auto distributor = std::unique_ptr<PedDistributor>(new PedDistributor(_config));
    // IMPORTANT: do not change the order in the following..
    _building = std::shared_ptr<Building>(new Building(_config, *distributor));

    // Initialize the agents sources that have been collected in the pedestrians distributor
    _agentSrcManager.SetBuilding(_building.get());
    _agentSrcManager.SetMaxSimTime(GetMaxSimTime());
    _gotSources = (bool) distributor->GetAgentsSources().size(); // did we have any sources? false if no sources
    std::cout << "\nGot " << _gotSources  << " sources"<< std::endl ;

    for (const auto& src: distributor->GetAgentsSources()) {
        _agentSrcManager.AddSource(src);
        src->Dump();
    }

    //perform customs initialisation, like computing the phi for the gcfm
    //this should be called after the routing engine has been initialised
    // because a direction is needed for this initialisation.
    Logging::Info("Init Operationl Model starting ...");
    if (!_operationalModel->Init(_building.get())) {
        return false;
    }
    Logging::Info("Init Operational Model done");
    Logging::Info(fmt::format("Got {} Train Types",
      _building->GetTrainTypes().size()));

    for(auto&& TT: _building->GetTrainTypes()) {
        Logging::Info(fmt::format("type {}", TT.second->type));
        Logging::Info(fmt::format("Max {}", TT.second->nmax));
        Logging::Info(fmt::format("number doors {}", TT.second->doors.size()));
    }
    if(_building->GetTrainTimeTables().size()) {
        Logging::Info(fmt::format("Got {} Train Time Tables", _building->GetTrainTimeTables().size()));
    } else {
        Logging::Warning(fmt::format(FMT_STRING("Got {} Train Time Tables"), _building->GetTrainTimeTables().size()));
    }
    for(auto&& TT: _building->GetTrainTimeTables())
    {
          Logging::Info(fmt::format("id           : {}",TT.second->id));
          Logging::Info(fmt::format("type         : {}",TT.second->type.c_str()));
          Logging::Info(fmt::format("room id      : {}",TT.second->rid));
          Logging::Info(fmt::format("tin          : {:.2f}",TT.second->tin));
          Logging::Info(fmt::format("tout         : {:.2f}",TT.second->tout));
          Logging::Info(fmt::format("track start  : ({:.2f}, {:.2f})",TT.second->pstart._x,TT.second->pstart._y));
          Logging::Info(fmt::format("track end    : ({:.2f}, {:.2f})",TT.second->pend._x,TT.second->pend._y));
          Logging::Info(fmt::format("train start  : ({:.2f}, {:.2f})",TT.second->tstart._x, TT.second->tstart._y));
          Logging::Info(fmt::format("train end    : ({:.2f}, {:.2f})",TT.second->tend._x, TT.second->tend._y));
    }
    //@todo: these variables are global
    TrainTypes = _building->GetTrainTypes();
    TrainTimeTables = _building->GetTrainTimeTables();
    _trainConstraints = (bool) TrainTimeTables.size();

    //-----
    // Give the DirectionStrategy the chance to perform some initialization.
    // This should be done after the initialization of the operationalModel
    // because then, invalid pedestrians have been deleted and FindExit()
    // has been called.

    //other initializations
    for (auto&& ped: _building->GetAllPedestrians()) {
        ped->Setdt(_deltaT);
    }
    _nPeds = _building->GetAllPedestrians().size();
    //_building->WriteToErrorLog();
    Log->Write("INFO:\t nPeds %d received", _nPeds);
    //get the seed
    _seed = _config->GetSeed();

    //size of the cells/GCFM/Gompertz
    if (_config->GetDistEffMaxPed()>_config->GetLinkedCellSize()) {
        Log->Write(
                "ERROR: the linked-cell size [%f] should be bigger than the force range [%f]",
                _config->GetLinkedCellSize(), _config->GetDistEffMaxPed());
        return false;
    }

    //read and initialize events
    _em = new EventManager(_config, _building.get(), _config->GetSeed());
    if (!_em->ReadEventsXml()) {
        Log->Write("ERROR: \tCould not initialize events handling");
    }
     if (!_em->ReadSchedule()) {
          Log->Write("ERROR: \tCould not initialize schedule handling");
     }

     _em->ListEvents();

    //everything went fine
    return true;
}

double Simulation::RunStandardSimulation(double maxSimTime)
{
    RunHeader(_nPeds+_agentSrcManager.GetMaxAgentNumber());
    double t = RunBody(maxSimTime);
    RunFooter();
    return t;
}

void Simulation::UpdateRoutesAndLocations()
{
     //pedestrians to be deleted
     //you should better create this in the constructor and allocate it once.
     std::set<Pedestrian*> pedsToRemove;
//     pedsToRemove.reserve(500); //just reserve some space

     // collect all pedestrians in the simulation.
     const std::vector<Pedestrian*>& allPeds = _building->GetAllPedestrians();
     const std::map<int, Goal*>& goals = _building->GetAllGoals();
     auto allRooms = _building->GetAllRooms();

//    for (signed int p = 0; p < allPeds.size(); ++p) {
//        Pedestrian* ped = allPeds[p];
//
//        std::cout << "FinalDestination of [" << ped->GetID() << "] in (" << ped->GetRoomID() << ", " << ped->GetSubRoomID() << "/" <<  ped->GetSubRoomUID() << "): " << ped->GetFinalDestination() << std::endl;
//    }

#pragma omp parallel for shared(pedsToRemove, allRooms)
     for (size_t p = 0; p < allPeds.size(); ++p) {
          auto ped = allPeds[p];
          Room* room = _building->GetRoom(ped->GetRoomID());
          SubRoom* sub0 = room->GetSubRoom(ped->GetSubRoomID());

          //set the new room if needed
          if ((ped->GetFinalDestination() == FINAL_DEST_OUT)
//              && (room->GetCaption() == "outside")) { //TODO Hier aendern fuer inside goals?
               && ped->IsOutside()){
#pragma omp critical(Simulation_Update_pedsToRemove)
               pedsToRemove.insert(ped);
          } else if ((ped->GetFinalDestination() != FINAL_DEST_OUT)
                    && (goals.at(ped->GetFinalDestination())->Contains(
                              ped->GetPos()))&& (goals.at(ped->GetFinalDestination())->GetIsFinalGoal())) {
#pragma omp critical(Simulation_Update_pedsToRemove)
               pedsToRemove.insert(ped);
          }

          // reposition in the case the pedestrians "accidentally left the room" not via the intended exit.
          // That may happen if the forces are too high for instance
          // the ped is removed from the simulation, if it could not be reassigned
          else if (!sub0->IsInSubRoom(ped))
          {

               bool assigned = false;
               std::function<void(const Pedestrian&)> f = std::bind(&Simulation::UpdateFlowAtDoors, this, std::placeholders::_1);
               assigned = ped->Relocate(f);
               //this will delete agents, that are pushed outside (maybe even if inside obstacles??)

               if (!assigned) {
#pragma omp critical(Simulation_Update_pedsToRemove)
                       pedsToRemove.insert(ped); //the agent left the old room
                    //actualize the eggress time for that room
#pragma omp critical(SetEgressTime)
                    allRooms.at(ped->GetRoomID())->SetEgressTime(ped->GetGlobalTime());

               }
          }
          // actualize routes for sources
          if(_gotSources)
               ped->FindRoute();
          //finally actualize the route
          if ( !_gotSources && ped->FindRoute() == -1 && !_trainConstraints) {
               //a destination could not be found for that pedestrian
               Log->Write("ERROR: \tCould not find a route for pedestrian %d in room %d and subroom %d",
                         ped->GetID(), ped->GetRoomID(), ped->GetSubRoomID());
               //ped->FindRoute(); //debug only, plz remove
               std::function<void(const Pedestrian&)> f = std::bind(&Simulation::UpdateFlowAtDoors, this, std::placeholders::_1);
               ped->Relocate(f);
               //exit(EXIT_FAILURE);
#pragma omp critical(Simulation_Update_pedsToRemove)
               {
                    pedsToRemove.insert(ped);
                    Log->incrementDeletedAgents();
               }
          }

          // Set pedestrian waiting when find route temp_close
          int goal = ped->FindRoute();
//          std::cout << std::endl;
//          std::cout << "Ped " << ped->GetID() << " Goal: " << goal << std::endl;
          Hline* target = _building->GetTransOrCrossByUID(goal);
          int roomID = ped->GetRoomID();
          int subRoomID = ped ->GetSubRoomID();

          if( Crossing* cross = dynamic_cast<Crossing*>(target) ) {
//               std::cout << "cross state: " << cross->IsTempClose() <<std::endl;
               if (cross->IsInRoom(roomID) && cross->IsInSubRoom(subRoomID)){
                    if (!ped->IsWaiting() && cross->IsTempClose()){
                         ped->StartWaiting();
                    }

                    if (ped->IsWaiting() && cross->IsOpen() && !ped->IsInsideWaitingAreaWaiting()){
                        ped->EndWaiting();
                    }
               }
          }

          // Get new goal for pedestrians who are inside waiting area and wait time is over
          // Check if current position is already waiting area
          // yes: set next goal and return findExit(p)
          _goalManager->ProcessPedPosition(ped);
     }

     _goalManager->ProcessWaitingAreas(Pedestrian::GetGlobalTime());

#ifdef _USE_PROTOCOL_BUFFER
     if (_hybridSimManager)
     {
          AgentsQueueOut::Add(pedsToRemove);    //this should be critical region (and it is)
     }
     else
#endif
    {

        // remove the pedestrians that have left the building
        for (auto p : pedsToRemove){
            UpdateFlowAtDoors(*p);
            _building->DeletePedestrian(p);
        }
        pedsToRemove.clear();
    }

    //    temporary fix for the safest path router
    //    if (dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1)))
    //    {
    //         SafestPathRouter* spr = dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1));
    //         spr->ComputeAndUpdateDestinations(_allPedestians);
    //    }
}

void Simulation::PrintStatistics(double simTime)
{
    Log->Write("\nRooms Egress. Simulation Time: %.2f", simTime);
    Log->Write("==================");
    Log->Write("id\tcaption\tegress time (s)");

    for (const auto& it:_building->GetAllRooms()) {
        auto&& room = it.second;
        if (room->GetCaption()!="outside")
            Log->Write("%d\t%s\t%.2f", room->GetID(), room->GetCaption().c_str(), room->GetEgressTime());
    }

    Log->Write("\nUsage of Exits");
    Log->Write("==========");
    for (const auto& itr : _building->GetAllTransitions()) {
        Transition* goal = itr.second;
        if (goal->GetDoorUsage()) {
            Log->Write(
                    "\nExit ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",
                    goal->GetID(), goal->GetDoorUsage(),
                    goal->GetLastPassingTime());

            fs::path statsfile{"flow_exit_id_"+std::to_string(goal->GetID())+"_"};
            if(goal->GetOutflowRate() <  std::numeric_limits<double>::max()) {
              statsfile += "rate_";
              std::stringstream buffer;
              buffer << std::setprecision(2)
                     << std::fixed
                     << goal->GetOutflowRate();
              statsfile += buffer.str();
              statsfile += '_';
            }
            statsfile += _config->GetOriginalTrajectoriesFile().filename().replace_extension("txt");
            Log->Write("More Information in the file: %s", statsfile.string().c_str());
            {
                 FileHandler statOutput(statsfile);
                 statOutput.Write("#Simulation time: %.2f", simTime);
                 statOutput.Write("#Flow at exit "+goal->GetCaption()+"( ID "+std::to_string(goal->GetID())+" )");
                 statOutput.Write("#Time (s)  cummulative number of agents \n");
                 statOutput.Write(goal->GetFlowCurve());
            }

        }
    }

        Log->Write("\nUsage of Crossings");
        Log->Write("==========");
        for (const auto& itr : _building->GetAllCrossings()) {
             Crossing* goal = itr.second;
             if (goal->GetDoorUsage()) {
                  Log->Write(
                       "\nCrossing ID [%d] in Room ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",
                       goal->GetID(), itr.first/1000, goal->GetDoorUsage(),
                       goal->GetLastPassingTime());

                  fs::path statsfile = "flow_crossing_id_"
                       + std::to_string(itr.first/1000) + "_" + std::to_string(itr.first % 1000) +".dat";
                  Log->Write("More Information in the file: %s", statsfile.string().c_str());
                  FileHandler output(statsfile);
                  output.Write("#Simulation time: %.2f", simTime);
                  output.Write("#Flow at crossing " + goal->GetCaption() + "( ID " + std::to_string(goal->GetID())
                                + " ) in Room ( ID "+ std::to_string(itr.first / 1000) + " )");
                  output.Write("#Time (s)  cummulative number of agents \n");
                  output.Write(goal->GetFlowCurve());
             }
        }

    Log->Write("\n");
}

void Simulation::RunHeader(long nPed)
{
    // writing the header
    if (nPed==-1) nPed = _nPeds;
    _iod->WriteHeader(nPed, _fps, _building.get(), _seed, 0);// first trajectory
                                                             // count = 0
    _iod->WriteGeometry(_building.get());
    if( _gotSources)
         _iod->WriteSources( GetAgentSrcManager().GetSources());

    int writeInterval = (int) ((1./_fps)/_deltaT+0.5);
    writeInterval = (writeInterval<=0) ? 1 : writeInterval; // mustn't be <= 0
    int firstframe = (Pedestrian::GetGlobalTime()/_deltaT)/writeInterval;

    _iod->WriteFrame(firstframe, _building.get());
    //first initialisation needed by the linked-cells
    UpdateRoutesAndLocations();
    ProcessAgentsQueue();
}

double Simulation::RunBody(double maxSimTime)
{
    //needed to control the execution time PART 1
    //in the case you want to run in no faster than realtime
    //time_t starttime, endtime;
    //time(&starttime);

    //take the current time from the pedestrian
    double t = Pedestrian::GetGlobalTime();

    //frame number. This function can be called many times,
    static int frameNr = (int) (1+t/_deltaT); // Frame Number

    //##########
    //PROBLEMATIC: time when frame should be printed out
    // possibly skipped when using the following lines
    // NEEDS TO BE FIXED!
    int writeInterval = (int) ((1./_fps)/_deltaT+0.5);
    writeInterval = (writeInterval<=0) ? 1 : writeInterval; // mustn't be <= 0
    // ##########

    //process the queue for incoming pedestrians
    //important since the number of peds is used
    //to break the main simulation loop
    ProcessAgentsQueue();
    _nPeds = _building->GetAllPedestrians().size();
    std::cout << "\n";
    std::string description = "Evacuation ";
    ProgressBar bar(_nPeds, description);
    int initialnPeds = _nPeds;

    _goalManager->SetBuilding(_building.get());
    _goalManager->SetGoals(_building->GetAllGoals());

    // main program loop
    while ((_nPeds || (!_agentSrcManager.IsCompleted()&& _gotSources) ) && t<maxSimTime) {
        t = 0+(frameNr-1)*_deltaT;
        // Handle train traffic: coorect geometry
        bool geometryChanged= TrainTraffic();

        //process the queue for incoming pedestrians
        ProcessAgentsQueue();

        if (t>Pedestrian::GetMinPremovementTime()) {
            //update the linked cells
            _building->UpdateGrid();

            // update the positions
            _operationalModel->ComputeNextTimeStep(t, _deltaT, _building.get(), _periodic);

            //update the events
            _em->ProcessEvent();

            //here we could place router-tasks (calc new maps) that can use multiple cores AND we have 't'
            //update quickestRouter
            if(geometryChanged)
            {
                 // debug
                 const std::string prefix = "tmp_" + std::to_string(t) + "_";
                 auto changedGeometryFile = add_prefix_to_filename(prefix, _config->GetGeometryFile());
                 std::cout << "\nUpdate geometry. New  geometry --> " << changedGeometryFile << "\n";

                 std::cout<< KGRN << "Enter correctGeometry: Building Has " << _building->GetAllTransitions().size() << " Transitions\n" << RESET;
                 _building->SaveGeometry(changedGeometryFile);

                 if(auto dirlocff = dynamic_cast<DirectionLocalFloorfield*>(_building->GetConfig()->GetDirectionManager()->GetDirectionStrategy().get())){
                      Log->Write("INFO:\t Init DirectionLOCALFloorfield starting ...");
                      dirlocff->Init(_building.get());
                      Log->Write("INFO:\t Init DirectionLOCALFloorfield done");
                 }
                }
            else{ // quickest needs update even if NeedsUpdate() is false
                 auto ffrouter = dynamic_cast<FFRouter*>(_routingEngine->GetRouter(ROUTING_FF_QUICKEST));
                 if(ffrouter != nullptr)
                      if (ffrouter->MustReInit()) {
                           ffrouter->ReInit();
                           ffrouter->SetRecalc(t);
                      }
            }

            // here the used routers are updated, when needed due to external changes
            if (_routingEngine->NeedsUpdate()){
                 std::cout << KBLU << " Init router in simulation\n" << RESET;
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
        if (0 == frameNr % writeInterval) {
              _iod->WriteFrame(frameNr / writeInterval, _building.get());
              WriteTrajectories();
        }

        if(!_gotSources && !_periodic && _config->print_prog_bar())
              // Log->ProgressBar(initialnPeds, initialnPeds-_nPeds, t);
              bar.Progressed(initialnPeds-_nPeds);
        else
             if ((!_gotSources) &&
                 ((frameNr < 100 &&  frameNr % 10 == 0) ||
                  (frameNr > 100 &&  frameNr % 100 == 0))){
                  printf("time: %6.2f (%4.0f)  | Agents: %6ld / %d [%4.1f%%]\n",  t , maxSimTime, _nPeds, initialnPeds, (double)(initialnPeds-_nPeds)/initialnPeds*100);
             }


        // needed to control the execution time PART 2
        // time(&endtime);
        // double timeToWait=t-difftime(endtime, starttime);
        // clock_t goal = timeToWait*1000 + clock();
        // while (goal > clock());
        ++frameNr;

        //Trigger JPSfire Toxicity Analysis
        //only executed every 3 seconds
        #ifdef JPSFIRE
        if( fmod(Pedestrian::GetGlobalTime(), 3) == 0 ) {
            for (auto&& ped: _building->GetAllPedestrians()) {
                ped->ConductToxicityAnalysis();
            }
        }
        #endif

        //init train trainOutfloww
        for (auto tab : TrainTimeTables)
        {
              trainOutflow[tab.first] = 0;
        }
        // regulate flow
        for (auto& itr: _building->GetAllTransitions())
        {
             Transition* Trans = itr.second;

             Trans->UpdateTemporaryState(_deltaT);
             //-----------
             // regulate train doorusage
             std::string transType = Trans->GetType();
             if (Trans->IsOpen() && transType.rfind("Train", 0) == 0)
             {
                   std::vector<std::string> strs;
                   boost::split(strs, transType, boost::is_any_of("_"),boost::token_compress_on);
                   int id = atoi(strs[1].c_str());
                   std::string type = Trans->GetCaption();
                   trainOutflow[id] += Trans->GetDoorUsage();
                   if(trainOutflow[id] >= TrainTypes[type]->nmax)
                   {
                        std::cout << "INFO:\tclosing train door "<<  transType.c_str() << " at "<<  Pedestrian::GetGlobalTime() << " capacity " <<  TrainTypes[type]->nmax<< "\n";
                         Log->Write("INFO:\tclosing train door %s at t=%.2f. Flow = %.2f (Train Capacity %d)", transType.c_str(), Pedestrian::GetGlobalTime(), trainOutflow[id], TrainTypes[type]->nmax);
                         Trans->Close();
                   }
             }
             //-----------
        }// Transitions
        if(frameNr % 1000 == 0)
        {
             if (_config->ShowStatistics()){
                  Log->Write("INFO:\tUpdate door statistics at t=%.2f", t);
                  PrintStatistics(t);
             }
        }
    }// while time
    return t;
}

void Simulation::WriteTrajectories()
{
    if(_config->GetFileFormat() != FORMAT_PLAIN) {return;}

    const fs::path& p = _config->GetTrajectoriesFile();

    int sf = fs::file_size(p);
    if(sf > _maxFileSize * 1024 * 1024)
    {
          const fs::path stem = p.stem();
          const fs::path extention = p.extension();
          const fs::path parent = p.parent_path();
          incrementCountTraj();
          char tmp_traj_name[100];
          sprintf(tmp_traj_name,"%s_%.4d_%s", stem.string().c_str(), _countTraj,
              extention.string().c_str());
          const fs::path abs_traj_name = parent / fs::path(tmp_traj_name);
          _config->SetTrajectoriesFile(abs_traj_name);
          Log->Write("INFO:\tNew trajectory file <%s>", tmp_traj_name);
          auto file = std::make_shared<FileHandler>(_config->GetTrajectoriesFile());
          outputTXT->SetOutputHandler(file);
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
bool Simulation::correctGeometry(std::shared_ptr<Building> building, std::shared_ptr<TrainTimeTable> tab)
{
     //auto platforms = building->GetPlatforms();
     int trainId = tab->id;
     std::string trainType = tab->type;
     Point TrackStart = tab->pstart;
     Point TrainStart = tab->tstart;
     Point TrackEnd = tab->pend;
     SubRoom * subroom;
     int room_id, subroom_id;
     auto mytrack = building->GetTrackWalls(TrackStart, TrackEnd, room_id, subroom_id);
     Room* room = building->GetRoom(room_id);
     subroom = room->GetSubRoom(subroom_id);
     if(subroom == nullptr)
     {
          Log->Write("ERROR:\t Simulation::correctGeometry got wrong room_id|subroom_id (%d|%d). TrainId %d", room_id, subroom_id, trainId);
          exit(EXIT_FAILURE);
     }
     static int transition_id = 10000; // randomly high number

     std::cout << "enter with train " << trainType.c_str() << "\n";
     std::cout<< KBLU << "Enter correctGeometry: Building Has " << building->GetAllTransitions().size() << " Transitions\n" << RESET;
     std::cout << "room: " << room_id << " subroom_id " << subroom_id << "\n" ;

      if(mytrack.empty() || subroom == nullptr)
            return false;


      auto train = building->GetTrainTypes().at(trainType);
      auto doors = train->doors;
      for(auto && d: doors)
      {
           auto newX  = d.GetPoint1()._x + TrainStart._x + TrackStart._x;
           auto newY  = d.GetPoint1()._y + TrainStart._y + TrackStart._y;
           d.SetPoint1(Point(newX, newY));
           newX  = d.GetPoint2()._x + TrainStart._x + TrackStart._x;
           newY  = d.GetPoint2()._y + TrainStart._y + TrackStart._y;
           d.SetPoint2(Point(newX, newY));
      }
      for(auto d: doors)
      {
           Log->Write("Train %s %d. Transformed coordinates of doors: %s -- %s", trainType.c_str(), trainId, d.GetPoint1().toString().c_str(), d.GetPoint2().toString().c_str());
      }

      // std::vector<std::pair<PointWall, pointWall > >
      auto pws = building->GetIntersectionPoints(doors, mytrack);
      if(pws.empty())
           std::cout << KRED << "simulation::correctGeometry: pws are empty. Something went south with train doors\n" << RESET;

      auto walls = subroom->GetAllWalls();
      //---
      for(auto pw: pws)
      {
            auto pw1 = pw.first;
            auto pw2 = pw.second;
            auto p1 = pw1.first;
            auto w1 = pw1.second;
            auto p2 = pw2.first;
            auto w2 = pw2.second;
            // std::cout << "p1 " << p1.toString() << ", wall: " << w1.toString() << "\n";
            // std::cout << "p2 " << p2.toString() << ", wall: " << w2.toString() << "\n";
            // std::cout << "------\n";
            // case 1
            Point P;
            if(w1 == w2)
            {
                  std::cout << "EQUAL\n";
                  Transition* e = new Transition();
                  e->SetID(transition_id++);
                  e->SetCaption(trainType);
                  e->SetPoint1(p1);
                  e->SetPoint2(p2);
                  std::string transType = "Train_"+std::to_string(tab->id)+"_"+std::to_string(tab->tin)+"_"+std::to_string(tab->tout);
                  e->SetType(transType);
                  room->AddTransitionID(e->GetUniqueID());// danger area
                  e->SetRoom1(room);
                  e->SetSubRoom1(subroom);
                  subroom->AddTransition(e);// danger area
                  building->AddTransition(e);// danger area
                  /* std::cout << KRED << "Trans added: " << e->toString() << "\n" << RESET; */

                  /* std::cout<< KGRN << "Transition added. Building Has " << building->GetAllTransitions().size() << " Transitions\n" << RESET; */
                  double dist_pt1 = (w1.GetPoint1() - e->GetPoint1()).NormSquare();
                  double dist_pt2 = (w1.GetPoint1() - e->GetPoint2()).NormSquare();
                  Point A, B;

                  if(dist_pt1<dist_pt2)
                  {
                        A = e->GetPoint1();
                        B = e->GetPoint2();
                  }
                  else
                  {
                        A = e->GetPoint2();
                        B = e->GetPoint1();
                  }

                  Wall NewWall(w1.GetPoint1(), A);
                  Wall NewWall1(w1.GetPoint2(), B);
                  NewWall.SetType(w1.GetType());
                  NewWall1.SetType(w1.GetType());

                  // add new lines to be controled against overlap with exits
                  if(NewWall.GetLength() > J_EPS_DIST)
                  {
                       building->TempAddedWalls[trainId].push_back(NewWall);
                       subroom->AddWall(NewWall);
                  }

                  else
                       std::cout << KRED << ">> WALL did not add: " << NewWall.toString() << "\n" << RESET ;

                  if(NewWall1.GetLength() > J_EPS_DIST)
                  {
                       building->TempAddedWalls[trainId].push_back(NewWall1);
                       subroom->AddWall(NewWall1);
                  }
                  else
                       std::cout << KRED << ">> WALL did not add: " << NewWall1.toString() << "\n" << RESET ;

                  building->TempAddedDoors[trainId].push_back(*e);
                  building->TempRemovedWalls[trainId].push_back(w1);

                  subroom->RemoveWall(w1);


                  /* std::cout << KRED << "WALL added " << NewWall1.toString() << "\n" << RESET ; */
                  /* std::cout << KRED << "WALL removed " << w1.toString() << "\n" << RESET ; */
                  /* getc(stdin); */

                  //room->AddTransitionID(e->GetUniqueID());
            }
            else if(w1.ShareCommonPointWith(w2, P))
            {

                  std::cout << "ONE POINT COMON\n";
                  //------------ transition --------
                  Transition* e = new Transition();
                  e->SetID(transition_id++);
                  e->SetCaption(trainType);
                  e->SetPoint1(p1);
                  e->SetPoint2(p2);
                  std::string transType = "Train_"+std::to_string(tab->id)+"_"+std::to_string(tab->tin)+"_"+std::to_string(tab->tout);
                  e->SetType(transType);
                  room->AddTransitionID(e->GetUniqueID());// danger area
                  e->SetRoom1(room);
                  e->SetSubRoom1(subroom);

                  subroom->AddTransition(e);// danger area
                  building->AddTransition(e);// danger area
                  //--------------------------------
                  Point N, M;
                  if(w1.GetPoint1()==P)
                        N = w1.GetPoint2();
                  else
                        N = w1.GetPoint1();

                  if(w2.GetPoint1()==P)
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
                  /* std::cout << KRED << ". WALL added " << NewWall.toString() << "\n" << RESET ; */
                  /* std::cout << KRED << "WALL added " << NewWall1.toString() << "\n" << RESET ; */
                  /* std::cout << KRED << "WALL removed " << w1.toString() << "\n" << RESET ; */
                  /* std::cout << KRED << "WALL removed " << w2.toString() << "\n" << RESET ; */
                  /* getc(stdin); */
            }
            else // disjoint
            {
                  std::cout << "DISJOINT\n";
                                    //------------ transition --------
                  Transition* e = new Transition();
                  e->SetID(transition_id++);
                  e->SetCaption(trainType);
                  e->SetPoint1(p1);
                  e->SetPoint2(p2);
                  std::string transType = "Train_"+std::to_string(tab->id)+"_"+std::to_string(tab->tin)+"_"+std::to_string(tab->tout);
                  e->SetType(transType);
                  room->AddTransitionID(e->GetUniqueID());// danger area
                  e->SetRoom1(room);
                  e->SetSubRoom1(subroom);

                  subroom->AddTransition(e);// danger area
                  building->AddTransition(e);// danger area
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
                  for(auto wall: mytrack)
                  {
                       if(e->isBetween(wall.GetPoint1()) || e->isBetween(wall.GetPoint2()))
                        {
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
    // writing the footer
    _iod->WriteFooter();
}

void Simulation::ProcessAgentsQueue()
{

     /* std::cout << "Call Simulation::ProcessAgentsQueue() at: " << Pedestrian::GetGlobalTime() << std::endl; */
     /* std::cout << KRED << " SIMULATION building " << _building << " size "  << _building->GetAllPedestrians().size() << "\n" << RESET; */
           /* for(auto pp: _building->GetAllPedestrians()) */
     /*           std::cout<< KBLU << "BUL: Simulation: " << pp->GetPos()._x << ", " << pp->GetPos()._y << RESET << std::endl; */

    //incoming pedestrians
    std::vector<Pedestrian*> peds;
    //  std::cout << ">>> peds " << peds.size() << RESET<< std::endl;

    AgentsQueueIn::GetandClear(peds);
    //std::cout << "SIMULATION BEFORE BOOL = " <<  _agentSrcManager.IsBuildingUpdated() << " peds size " << peds.size() << "\n" ;

    //_agentSrcManager.SetBuildingUpdated(true);
    /* std::cout << "SIMULATION AFTER BOOL = " <<  _agentSrcManager.IsBuildingUpdated() << "\n" ; */

    for (auto&& ped: peds) {
            /* std::cout << "Add to building : " << ped->GetPos()._x << ", " << ped->GetPos()._y << " t: "<< Pedestrian::GetGlobalTime() << std::endl; */
        _building->AddPedestrian(ped);
    }
    _building->UpdateGrid();
    //  for(auto pp: _building->GetAllPedestrians())
    //         std::cout<< KBLU << "BUL: Simulation: " << pp->GetPos()._x << ", " << pp->GetPos()._y  << " t: "<< Pedestrian::GetGlobalTime() <<RESET << std::endl;


    /* std::cout << "LEAVE Simulation::ProcessAgentsQueue() with " << " size "  << _building->GetAllPedestrians().size() << "\n" << RESET; */
}

void Simulation::UpdateDoorticks() const {
//    int softstateDecay = 1;
//    //Softstate of _lastTickTime is valid for X seconds as in (X/_deltaT); here it is 2s
//    auto& allCross = _building->GetAllCrossings();
//    for (auto& crossPair : allCross) {
//        crossPair.second->_refresh1 += 1;
//        crossPair.second->_refresh2 += 1;
//        if (crossPair.second->_refresh1 > (softstateDecay/_deltaT)) {
//            crossPair.second->_lastTickTime1 = 0;
//            crossPair.second->_refresh1 = 0;
//        }
//        if (crossPair.second->_refresh2 > (softstateDecay/_deltaT)) {
//            crossPair.second->_lastTickTime2 = 0;
//            crossPair.second->_refresh2 = 0;
//        }
//    }
//
//    auto& allTrans = _building->GetAllTransitions();
//    for (auto& transPair : allTrans) {
//        transPair.second->_refresh1 += 1;
//        transPair.second->_refresh2 += 1;
//        if (transPair.second->_refresh1 > (softstateDecay/_deltaT)) {
//            transPair.second->_lastTickTime1 = 0;
//            transPair.second->_refresh1 = 0;
//        }
//        if (transPair.second->_refresh2 > (softstateDecay/_deltaT)) {
//            transPair.second->_lastTickTime2 = 0;
//            transPair.second->_refresh2 = 0;
//        }
//    }
};

void Simulation::UpdateFlowAtDoors(const Pedestrian& ped) const
{
     Transition* trans = _building->GetTransitionByUID(ped.GetExitIndex());
     if (!trans)
          return;

     bool regulateFlow = trans->GetOutflowRate() <  (std::numeric_limits<double>::max)() ||
             trans->GetMaxDoorUsage() < std::numeric_limits<double>::max();
     // flow of trans does not need regulation
     // and we don't want to have statistics
     if(!(regulateFlow || _config->ShowStatistics())) return;
     if(auto new_trans = correctDoorStatistics(ped, trans->DistTo(ped.GetPos()), trans->GetUniqueID()); new_trans)
          trans = new_trans;

     trans->IncreaseDoorUsage(1, ped.GetGlobalTime());
     trans->IncreasePartialDoorUsage(1);
     if(regulateFlow)
     {
          // when <dn> agents pass <trans>, we start evaluating the flow
          // .. and maybe close the <trans>
          if( trans->GetPartialDoorUsage() ==  trans->GetDN() ) {
               trans->RegulateFlow(Pedestrian::GetGlobalTime());
               trans->ResetPartialDoorUsage();
          }
     }
     // no flow regulation for crossings
     Crossing* cross = _building->GetCrossingByUID(ped.GetExitIndex());
     if (cross) {
          cross->IncreaseDoorUsage(1, ped.GetGlobalTime());
     }
}

void Simulation::incrementCountTraj()
{
      _countTraj++;
}

AgentsSourcesManager& Simulation::GetAgentSrcManager()
{
    return _agentSrcManager;
}

Building* Simulation::GetBuilding()
{
    return _building.get();
}

int Simulation::GetMaxSimTime() const{
      return _maxSimTime;
}

// return true is changes are made to the geometry
bool Simulation::TrainTraffic()
{
     bool trainHere = false;
     bool trainLeave = false;
     std::string trainType = "";
     Point trackStart, trackEnd;
     int trainId = 0;
     auto now = Pedestrian::GetGlobalTime();
     for(auto && tab: TrainTimeTables)
     {
          trainType = tab.second->type;
          trainId = tab.second->id;
          trackStart = tab.second->pstart;
          trackEnd = tab.second->pend;
          if(!tab.second->arrival && (now >= tab.second->tin) && (now <= tab.second->tout))
          {
               trainHere = true;
               TrainTimeTables.at(trainId)->arrival = true;
               std::cout << KRED << "Arrival: TRAIN " << trainType << " at time: " << now << "\n" << RESET;
               correctGeometry(_building, tab.second);

          }
          else if(tab.second->arrival && now >= tab.second->tout)
          {
               std::cout <<KGRN << "Departure: TRAIN " << trainType << " at time: " << now  << "\n" << RESET;
               _building->resetGeometry(tab.second);
               trainLeave = true;
               TrainTimeTables.at(trainId)->arrival = false;

          }
     }
     if(trainHere || trainLeave)
     {
          return true;
     }

     /* std::cout<< KRED << "Check: Building Has " << _building->GetAllTransitions().size() << " Transitions\n" << RESET; */

     return false;

}

Transition* Simulation::correctDoorStatistics(const Pedestrian& ped, double distance, int trans_id) const
{
     if(distance<=0.5) return nullptr;
     Transition* trans = nullptr;
     double smallest_distance = 0.3;
     bool success = false;
     Log->Write("WARNING:\t pedestrian [%d] left room/subroom [%d/%d] in an unusual way. Please check",
                ped.GetID(), ped.GetRoomID(), ped.GetSubRoomID());
     Log->Write("       :\t distance to last door (%d | %d) is %f. That should be smaller.",
                trans_id, ped.GetExitIndex(),
                distance);
     Log->Write("       :\t correcting the door statistics");
     //checking the history and picking the nearest previous destination
     for (const auto& dest:ped.GetLastDestinations()) {
          if (dest == -1) continue;
          Transition* trans_tmp = _building->GetTransitionByUID(dest);
          if(!trans_tmp) continue;
          if (auto tmp_distance = trans_tmp->DistTo(ped.GetPos());
              tmp_distance < smallest_distance) {
               smallest_distance = tmp_distance;
               trans = trans_tmp;
               Log->Write("       :\t Best match found at door %d", dest);
               success = true;//at least one door was found
          }
     }
     if (!success) {
          Log->Write("WARNING       :\t correcting the door statistics failed!");
          //todo we need to check if the ped is in a subroom neighboring the target. If so, no problems!
     }
     return trans;
}
