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

#include "Simulation.hpp"

#include "IO/EventFileParser.hpp"
#include "IO/TrainFileParser.hpp"
#include "IO/Trajectories.hpp"
#include "SimulationClock.hpp"
#include "SimulationHelper.hpp"
#include "direction/DirectionManager.hpp"
#include "general/Filesystem.hpp"
#include "geometry/GoalManager.hpp"
#include "geometry/Line.hpp"
#include "geometry/TrainGeometryInterface.hpp"
#include "geometry/WaitingArea.hpp"
#include "geometry/Wall.hpp"
#include "geometry/helper/CorrectGeometry.hpp"
#include "math/GCFMModel.hpp"
#include "math/OperationalModel.hpp"
#include "pedestrian/AgentsSourcesManager.hpp"
#include "pedestrian/Pedestrian.hpp"
#include "routing/ff_router/ffRouter.hpp"

#include <Logger.hpp>
#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <tinyxml.h>
#include <variant>

Simulation::Simulation(Configuration * args, std::unique_ptr<Building> && building) :
    _config(args),
    _clock(_config->dT),
    _building(std::move(building)),
    _directionManager(DirectionManager::Create(*args, _building.get())),
    _routingEngine(std::make_unique<RoutingEngine>(args, _building.get(), _directionManager.get())),
    _operationalModel(
        OperationalModel::CreateFromType(args->operationalModel, *args, _directionManager.get())),
    _currentTrajectoriesFile(_config->trajectoriesFile)
{
    _routingEngine->SetSimulation(this);
}

void Simulation::Iterate()
{
    _building->UpdateGrid();
    const double t_in_sec = _clock.ElapsedTime();

    _directionManager->Update(t_in_sec);
    _operationalModel->Update(t_in_sec);
    _routingEngine->UpdateTime(t_in_sec);

    if(t_in_sec > Pedestrian::GetMinPremovementTime()) {
        _routingEngine->setNeedUpdate(_eventProcessed || _routingEngine->NeedsUpdate());
        UpdateRoutes();
        // update the positions
        _operationalModel->ComputeNextTimeStep(t_in_sec, _clock.dT(), _building.get());

        //update the events

        //here we could place router-tasks (calc new maps) that can use multiple cores AND we have 't'
        //update quickestRouter
        if(_eventProcessed) {
            LOG_INFO(
                "Enter correctGeometry: Building Has {} Transitions.",
                _building->GetAllTransitions().size());

            _directionManager->GetDirectionStrategy().ReInit();
        }

        // here the used routers are update, when needed due to external changes

        //update the routes and locations
        UpdateLocations();

        // Checks if position of pedestrians is inside waiting area and should be waiting or if
        // left waiting area and assign new goal
        GoalManager gm{_building.get(), this};
        gm.update(t_in_sec);
    }
    _eventProcessed = false;
    _clock.Advance();
}

void Simulation::AddAgent(std::unique_ptr<Pedestrian> && agent)
{
    agent->SetBuilding(_building.get());
    const Point pos = agent->GetPos();
    auto * router   = _routingEngine->GetRouter(agent->GetRouterID());
    Point target    = Point{0.0, 0.0};
    if(router->FindExit(agent.get()) == -1) {
        Point p1 = agent->GetPos();
        p1._x += 1;
        p1._y -= 1;
        Point p2 = agent->GetPos();
        p2._x += 1;
        p2._y += 1;
        Line dummy(Line{p1, p2});
        agent->SetExitLine(&dummy);
    } else {
        target = agent->GetExitLine().ShortestPoint(pos);
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

void Simulation::RemoveAgents(std::vector<Pedestrian::UID> ids)
{
    _agents.erase(
        std::remove_if(
            _agents.begin(),
            _agents.end(),
            [&ids](auto & agent) {
                const auto uid = agent->GetUID();
                return std::find_if(ids.begin(), ids.end(), [uid](Pedestrian::UID other) {
                           return uid == other;
                       }) != ids.end();
            }),
        _agents.end());
}

Pedestrian & Simulation::Agent(Pedestrian::UID id) const
{
    const auto iter = std::find_if(
        _agents.begin(), _agents.end(), [id](auto & ped) { return id == ped->GetUID(); });
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

void Simulation::OpenDoor(int doorId)
{
    _eventProcessed = true;
    _building->GetTransition(doorId)->Open(true);
}

void Simulation::TempCloseDoor(int doorId)
{
    _eventProcessed = true;
    _building->GetTransition(doorId)->TempClose(true);
}

void Simulation::CloseDoor(int doorId)
{
    _eventProcessed = true;
    _building->GetTransition(doorId)->Close(true);
}

void Simulation::ResetDoor(int doorId)
{
    _eventProcessed = true;
    _building->GetTransition(doorId)->ResetDoorUsage();
}

void Simulation::ActivateTrain(
    int trainId,
    int trackId,
    const TrainType & type,
    double startOffset,
    bool reversed)
{
    geometry::helper::AddTrainDoors(trainId, trackId, *_building, type, startOffset, reversed);
    _eventProcessed = true;
};

void Simulation::DeactivateTrain(int trainId, int trackId)
{
    const auto track = _building->GetTrack(trackId);
    if(!track) {
        throw std::runtime_error(
            fmt::format(FMT_STRING("Could not find track with ID {:d}"), trackId));
    }

    const auto roomID    = track->_roomID;
    const auto subroomID = track->_subRoomID;
    auto * subroom       = _building->GetRoom(roomID)->GetSubRoom(subroomID);

    // remove temp added walls
    const auto tempAddedWalls = _building->GetTrainWallsAdded(trainId);
    if(tempAddedWalls) {
        std::for_each(
            std::begin(tempAddedWalls.value()),
            std::end(tempAddedWalls.value()),
            [&subroom](const Wall & wall) { subroom->RemoveWall(wall); });

        _building->ClearTrainWallsAdded(trainId);
    }

    // add removed walls
    const auto tempRemovedWalls = _building->GetTrainWallsRemoved(trainId);
    if(tempRemovedWalls) {
        std::for_each(
            std::begin(tempRemovedWalls.value()),
            std::end(tempRemovedWalls.value()),
            [&subroom](const Wall & wall) { subroom->AddWall(wall); });

        _building->ClearTrainWallsRemoved(trainId);
    }

    // remove added doors
    const auto tempDoors = _building->GetTrainDoorsAdded(trainId);
    if(tempDoors) {
        std::for_each(
            std::begin(tempDoors.value()),
            std::end(tempDoors.value()),
            [&subroom, this](const Transition & door) {
                subroom->RemoveTransitionByUID(door.GetUniqueID());
                _building->RemoveTransition(&door);
            });

        _building->ClearTrainDoorsAdded(trainId);
    }

    subroom->Update();
    _eventProcessed = true;
};

bool Simulation::InitArgs()
{
    const fs::path & trajPath     = _config->trajectoriesFile;
    const fs::path trajParentPath = trajPath.parent_path();
    if(!trajParentPath.empty()) {
        fs::create_directories(trajParentPath);
    }

    _fps = _config->fps;


    // IMPORTANT: do not change the order in the following..
    _building->SetAgents(&_agents);

    //perform customs initialisation, like computing the phi for the gcfm
    //this should be called after the routing engine has been initialised
    // because a direction is needed for this initialisation.
    LOG_INFO("Init Operational Model starting ...");
    _operationalModel->Init(_building.get(), this, *_config);
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
    _seed = _config->seed;

    if(_config->distEffMaxPed > _config->linkedCellSize) {
        LOG_ERROR(
            "The linked-cell size [{}] should be larger than the force range [{}]",
            _config->linkedCellSize,
            _config->distEffMaxPed);
        return false;
    }

    return true;
}

void Simulation::UpdateLocations()
{
    SimulationHelper::UpdateFlowAtDoors(*_building, _agents, _clock.ElapsedTime());
    auto pedsOutside = SimulationHelper::FindPedestriansOutside(*_building, _agents);
    RemoveAgents(pedsOutside);

    //TODO discuss simulation flow -> better move to main loop, does not belong here
    bool geometryChangedFlow = SimulationHelper::UpdateFlowRegulation(*_building, _clock);
    bool geometryChangedTrain =
        SimulationHelper::UpdateTrainFlowRegulation(*_building, _clock.ElapsedTime());

    _routingEngine->setNeedUpdate(geometryChangedFlow || geometryChangedTrain);
}

void Simulation::UpdateRoutes()
{
    if(_routingEngine->NeedsUpdate()) {
        LOG_INFO("Update router during simulation.");
        _routingEngine->UpdateRouter();
    }
    for(const auto & ped : _agents) {
        // set ped waiting, if no target is found
        auto * router = _routingEngine->GetRouter(ped->GetRouterID());
        int target    = router->FindExit(ped.get());

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
            statsfile += _config->trajectoriesFile.filename().replace_extension("txt");

            statsfile = _config->outputPath / statsfile;

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
    fs::create_directories(_config->outputPath);
    // In the case we stored the corrected Geometry already in the output path we do not need to copy it again
    if(_config->outputPath != _config->geometryFile.parent_path()) {
        fs::copy(
            _config->projectRootDir / _config->geometryFile,
            _config->outputPath,
            fs::copy_options::overwrite_existing);
    }

    fs::copy(_config->iniFile, _config->outputPath, fs::copy_options::overwrite_existing);

    CopyInputFileToOutPath(_config->trafficContraintFile);
    CopyInputFileToOutPath(_config->goalFile);
    CopyInputFileToOutPath(_config->transitionFile);
    if(_config->eventFile) {
        CopyInputFileToOutPath(_config->eventFile.value());
    }
    if(_config->scheduleFile) {
        CopyInputFileToOutPath(_config->scheduleFile.value());
    }
    CopyInputFileToOutPath(_config->sourceFile);
    CopyInputFileToOutPath(_config->trainTimeTableFile);
    CopyInputFileToOutPath(_config->trainTypeFile);
}

void Simulation::CopyInputFileToOutPath(fs::path file)
{
    if(!file.empty() && fs::exists(file)) {
        fs::copy(file, _config->outputPath, fs::copy_options::overwrite_existing);
    }
}

void Simulation::UpdateOutputFiles()
{
    UpdateOutputIniFile();
    UpdateOutputGeometryFile();
}

void Simulation::UpdateOutputIniFile()
{
    fs::path iniOutputPath = _config->outputPath / _config->iniFile.filename();

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
            ->SetValue(_config->geometryFile.filename().string());
    } else if(
        mainNode->FirstChild("header") && mainNode->FirstChild("header")->FirstChild("geometry")) {
        mainNode->FirstChild("header")
            ->FirstChild("geometry")
            ->FirstChild()
            ->SetValue(_config->geometryFile.filename().string());
    }

    // update new traffic constraint file name
    if(!_config->trafficContraintFile.empty()) {
        if(mainNode->FirstChild("traffic_constraints") &&
           mainNode->FirstChild("traffic_constraints")->FirstChild("file")) {
            mainNode->FirstChild("traffic_constraints")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->trafficContraintFile.filename().string());
        }
    }

    // update new goal file name
    if(!_config->goalFile.empty()) {
        if(mainNode->FirstChild("routing") &&
           mainNode->FirstChild("routing")->FirstChild("goals") &&
           mainNode->FirstChild("routing")->FirstChild("goals")->FirstChild("file")) {
            mainNode->FirstChild("routing")
                ->FirstChild("goals")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->goalFile.filename().string());
        }
    }

    // update new source file name
    if(!_config->sourceFile.empty()) {
        if(mainNode->FirstChild("agents") &&
           mainNode->FirstChild("agents")->FirstChild("agents_sources") &&
           mainNode->FirstChild("agents")->FirstChild("agents_sources")->FirstChild("file")) {
            mainNode->FirstChild("agents")
                ->FirstChild("agents_sources")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->sourceFile.filename().string());
        }
    }

    // update new event file name
    if(_config->eventFile) {
        if(mainNode->FirstChild("events_file")) {
            mainNode->FirstChild("events_file")
                ->FirstChild()
                ->SetValue(_config->eventFile.value().filename().string());
        } else if(
            mainNode->FirstChild("header") &&
            mainNode->FirstChild("header")->FirstChild("events_file")) {
            mainNode->FirstChild("header")
                ->FirstChild("events_file")
                ->FirstChild()
                ->SetValue(_config->eventFile.value().filename().string());
        }
    }

    // update new schedule file name
    if(_config->scheduleFile) {
        if(mainNode->FirstChild("schedule_file")) {
            mainNode->FirstChild("schedule_file")
                ->FirstChild()
                ->SetValue(_config->scheduleFile.value().filename().string());

        } else if(
            mainNode->FirstChild("header") &&
            mainNode->FirstChild("header")->FirstChild("schedule_file")) {
            mainNode->FirstChild("header")
                ->FirstChild("schedule_file")
                ->FirstChild()
                ->SetValue(_config->scheduleFile.value().filename().string());
        }
    }

    // update new train time table file name
    if(!_config->trainTimeTableFile.empty()) {
        if(mainNode->FirstChild("train_constraints") &&
           mainNode->FirstChild("train_constraints")->FirstChild("train_time_table")) {
            mainNode->FirstChild("train_constraints")
                ->FirstChild("train_time_table")
                ->FirstChild()
                ->SetValue(_config->trainTimeTableFile.filename().string());
        }
    }

    // update new train types file name
    if(!_config->trainTypeFile.empty()) {
        if(mainNode->FirstChild("train_constraints") &&
           mainNode->FirstChild("train_constraints")->FirstChild("train_types")) {
            mainNode->FirstChild("train_constraints")
                ->FirstChild("train_types")
                ->FirstChild()
                ->SetValue(_config->trainTypeFile.filename().string());
        }
    }

    iniOutput.SaveFile(iniOutputPath.string());
}

void Simulation::UpdateOutputGeometryFile()
{
    fs::path geoOutputPath = _config->outputPath / _config->geometryFile.filename();

    TiXmlDocument geoOutput(geoOutputPath.string());

    if(!geoOutput.LoadFile()) {
        LOG_ERROR("Could not parse the ini file.");
        LOG_ERROR("{}", geoOutput.ErrorDesc());
    }
    TiXmlElement * mainNode = geoOutput.RootElement();

    if(!_config->transitionFile.empty()) {
        if(mainNode->FirstChild("transitions") &&
           mainNode->FirstChild("transitions")->FirstChild("file")) {
            mainNode->FirstChild("transitions")
                ->FirstChild("file")
                ->FirstChild()
                ->SetValue(_config->transitionFile.filename().string());
        }
    }

    geoOutput.SaveFile(geoOutputPath.string());
}
