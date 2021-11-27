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

#include "IO/EventFileParser.h"
#include "IO/TrainFileParser.h"
#include "IO/Trajectories.h"
#include "SimulationClock.h"
#include "SimulationHelper.h"
#include "events/Event.h"
#include "events/EventManager.h"
#include "events/EventVisitors.h"
#include "general/Filesystem.h"
#include "geometry/GoalManager.h"
#include "geometry/NavLine.h"
#include "geometry/WaitingArea.h"
#include "geometry/Wall.h"
#include "math/GCFMModel.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "pedestrian/Pedestrian.h"
#include "routing/ff_router/ffRouter.h"

#include <Logger.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <tinyxml.h>
#include <variant>

// TODO: add these variables to class simulation
std::map<int, double> trainOutflow;

Simulation::Simulation(Configuration * args, std::unique_ptr<Building> && building) :
    _config(args), _clock(_config->Getdt()), _building(std::move(building))
{
    _seed                    = 8091983;
    _operationalModel        = nullptr;
    _fps                     = 1;
    _old_em                  = nullptr;
    _gotSources              = false;
    _fps                     = 1;
    _old_em                  = nullptr;
    _gotSources              = false;
    _maxSimTime              = 100;
    _currentTrajectoriesFile = _config->GetTrajectoriesFile();
}

void Simulation::Iterate()
{
    _building->UpdateGrid();
    const double t_in_sec = _clock.ElapsedTime();

    auto directionManager = _config->GetDirectionManager();
    if(directionManager) {
        directionManager->Update(t_in_sec);
    }

    _config->GetModel()->Update(t_in_sec);

    _routingEngine->UpdateTime(t_in_sec);

    if(t_in_sec > Pedestrian::GetMinPremovementTime()) {
        UpdateRoutes();
        // update the positions
        _operationalModel->ComputeNextTimeStep(t_in_sec, _clock.dT(), _building.get(), _periodic);

        //update the events
        bool eventProcessed = _old_em->ProcessEvents(t_in_sec);
        _routingEngine->setNeedUpdate(eventProcessed || _routingEngine->NeedsUpdate());

        //here we could place router-tasks (calc new maps) that can use multiple cores AND we have 't'
        //update quickestRouter
        if(eventProcessed) {
            LOG_INFO(
                "Enter correctGeometry: Building Has {} Transitions.",
                _building->GetAllTransitions().size());

            _building->GetConfig()->GetDirectionManager()->GetDirectionStrategy()->Init(
                _building.get());
        } else { // quickest needs update even if NeedsUpdate() is false
            auto * ffrouter =
                dynamic_cast<FFRouter *>(_routingEngine->GetRouter(ROUTING_FF_QUICKEST));
            if(ffrouter != nullptr && ffrouter->MustReInit()) {
                ffrouter->ReInit();
                ffrouter->SetRecalc(t_in_sec);
            }
        }

        // here the used routers are update, when needed due to external changes
        if(_routingEngine->NeedsUpdate()) {
            LOG_INFO("Update router during simulation.");
            _routingEngine->UpdateRouter();
        }

        //update the routes and locations
        UpdateLocations();

        // Checks if position of pedestrians is inside waiting area and should be waiting or if
        // left waiting area and assign new goal
        GoalManager gm{_building.get(), this};
        gm.update(t_in_sec);
    }
    _clock.Advance();
}

void Simulation::AddAgent(std::unique_ptr<Pedestrian> && agent)
{
    agent->SetBuilding(_building.get());
    const Point pos = agent->GetPos();
    agent->SetRouter(_routingEngine->GetRouter(agent->GetRouterID()));
    Point target = Point{0.0, 0.0};
    if(agent->FindRoute() == -1) {
        Point p1 = agent->GetPos();
        p1._x += 1;
        p1._y -= 1;
        Point p2 = agent->GetPos();
        p2._x += 1;
        p2._y += 1;
        NavLine dummy(Line{p1, p2});
        agent->SetExitLine(&dummy);
    } else {
        target = agent->GetExitLine()->ShortestPoint(pos);
    }
    // Compute orientation
    const Point posToTarget = target - pos;
    const Point orientation = posToTarget.Normalized();
    agent->InitV0(target);

    JEllipse E = agent->GetEllipse();
    E.SetCosPhi(orientation._x);
    E.SetSinPhi(orientation._y);
    agent->SetEllipse(E);
    _agents.emplace_back(std::move(agent));
}

void Simulation::AddAgents(std::vector<std::unique_ptr<Pedestrian>> && agents)
{
    for(auto && agent : agents) {
        AddAgent(std::move(agent));
    }
}

void Simulation::RemoveAgents(std::vector<int> ids)
{
    _agents.erase(
        std::remove_if(
            _agents.begin(),
            _agents.end(),
            [&ids](auto & agent) {
                const int id = agent->GetID();
                return std::find_if(ids.begin(), ids.end(), [id](int other) {
                           return id == other;
                       }) != ids.end();
            }),
        _agents.end());
}

Pedestrian & Simulation::Agent(int id) const
{
    const auto iter = std::find_if(
        _agents.begin(), _agents.end(), [id](auto & ped) { return id == ped->GetID(); });
    if(iter == _agents.end()) {
        throw std::logic_error("Trying to access unknown Agent.");
    }
    return **iter;
}

const std::vector<std::unique_ptr<Pedestrian>> & Simulation::Agents() const
{
    return _agents;
}

size_t Simulation::GetPedsNumber() const
{
    return _agents.size();
}

void Simulation::AddEvent(Event event)
{
    _em.add(event);
}

void Simulation::AddEvents(std::vector<Event> events)
{
    for(auto e : events) {
        AddEvent(e);
    }
}

bool Simulation::InitArgs()
{
    const fs::path & trajPath(_config->GetTrajectoriesFile());
    const fs::path trajParentPath = trajPath.parent_path();
    if(!trajParentPath.empty()) {
        fs::create_directories(trajParentPath);
    }


    _operationalModel = _config->GetModel();
    _maxSimTime       = _config->GetTmax();
    _periodic         = _config->IsPeriodic();
    _fps              = _config->GetFps();

    _routingEngine = _config->GetRoutingEngine();
    _routingEngine->SetSimulation(this);

    // IMPORTANT: do not change the order in the following..
    _building->SetAgents(&_agents);

    //perform customs initialisation, like computing the phi for the gcfm
    //this should be called after the routing engine has been initialised
    // because a direction is needed for this initialisation.
    LOG_INFO("Init Operational Model starting ...");
    _operationalModel->Init(_building.get(), this);
    LOG_INFO("Init Operational Model done.");

    // Give the DirectionStrategy the chance to perform some initialization.
    // This should be done after the initialization of the operationalModel
    // because then, invalid pedestrians have been deleted and FindExit()
    // has been called.

    //other initializations
    for(auto && ped : _agents) {
        ped->SetDeltaT(_clock.dT());
    }
    LOG_INFO("Number of peds received: {}", _agents.size());
    _seed = _config->GetSeed();

    if(_config->GetDistEffMaxPed() > _config->GetLinkedCellSize()) {
        LOG_ERROR(
            "The linked-cell size [{}] should be larger than the force range [{}]",
            _config->GetLinkedCellSize(),
            _config->GetDistEffMaxPed());
        return false;
    }

    _old_em = std::make_unique<OldEventManager>();
    if(!_config->GetEventFile().empty()) {
        EventFileParser::ParseDoorEvents(*_old_em, _building.get(), _config->GetEventFile());
    }
    if(!_config->GetScheduleFile().empty()) {
        EventFileParser::ParseSchedule(*_old_em, _building.get(), _config->GetScheduleFile());

        // Read and set max door usage from schedule file
        auto groupMaxAgents = EventFileParser::ParseMaxAgents(_config->GetScheduleFile());
        for(auto const & [transID, maxAgents] : groupMaxAgents) {
            _building->GetTransition(transID)->SetMaxDoorUsage(maxAgents);
        }
    }

    if(!_config->GetTrainTypeFile().empty() && !_config->GetTrainTimeTableFile().empty()) {
        auto trainTypes = TrainFileParser::ParseTrainTypes(_config->GetTrainTypeFile());
        TrainFileParser::ParseTrainTimeTable(
            *_old_em, *_building, trainTypes, _config->GetTrainTimeTableFile());
    }

    LOG_INFO("Got {} Trains", _building->GetTrainTypes().size());

    for(auto && TT : _building->GetTrainTypes()) {
        LOG_INFO("Type {}", TT._type);
        LOG_INFO("Max {}", TT._maxAgents);
        LOG_INFO("Number of doors {}", TT._doors.size());
    }

    _old_em->ListEvents();
    return true;
}

void Simulation::UpdateLocations()
{
    auto [pedsChangedRoom, pedsNotRelocated] =
        SimulationHelper::UpdatePedestriansLocations(*_building, _agents);

    // not needed at the moment, as we do not have inside final goals yet
    auto pedsAtFinalGoal = SimulationHelper::FindPedestriansReachedFinalGoal(*_building, _agents);
    _pedsToRemove.insert(_pedsToRemove.end(), pedsAtFinalGoal.begin(), pedsAtFinalGoal.end());

    auto pedsOutside = SimulationHelper::FindPedestriansOutside(*_building, pedsNotRelocated);
    _pedsToRemove.insert(_pedsToRemove.end(), pedsOutside.begin(), pedsOutside.end());

    SimulationHelper::UpdateFlowAtDoors(*_building, pedsChangedRoom, _clock.ElapsedTime());
    SimulationHelper::UpdateFlowAtDoors(*_building, pedsOutside, _clock.ElapsedTime());

    SimulationHelper::RemoveFaultyPedestrians(
        *this, pedsNotRelocated, "Could not be properly relocated");
    SimulationHelper::RemovePedestrians(*this, _pedsToRemove);

    //TODO discuss simulation flow -> better move to main loop, does not belong here
    bool geometryChangedFlow =
        SimulationHelper::UpdateFlowRegulation(*_building, _clock.ElapsedTime());
    bool geometryChangedTrain =
        SimulationHelper::UpdateTrainFlowRegulation(*_building, _clock.ElapsedTime());

    _routingEngine->setNeedUpdate(geometryChangedFlow || geometryChangedTrain);
}

void Simulation::UpdateRoutes()
{
    for(const auto & ped : _agents) {
        // set ped waiting, if no target is found
        int target = ped->FindRoute();

        if(target == FINAL_DEST_OUT) {
            ped->StartWaiting();
        } else {
            if(ped->IsWaiting() && !ped->IsInsideWaitingAreaWaiting(_clock.ElapsedTime())) {
                ped->EndWaiting();
            }
        }
        if(target != FINAL_DEST_OUT) {
            const Hline * door          = _building->GetTransOrCrossByUID(target);
            auto [roomID, subRoomID, _] = _building->GetRoomAndSubRoomIDs(ped->GetPos());

            if(const auto * cross = dynamic_cast<const Crossing *>(door)) {
                if(cross->IsInRoom(roomID) && cross->IsInSubRoom(subRoomID)) {
                    if(!ped->IsWaiting() && cross->IsTempClose()) {
                        ped->StartWaiting();
                    }

                    if(ped->IsWaiting() && cross->IsOpen() &&
                       !ped->IsInsideWaitingAreaWaiting(_clock.ElapsedTime())) {
                        ped->EndWaiting();
                    }
                }
            }
        }
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

            statsfile = _config->GetOutputPath() / statsfile;

            LOG_INFO("More Information in the file: {}", statsfile.string());
            {
                FileHandler statOutput(statsfile);
                statOutput.Write("#Simulation time: %.2f", simTime);
                statOutput.Write(
                    "#Flow at exit " + goal->GetCaption() + "( ID " +
                    std::to_string(goal->GetID()) + " )");
                statOutput.Write("#Time (s), cummulative number of agents, pedestrian ID\n");
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

void Simulation::RunHeader(long nPed, TrajectoryWriter & writer)
{
    // Copy input files used for simulation to output folder for reproducibility
    CopyInputFilesToOutPath();
    UpdateOutputFiles();

    writer.WriteHeader(nPed, _fps, *_config, 0); // first trajectory
    writer.WriteFrame(0, _agents);
    //first initialisation needed by the linked-cells
    UpdateLocations();
}

void Simulation::CopyInputFilesToOutPath()
{
    // In the case we stored the corrected Geometry already in the output path we do not need to copy it again
    if(_config->GetOutputPath() != _config->GetGeometryFile().parent_path()) {
        fs::copy(
            _config->GetProjectRootDir() / _config->GetGeometryFile(),
            _config->GetOutputPath(),
            fs::copy_options::overwrite_existing);
    }

    fs::copy(
        _config->GetProjectFile(), _config->GetOutputPath(), fs::copy_options::overwrite_existing);

    CopyInputFileToOutPath(_config->GetTrafficContraintFile());
    CopyInputFileToOutPath(_config->GetGoalFile());
    CopyInputFileToOutPath(_config->GetTransitionFile());
    CopyInputFileToOutPath(_config->GetEventFile());
    CopyInputFileToOutPath(_config->GetScheduleFile());
    CopyInputFileToOutPath(_config->GetSourceFile());
    CopyInputFileToOutPath(_config->GetTrainTimeTableFile());
    CopyInputFileToOutPath(_config->GetTrainTypeFile());
}

void Simulation::CopyInputFileToOutPath(fs::path file)
{
    if(!file.empty() && fs::exists(file)) {
        fs::copy(file, _config->GetOutputPath(), fs::copy_options::overwrite_existing);
    }
}

void Simulation::UpdateOutputFiles()
{
    UpdateOutputIniFile();
    UpdateOutputGeometryFile();
}

void Simulation::UpdateOutputIniFile()
{
    fs::path iniOutputPath = _config->GetOutputPath() / _config->GetProjectFile().filename();

    TiXmlDocument iniOutput(iniOutputPath.string());

    if(!iniOutput.LoadFile()) {
        LOG_ERROR("Could not parse the ini file.");
        LOG_ERROR("{}", iniOutput.ErrorDesc());
    }
    TiXmlElement * mainNode = iniOutput.RootElement();

    // update geometry file name
    if(mainNode->FirstChild("geometry")) {
        mainNode->FirstChild("geometry")
            ->FirstChild()
            ->SetValue(_config->GetGeometryFile().filename().string());
    } else if(
        mainNode->FirstChild("header") && mainNode->FirstChild("header")->FirstChild("geometry")) {
        mainNode->FirstChild("header")
            ->FirstChild("geometry")
            ->FirstChild()
            ->SetValue(_config->GetGeometryFile().filename().string());
    }

    // update new traffic constraint file name
    if(!_config->GetTrafficContraintFile().empty()) {
        if(mainNode->FirstChild("traffic_constraints") &&
           mainNode->FirstChild("traffic_constraints")->FirstChild("file")) {
            mainNode->FirstChild("traffic_constraints")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->GetTrafficContraintFile().filename().string());
        }
    }

    // update new goal file name
    if(!_config->GetGoalFile().empty()) {
        if(mainNode->FirstChild("routing") &&
           mainNode->FirstChild("routing")->FirstChild("goals") &&
           mainNode->FirstChild("routing")->FirstChild("goals")->FirstChild("file")) {
            mainNode->FirstChild("routing")
                ->FirstChild("goals")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->GetGoalFile().filename().string());
        }
    }

    // update new source file name
    if(!_config->GetSourceFile().empty()) {
        if(mainNode->FirstChild("agents") &&
           mainNode->FirstChild("agents")->FirstChild("agents_sources") &&
           mainNode->FirstChild("agents")->FirstChild("agents_sources")->FirstChild("file")) {
            mainNode->FirstChild("agents")
                ->FirstChild("agents_sources")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->GetSourceFile().filename().string());
        }
    }

    // update new event file name
    if(!_config->GetEventFile().empty()) {
        if(mainNode->FirstChild("events_file")) {
            mainNode->FirstChild("events_file")
                ->FirstChild()
                ->SetValue(_config->GetEventFile().filename().string());
        } else if(
            mainNode->FirstChild("header") &&
            mainNode->FirstChild("header")->FirstChild("events_file")) {
            mainNode->FirstChild("header")
                ->FirstChild("events_file")
                ->FirstChild()
                ->SetValue(_config->GetEventFile().filename().string());
        }
    }

    // update new schedule file name
    if(!_config->GetScheduleFile().empty()) {
        if(mainNode->FirstChild("schedule_file")) {
            mainNode->FirstChild("schedule_file")
                ->FirstChild()
                ->SetValue(_config->GetScheduleFile().filename().string());

        } else if(
            mainNode->FirstChild("header") &&
            mainNode->FirstChild("header")->FirstChild("schedule_file")) {
            mainNode->FirstChild("header")
                ->FirstChild("schedule_file")
                ->FirstChild()
                ->SetValue(_config->GetScheduleFile().filename().string());
        }
    }

    // update new train time table file name
    if(!_config->GetTrainTimeTableFile().empty()) {
        if(mainNode->FirstChild("train_constraints") &&
           mainNode->FirstChild("train_constraints")->FirstChild("train_time_table")) {
            mainNode->FirstChild("train_constraints")
                ->FirstChild("train_time_table")
                ->FirstChild()
                ->SetValue(_config->GetTrainTimeTableFile().filename().string());
        }
    }

    // update new train types file name
    if(!_config->GetTrainTypeFile().empty()) {
        if(mainNode->FirstChild("train_constraints") &&
           mainNode->FirstChild("train_constraints")->FirstChild("train_types")) {
            mainNode->FirstChild("train_constraints")
                ->FirstChild("train_types")
                ->FirstChild()
                ->SetValue(_config->GetTrainTypeFile().filename().string());
        }
    }

    iniOutput.SaveFile(iniOutputPath.string());
}

void Simulation::UpdateOutputGeometryFile()
{
    fs::path geoOutputPath = _config->GetOutputPath() / _config->GetGeometryFile().filename();

    TiXmlDocument geoOutput(geoOutputPath.string());

    if(!geoOutput.LoadFile()) {
        LOG_ERROR("Could not parse the ini file.");
        LOG_ERROR("{}", geoOutput.ErrorDesc());
    }
    TiXmlElement * mainNode = geoOutput.RootElement();

    if(!_config->GetTransitionFile().empty()) {
        if(mainNode->FirstChild("transitions") &&
           mainNode->FirstChild("transitions")->FirstChild("file")) {
            mainNode->FirstChild("transitions")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->GetTransitionFile().filename().string());
        }
    }

    geoOutput.SaveFile(geoOutputPath.string());
}

int Simulation::GetMaxSimTime() const
{
    return _maxSimTime;
}
