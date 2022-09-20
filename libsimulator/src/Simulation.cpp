/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"

#include "Agent.hpp"
#include "Journey.hpp"
#include "OperationalModel.hpp"
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

uint64_t Simulation::AddAgent(
    const Point& position,
    const Point& orientation,
    double Av,
    double AMin,
    double BMax,
    double BMin,
    Journey::ID journeyId,
    OperationalModel::ParametersID profileId)
{
    auto agent = std::make_unique<Agent>();
    const auto orientationNormalised = orientation.Normalized();

    Ellipse e{};
    e.SetAv(Av);
    e.SetAmin(AMin);
    e.SetBmax(BMax);
    e.SetBmin(BMin);
    agent->orientation = orientationNormalised;
    agent->ellipse = e;
    agent->pos = position;
    agent->parameterProfileId = profileId;
    // TODO(kkratz): Replace later
    agent->radius = BMax;

    if(const auto& iter = _journeys.find(journeyId); iter != _journeys.end()) {
        agent->behaviour = std::make_unique<FollowWaypointsBehaviour>(
            dynamic_cast<SimpleJourney*>(iter->second.get()));
    } else {
        throw std::runtime_error(fmt::format("Unknown journey id: {}", journeyId));
    }

    _agents.emplace_back(std::move(agent));
    return _agents.back()->id.getID();
}

void Simulation::RemoveAgent(uint64_t id)
{
    const auto iter = std::remove_if(std::begin(_agents), std::end(_agents), [id](auto& agent) {
        return agent->id.getID() == id;
    });
    if(iter == std::end(_agents)) {
        throw std::runtime_error(fmt::format("Unknown agent id {}", id));
    }
    _agents.erase(iter, std::end(_agents));
}

Agent* Simulation::AgentPtr(Agent::ID id) const
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped->id; });
    if(iter == _agents.end()) {
        throw std::logic_error(fmt::format("Trying to access unknown Agent {}", id));
    }
    return iter->get();
}

const std::vector<uint64_t>& Simulation::RemovedAgents() const
{
    return _removedAgentsInLastIteration;
}

size_t Simulation::AgentCount() const
{
    return _agents.size();
}

void Simulation::SwitchAgentProfile(Agent::ID agent_id, OperationalModel::ParametersID profile_id)
    const
{
    _operationalDecisionSystem.ValidateAgentParameterProfileId(profile_id);
    if(auto agent = AgentPtr(agent_id); agent != nullptr) {
        agent->parameterProfileId = profile_id;
    } else {
        throw std::runtime_error(fmt::format("Unknown agent id={} supplied", agent_id.getID()));
    }
}
