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

#include <Logger.hpp>
#include <algorithm>
#include <chrono>
#include <fmt/core.h>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tinyxml.h>
#include <variant>

Simulation::Simulation(
    Configuration* args,
    std::unique_ptr<Building>&& building,
    std::unique_ptr<Geometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine)
    : _config(args)
    , _clock(_config->dT)
    , _operationalDecisionSystem(OperationalModel::CreateFromType(args->operationalModel, *args))
    , _neighborhoodSearch(_config->linkedCellSize)
    , _building(std::move(building))
    , _routingEngine(std::move(routingEngine))
    , _geometry(std::move(geometry))
    , _areas(std::move(_config->areas))
{
    // TODO(kkratz): Ensure all areas are fully contained inside the walkable area. Otherwise an
    // agent may try to navigate to a point outside the navigation mesh, resulting in an exception.
}

void Simulation::Iterate()
{
    _neighborhoodSearch.Update(_agents);
    _agentExitSystem.Run(_areas, _agents);

    // TODO(kkratz): This currently mirrors the door state of the
    // old representation into the new one.
    // Intercepting external door events is not an issue, what is
    // problematic however is that our old door models (Crossings/Transitions)
    // implement flow control and close themselves.
    for(const auto& [id, t] : _building->GetAllTransitions()) {
        _geometry->UpdateDoorState(id, t->GetState());
    }

    _stategicalDecisionSystem.Run(_areas, _agents);
    _tacticalDecisionSystem.Run(_areas, *_routingEngine, _agents);
    _operationalDecisionSystem.Run(
        _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);

    if(_clock.ElapsedTime() > Pedestrian::GetMinPremovementTime()) {
        UpdateLocations();
    }
    _eventProcessed = false;
    _clock.Advance();
    LOG_DEBUG("Iteration done.");
}

void Simulation::AddAgent(std::unique_ptr<Pedestrian>&& agent)
{
    agent->SetBuilding(_building.get());
    // TODO(kkratz): this should be done by the tac-lvl
    const Point target{};
    const Point pos = agent->GetPos();
    // Compute orientation
    const Point posToTarget = target - pos;
    const Point orientation = posToTarget.Normalized();
    agent->InitV0(target);

    JEllipse E = agent->GetEllipse();
    E.SetCosPhi(orientation.x);
    E.SetSinPhi(orientation.y);
    agent->SetEllipse(E);
    _agents.emplace_back(std::move(agent));
}

void Simulation::AddAgents(std::vector<std::unique_ptr<Pedestrian>>&& agents)
{
    for(auto&& agent : agents) {
        AddAgent(std::move(agent));
    }
}

void Simulation::RemoveAgents(std::vector<Pedestrian::UID> ids)
{
    _agents.erase(
        std::remove_if(
            _agents.begin(),
            _agents.end(),
            [&ids](auto& agent) {
                const auto uid = agent->GetUID();
                return std::find_if(ids.begin(), ids.end(), [uid](Pedestrian::UID other) {
                           return uid == other;
                       }) != ids.end();
            }),
        _agents.end());
}

Pedestrian& Simulation::Agent(Pedestrian::UID id) const
{
    const auto iter = std::find_if(
        _agents.begin(), _agents.end(), [id](auto& ped) { return id == ped->GetUID(); });
    if(iter == _agents.end()) {
        throw std::logic_error("Trying to access unknown Agent.");
    }
    return **iter;
}

const std::vector<std::unique_ptr<Pedestrian>>& Simulation::Agents() const
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
    const TrainType& type,
    double startOffset,
    bool reversed)
{
    geometry::helper::AddTrainDoors(
        trainId, trackId, *_building, type, startOffset, reversed, *_geometry);
    _eventProcessed = true;
};

void Simulation::DeactivateTrain(int trainId, int trackId)
{
    const auto track = _building->GetTrack(trackId);
    if(!track) {
        throw std::runtime_error(
            fmt::format(FMT_STRING("Could not find track with ID {:d}"), trackId));
    }

    const auto roomID = track->_roomID;
    const auto subroomID = track->_subRoomID;
    auto* subroom = _building->GetRoom(roomID)->GetSubRoom(subroomID);

    // remove temp added walls
    const auto tempAddedWalls = _building->GetTrainWallsAdded(trainId);
    if(tempAddedWalls) {
        std::for_each(
            std::begin(tempAddedWalls.value()),
            std::end(tempAddedWalls.value()),
            [&subroom, this](const Wall& wall) {
                subroom->RemoveWall(wall);
                _geometry->RemoveLineSegment(wall);
            });

        _building->ClearTrainWallsAdded(trainId);
    }

    // add removed walls
    const auto tempRemovedWalls = _building->GetTrainWallsRemoved(trainId);
    if(tempRemovedWalls) {
        std::for_each(
            std::begin(tempRemovedWalls.value()),
            std::end(tempRemovedWalls.value()),
            [&subroom, this](const Wall& wall) {
                subroom->AddWall(wall);
                _geometry->AddLineSegment(wall);
            });

        _building->ClearTrainWallsRemoved(trainId);
    }

    // remove added doors
    const auto tempDoors = _building->GetTrainDoorsAdded(trainId);
    if(tempDoors) {
        std::for_each(
            std::begin(tempDoors.value()),
            std::end(tempDoors.value()),
            [&subroom, this](const Transition& door) {
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
    _fps = _config->fps;

    // Give the DirectionStrategy the chance to perform some initialization.
    // This should be done after the initialization of the operationalModel
    // because then, invalid pedestrians have been deleted and FindExit()
    // has been called.

    // other initializations
    for(auto&& ped : _agents) {
        ped->SetDeltaT(_clock.dT());
    }
    _seed = _config->seed;

    if(_config->distEffMaxPed > _config->linkedCellSize) {
        LOG_ERROR(
            "The linked-cell size [{}] should be larger than the force range [{}]",
            _config->linkedCellSize,
            _config->distEffMaxPed);
        return false;
    }
    UpdateLocations();

    return true;
}

void Simulation::UpdateLocations()
{
    SimulationHelper::UpdateFlowAtDoors(*_building, _agents, _clock.ElapsedTime());
    auto pedsOutside = SimulationHelper::FindPedestriansOutside(*_building, _agents);
    RemoveAgents(pedsOutside);

    // TODO discuss simulation flow -> better move to main loop, does not belong here
    //  TODO(kkratz): Notify new routing engine of potential updates / need to rebuild navigation
    //  graph
    bool geometryChangedFlow = SimulationHelper::UpdateFlowRegulation(*_building, _clock);
    bool geometryChangedTrain =
        SimulationHelper::UpdateTrainFlowRegulation(*_building, _clock.ElapsedTime());
    (void) geometryChangedFlow;
    (void) geometryChangedTrain;
}

void Simulation::PrintStatistics(double simTime)
{
    LOG_INFO("Usage of Exits");
    for(const auto& itr : _building->GetAllTransitions()) {
        Transition* goal = itr.second;
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
                statOutput.Write(fmt::format(FMT_STRING("#Simulation time: :.2f"), simTime));
                statOutput.Write(
                    "#Flow at exit " + goal->GetCaption() + "( ID " +
                    std::to_string(goal->GetID()) + " )");
                statOutput.Write("#Time (s), cummulative number of agents, pedestrian ID\n");
                statOutput.Write(goal->GetFlowCurve());
            }
        }
    }

    LOG_INFO("Usage of Crossings");
    for(const auto& itr : _building->GetAllCrossings()) {
        Crossing* goal = itr.second;
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
            output.Write(fmt::format(FMT_STRING("#Simulation time: :.2f"), simTime));
            output.Write(
                "#Flow at crossing " + goal->GetCaption() + "( ID " +
                std::to_string(goal->GetID()) + " ) in Room ( ID " +
                std::to_string(itr.first / 1000) + " )");
            output.Write("#Time (s)  cummulative number of agents \n");
            output.Write(goal->GetFlowCurve());
        }
    }
}
