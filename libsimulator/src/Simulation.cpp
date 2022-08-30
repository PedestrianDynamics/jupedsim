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

#include "Journey.hpp"
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
    std::unique_ptr<CollisionGeometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine,
    std::unique_ptr<Areas>&& areas,
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
    _agentExitSystem.Run(*_areas, _agents, _removedAgentsInLastIteration);
    _stategicalDecisionSystem.Run(*_areas, _agents);
    _tacticalDecisionSystem.Run(*_areas, *_routingEngine, _agents);
    _operationalDecisionSystem.Run(
        _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);

    _eventProcessed = false;
    _clock.Advance();
    LOG_DEBUG("Iteration done.");
}

Journey::ID Simulation::AddJourney(std::unique_ptr<Journey>&& journey)
{
    const auto id = journey->Id();
    _journeys.emplace(id, std::move(journey));
    return id;
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

uint64_t Simulation::AddAgent(
    const Point& position,
    const Point& orientation,
    double Av,
    double AMin,
    double BMax,
    double BMin,
    double Tau,
    double T,
    double v0,
    Journey::ID journeyId)
{
    auto agent = std::make_unique<Pedestrian>();
    agent->SetDeltaT(_clock.dT());

    const auto orientationNormalised = orientation.Normalized();

    JEllipse e{};
    e.SetAv(Av);
    e.SetAmin(AMin);
    e.SetBmax(BMax);
    e.SetBmin(BMin);
    e.SetCosPhi(orientationNormalised.x);
    e.SetSinPhi(orientationNormalised.y);
    agent->SetEllipse(e);
    agent->SetT(T);
    agent->SetPos(position);
    agent->SetV0Norm(v0, 0, 0, 0, 0);
    agent->SetTau(Tau);

    if(const auto& iter = _journeys.find(journeyId); iter != _journeys.end()) {
        agent->behaviour = std::make_unique<FollowWaypointsBehaviour>(
            dynamic_cast<SimpleJourney*>(iter->second.get()));
    } else {
        throw std::runtime_error(fmt::format("Unknown journey id: {}", journeyId));
    }

    _agents.emplace_back(std::move(agent));
    return _agents.back()->GetUID().getID();
}

void Simulation::AddAgents(std::vector<std::unique_ptr<Pedestrian>>&& agents)
{
    for(auto&& agent : agents) {
        AddAgent(std::move(agent));
    }
}

void Simulation::RemoveAgent(uint64_t id)
{
    const auto iter = std::remove_if(std::begin(_agents), std::end(_agents), [id](auto& agent) {
        return agent->GetUID().getID() == id;
    });
    if(iter == std::end(_agents)) {
        throw std::runtime_error(fmt::format("Unknown agent id {}", id));
    }
    _agents.erase(iter, std::end(_agents));
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

Pedestrian* Simulation::AgentPtr(Pedestrian::UID id) const
{
    const auto iter = std::find_if(
        _agents.begin(), _agents.end(), [id](auto& ped) { return id == ped->GetUID(); });
    if(iter == _agents.end()) {
        throw std::logic_error("Trying to access unknown Agent.");
    }
    return iter->get();
}

const std::vector<std::unique_ptr<Pedestrian>>& Simulation::Agents() const
{
    return _agents;
}

const std::vector<uint64_t>& Simulation::RemovedAgents() const
{
    return _removedAgentsInLastIteration;
}

size_t Simulation::AgentCount() const
{
    return _agents.size();
}
