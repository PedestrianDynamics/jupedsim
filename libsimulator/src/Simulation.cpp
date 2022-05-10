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

#include "OperationalModel.hpp"
#include "Pedestrian.hpp"
#include "SimulationClock.hpp"

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
    std::unique_ptr<OperationalModel> operationalModel,
    std::unique_ptr<Geometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine,
    std::map<Area::Id, Area> areas,
    double dT)
    : _clock(dT)
    , _operationalDecisionSystem(std::move(operationalModel))
    , _routingEngine(std::move(routingEngine))
    , _geometry(std::move(geometry))
    , _areas(std::move(areas))
{
    // TODO(kkratz): Ensure all areas are fully contained inside the walkable area. Otherwise an
    // agent may try to navigate to a point outside the navigation mesh, resulting in an exception.
}

void Simulation::Iterate()
{
    _neighborhoodSearch.Update(_agents);
    _agentExitSystem.Run(_areas, _agents);
    _stategicalDecisionSystem.Run(_areas, _agents);
    _tacticalDecisionSystem.Run(_areas, *_routingEngine, _agents);
    _operationalDecisionSystem.Run(
        _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);

    _eventProcessed = false;
    _clock.Advance();
    LOG_DEBUG("Iteration done.");
}

void Simulation::AddAgent(std::unique_ptr<Pedestrian>&& agent)
{
    // TODO(kkratz): this should be done by the tac-lvl
    const Point target{};
    const Point pos = agent->GetPos();
    // Compute orientation
    const Point posToTarget = target - pos;
    const Point orientation = posToTarget.Normalized();
    agent->InitV0(target);
    agent->SetDeltaT(_clock.dT());

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

size_t Simulation::AgentCount() const
{
    return _agents.size();
}

bool Simulation::InitArgs()
{
    return true;
}
