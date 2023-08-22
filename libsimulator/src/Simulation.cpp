// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"
#include "GenericAgent.hpp"
#include "OperationalModel.hpp"

Simulation::Simulation(
    std::unique_ptr<OperationalModel>&& operationalModel,
    std::unique_ptr<CollisionGeometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine,
    double dT)
    : _clock(dT)
    , _operationalDecisionSystem(std::move(operationalModel))
    , _routingEngine(std::move(routingEngine))
    , _geometry(std::move(geometry))
{
    // TODO(kkratz): Ensure all areas are fully contained inside the walkable area. Otherwise an
    // agent may try to navigate to a point outside the navigation mesh, resulting in an exception.
}
const SimulationClock& Simulation::Clock() const
{
    return _clock;
}

void Simulation::SetTracing(bool status)
{
    _perfStats.SetEnabled(status);
};

PerfStats Simulation::GetLastStats() const
{
    return _perfStats;
};

void Simulation::Iterate()
{
    auto t = _perfStats.TraceIterate();
    _agentExitSystem.Run(_agents, _removedAgentsInLastIteration);
    _neighborhoodSearch.Update(_agents);

    for(auto& [_, stage] : _stages) {
        if(auto* updatable_stage = dynamic_cast<NotifiableWaitingSet*>(stage.get());
           updatable_stage != nullptr) {
            updatable_stage->Update(_neighborhoodSearch);
        } else if(auto* updatable_stage = dynamic_cast<NotifiableQueue*>(stage.get());
                  updatable_stage != nullptr) {
            updatable_stage->Update(_neighborhoodSearch);
        }
    }

    _stategicalDecisionSystem.Run(_journeys, _agents);
    _tacticalDecisionSystem.Run(*_routingEngine, _agents);
    {
        auto t2 = _perfStats.TraceOperationalDecisionSystemRun();
        _operationalDecisionSystem.Run(
            _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);
    }
    _clock.Advance();
}

Journey::ID Simulation::AddJourney(const std::vector<Stage::ID>& stageIds)
{
    std::vector<Stage*> stages;
    stages.reserve(stageIds.size());
    std::transform(
        std::begin(stageIds),
        std::end(stageIds),
        std::back_inserter(stages),
        [this](const auto id) {
            const auto iter = _stages.find(id);
            if(iter == std::end(_stages)) {
                throw SimulationError("Unknown stage id ({}) provided in journey.", id.getID());
            }
            return iter->second.get();
        });
    auto journey = std::make_unique<Journey>(std::move(stages));
    const auto id = journey->Id();
    _journeys.emplace(id, std::move(journey));
    return id;
}

Stage::ID Simulation::AddStage(const StageDescription stageDescription)
{
    std::unique_ptr<Stage> stage = std::visit(
        overloaded{
            [](const WaypointDescription& d) -> std::unique_ptr<Stage> {
                return std::make_unique<Waypoint>(d.position, d.distance);
            },
            [this](const ExitDescription& d) -> std::unique_ptr<Stage> {
                return std::make_unique<Exit>(d.polygon, _removedAgentsInLastIteration);
            },
            [](const NotifiableWaitingSetDescription& d) -> std::unique_ptr<Stage> {
                return std::make_unique<NotifiableWaitingSet>(d.slots);
            },
            [](const NotifiableQueueDescription& d) -> std::unique_ptr<Stage> {
                return std::make_unique<NotifiableQueue>(d.slots);
            }},
        stageDescription);
    if(_stages.find(stage->Id()) != _stages.end()) {
        throw SimulationError("Internal error, stage id already in use.");
    }
    const auto id = stage->Id();
    _stages.emplace(id, std::move(stage));
    return id;
}

GenericAgent::ID Simulation::AddAgent(GenericAgent&& agent)
{
    agent.orientation = agent.orientation.Normalized();
    _operationalDecisionSystem.ValidateAgent(agent, _neighborhoodSearch);

    if(_journeys.count(agent.journeyId) == 0) {
        throw SimulationError("Unknown journey id: {}", agent.journeyId);
    }

    _agents.emplace_back(std::move(agent));
    _neighborhoodSearch.AddAgent(_agents.back());

    return _agents.back().id.getID();
}

void Simulation::RemoveAgent(GenericAgent::ID id)
{
    const auto iter = std::find_if(
        std::begin(_agents), std::end(_agents), [id](auto& agent) { return agent.id == id; });
    if(iter == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }
    _agents.erase(iter);
    _neighborhoodSearch.RemoveAgent(*iter);
}

const GenericAgent& Simulation::Agent(GenericAgent::ID id) const
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

GenericAgent& Simulation::Agent(GenericAgent::ID id)
{
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

const std::vector<GenericAgent::ID>& Simulation::RemovedAgents() const
{
    return _removedAgentsInLastIteration;
}

double Simulation::ElapsedTime() const
{
    return _clock.ElapsedTime();
}

double Simulation::DT() const
{
    return _clock.dT();
}

uint64_t Simulation::Iteration() const
{
    return _clock.Iteration();
}

size_t Simulation::AgentCount() const
{
    return _agents.size();
}

const std::vector<GenericAgent>& Simulation::Agents() const
{
    return _agents;
};

void Simulation::SwitchAgentProfile(
    GenericAgent::ID agent_id,
    OperationalModel::ParametersID profile_id)
{
    _operationalDecisionSystem.ValidateAgentParameterProfileId(profile_id);
    Agent(agent_id).parameterProfileId = profile_id;
}

void Simulation::SwitchAgentJourney(
    GenericAgent::ID agent_id,
    Journey::ID journey_id,
    size_t stage_idx)
{
    const auto find_iter = _journeys.find(journey_id);
    if(find_iter == std::end(_journeys)) {
        throw SimulationError("Unknown Journey id {}", journey_id);
    }
    auto& journey = find_iter->second;
    if(stage_idx >= journey->CountStages()) {
        throw SimulationError("Stage index {} for journey {} out of range", stage_idx, journey_id);
    }
    auto& agent = Agent(agent_id);
    agent.journeyId = journey_id;
    agent.currentJourneyStageIdx = stage_idx;
    agent.stageId = journey->StageAt(stage_idx)->Id();
}

std::vector<GenericAgent::ID> Simulation::AgentsInRange(Point p, double distance)
{
    const auto neighbors = _neighborhoodSearch.GetNeighboringAgents(p, distance);

    std::vector<GenericAgent::ID> neighborIds{};
    neighborIds.reserve(neighbors.size());
    std::transform(
        std::begin(neighbors),
        std::end(neighbors),
        std::back_inserter(neighborIds),
        [](const auto& agent) { return agent.id; });
    return neighborIds;
}

std::vector<GenericAgent::ID> Simulation::AgentsInPolygon(const std::vector<Point>& polygon)
{
    const Polygon poly{polygon};
    if(!poly.IsConvex()) {
        throw SimulationError("Polygon needs to be simple and convex");
    }
    const auto [p, dist] = poly.ContainingCircle();

    const auto candidates = _neighborhoodSearch.GetNeighboringAgents(p, dist);
    std::vector<GenericAgent::ID> result{};
    result.reserve(candidates.size());
    std::for_each(
        std::begin(candidates), std::end(candidates), [&result, &poly](const auto& agent) {
            if(poly.IsInside(agent.pos)) {
                result.push_back(agent.id);
            }
        });
    return result;
}

void Simulation::Notify(Event evt)
{
    std::visit(
        overloaded{
            [this](const NotifyWaitingSet& evt) {
                auto* stage = dynamic_cast<NotifiableWaitingSet*>(_stages.at(evt.stageId).get());
                if(stage == nullptr) {
                    throw SimulationError(
                        "Stage id {} is not a NotiafiableWaitingSet", evt.stageId);
                }
                stage->State(evt.newState);
            },
            [this](const NotifyQueue& evt) {
                auto* stage = dynamic_cast<NotifiableQueue*>(_stages.at(evt.stageId).get());
                if(stage == nullptr) {
                    throw SimulationError("Stage id {} is not a NotiafiableQueue", evt.stageId);
                }
                stage->Pop(evt.count);
            }},
        evt);
}
OperationalModelType Simulation::ModelType() const
{
    return _operationalDecisionSystem.ModelType();
}
