// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"
#include "CollisionFreeSpeedModelData.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometrySwitchError.hpp"
#include "IteratorPair.hpp"
#include "Logger.hpp"
#include "OperationalModel.hpp"
#include "Stage.hpp"
#include "Visitor.hpp"
#include <memory>

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
    // LOG_DEBUG("Iteration {} / Time {}s", _clock.Iteration(), _clock.ElapsedTime());
    auto t = _perfStats.TraceIterate();
    _agentRemovalSystem.Run(_agents, _removedAgentsInLastIteration, _stageManager);
    _neighborhoodSearch.Update(_agents);

    _stageSystem.Run(_stageManager, _neighborhoodSearch);
    _stategicalDecisionSystem.Run(_journeys, _agents, _stageManager);
    _tacticalDecisionSystem.Run(*_routingEngine, _agents);
    {
        auto t2 = _perfStats.TraceOperationalDecisionSystemRun();
        _operationalDecisionSystem.Run(
            _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);
    }
    _clock.Advance();
}

Journey::ID Simulation::AddJourney(const std::map<BaseStage::ID, TransitionDescription>& stages)
{
    std::map<BaseStage::ID, JourneyNode> nodes;

    std::transform(
        std::begin(stages),
        std::end(stages),
        std::inserter(nodes, std::end(nodes)),
        [this](auto const& pair) -> std::pair<BaseStage::ID, JourneyNode> {
            const auto& [id, desc] = pair;
            auto stage = _stageManager.Stage(id);
            return {
                id,
                JourneyNode{
                    stage,
                    std::visit(
                        overloaded{
                            [stage](
                                const NonTransitionDescription&) -> std::unique_ptr<Transition> {
                                return std::make_unique<FixedTransition>(stage);
                            },
                            [this](const FixedTransitionDescription& d)
                                -> std::unique_ptr<Transition> {
                                return std::make_unique<FixedTransition>(
                                    _stageManager.Stage(d.NextId()));
                            },
                            [this](const RoundRobinTransitionDescription& d)
                                -> std::unique_ptr<Transition> {
                                std::vector<std::tuple<BaseStage*, uint64_t>> weightedStages{};
                                weightedStages.reserve(d.WeightedStages().size());

                                std::transform(
                                    std::begin(d.WeightedStages()),
                                    std::end(d.WeightedStages()),
                                    std::back_inserter(weightedStages),
                                    [this](auto const& pair) -> std::tuple<BaseStage*, uint64_t> {
                                        const auto& [id, weight] = pair;
                                        return {_stageManager.Stage(id), weight};
                                    });

                                return std::make_unique<RoundRobinTransition>(weightedStages);
                            },
                            [this](const LeastTargetedTransitionDescription& d)
                                -> std::unique_ptr<Transition> {
                                std::vector<BaseStage*> candidates{};
                                candidates.reserve(d.TargetCandidates().size());

                                std::transform(
                                    std::begin(d.TargetCandidates()),
                                    std::end(d.TargetCandidates()),
                                    std::back_inserter(candidates),
                                    [this](auto const& id) -> BaseStage* {
                                        return _stageManager.Stage(id);
                                    });

                                return std::make_unique<LeastTargetedTransition>(candidates);
                            }},
                        desc)}};
        });
    auto journey = std::make_unique<Journey>(std::move(nodes));
    const auto id = journey->Id();
    _journeys.emplace(id, std::move(journey));
    return id;
}

BaseStage::ID Simulation::AddStage(const StageDescription stageDescription)
{
    std::visit(
        overloaded{
            [this](const WaypointDescription& d) -> void {
                if(!this->_geometry->InsideGeometry(d.position)) {
                    throw SimulationError("WayPoint {} not inside walkable area", d.position);
                }
            },
            [this](const ExitDescription& d) -> void {
                if(!this->_geometry->InsideGeometry(d.polygon.Centroid())) {
                    throw SimulationError("Exit {} not inside walkable area", d.polygon.Centroid());
                }
            },
            [this](const NotifiableWaitingSetDescription& d) -> void {
                for(const auto& point : d.slots) {
                    if(!this->_geometry->InsideGeometry(point)) {
                        throw SimulationError(
                            "NotifiableWaitingSet point {} not inside walkable area", point);
                    }
                }
            },
            [this](const NotifiableQueueDescription& d) -> void {
                for(const auto& point : d.slots) {
                    if(!this->_geometry->InsideGeometry(point)) {
                        throw SimulationError(
                            "NotifiableQueue point {} not inside walkable area", point);
                    }
                }
            }},
        stageDescription);

    return _stageManager.AddStage(stageDescription, _removedAgentsInLastIteration);
}

GenericAgent::ID Simulation::AddAgent(GenericAgent&& agent)
{
    if(!_geometry->InsideGeometry(agent.pos)) {
        throw SimulationError("Agent {} not inside walkable area", agent.pos);
    }

    agent.orientation = agent.orientation.Normalized();
    _operationalDecisionSystem.ValidateAgent(agent, _neighborhoodSearch, *_geometry.get());

    if(_journeys.count(agent.journeyId) == 0) {
        throw SimulationError("Unknown journey id: {}", agent.journeyId);
    }

    if(!_journeys.at(agent.journeyId)->ContainsStage(agent.stageId)) {
        throw SimulationError("Unknown stage id: {}", agent.stageId);
    }
    _stageManager.HandleNewAgent(agent.stageId);
    _agents.emplace_back(std::move(agent));
    _neighborhoodSearch.AddAgent(_agents.back());

    auto v = IteratorPair(std::prev(std::end(_agents)), std::end(_agents));
    _stategicalDecisionSystem.Run(_journeys, v, _stageManager);
    _tacticalDecisionSystem.Run(*_routingEngine, v);
    return _agents.back().id.getID();
}

void Simulation::MarkAgentForRemoval(GenericAgent::ID id)
{
    const auto iter = std::find_if(
        std::begin(_agents), std::end(_agents), [id](auto& agent) { return agent.id == id; });
    if(iter == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }

    _removedAgentsInLastIteration.push_back(id);
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

std::vector<GenericAgent>& Simulation::Agents()
{
    return _agents;
};

void Simulation::SwitchAgentJourney(
    GenericAgent::ID agent_id,
    Journey::ID journey_id,
    BaseStage::ID stage_id)
{
    const auto find_iter = _journeys.find(journey_id);
    if(find_iter == std::end(_journeys)) {
        throw SimulationError("Unknown Journey id {}", journey_id);
    }
    auto& journey = find_iter->second;
    if(!journey->ContainsStage(stage_id)) {
        throw SimulationError("Stage {} not part of Journey {}", stage_id, journey_id);
    }
    auto& agent = Agent(agent_id);
    agent.journeyId = journey_id;
    _stageManager.MigrateAgent(agent.stageId, stage_id);
    agent.stageId = stage_id;
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

OperationalModelType Simulation::ModelType() const
{
    return _operationalDecisionSystem.ModelType();
}

StageProxy Simulation::Stage(BaseStage::ID stageId)
{
    return _stageManager.Stage(stageId)->Proxy(this);
}
Geometry Simulation::Geo() const
{
    return {std::make_unique<CollisionGeometry>(*_geometry), _routingEngine->Clone()};
}

void Simulation::SwitchGeometry(
    std::unique_ptr<CollisionGeometry>&& geometry,
    std::unique_ptr<RoutingEngine>&& routingEngine)
{
    ValidateGeometry(geometry);
    _geometry = std::move(geometry);
    _routingEngine = std::move(routingEngine);
}

void Simulation::ValidateGeometry(const std::unique_ptr<CollisionGeometry>& geometry) const
{
    std::vector<GenericAgent::ID> faultyAgents;
    for(const auto& agent : _agents) {
        if(const auto find_iter = std::find(
               std::begin(_removedAgentsInLastIteration),
               std::end(_removedAgentsInLastIteration),
               agent.id);
           find_iter != std::end(_removedAgentsInLastIteration)) {
            continue;
        }

        if(!geometry->InsideGeometry(agent.pos)) {
            faultyAgents.push_back(agent.id);
        }
    }

    std::vector<BaseStage::ID> faultyStages;
    for(const auto& [_, journey] : _journeys) {
        for(const auto& [stageId, node] : journey->Stages()) {

            if(auto exit = dynamic_cast<Exit*>(node.stage); exit != nullptr) {
                if(!geometry->InsideGeometry(exit->Position().Centroid())) {
                    faultyStages.push_back(stageId);
                }
            } else if(auto waypoint = dynamic_cast<Waypoint*>(node.stage); waypoint != nullptr) {
                if(!geometry->InsideGeometry(waypoint->Position())) {
                    faultyStages.push_back(stageId);
                }
            } else if(auto queue = dynamic_cast<NotifiableQueue*>(node.stage); queue != nullptr) {
                for(const auto& point : queue->Slots()) {
                    if(!geometry->InsideGeometry(point)) {
                        faultyStages.push_back(stageId);
                    }
                }
            } else if(auto waitingset = dynamic_cast<NotifiableWaitingSet*>(node.stage);
                      waitingset != nullptr) {
                for(const auto& point : waitingset->Slots()) {
                    if(!geometry->InsideGeometry(point)) {
                        faultyStages.push_back(stageId);
                    }
                }
            }
        }
    }

    if(!faultyAgents.empty() || !faultyStages.empty()) {
        std::string message = "Could not switch the geometry.\n";

        if(!faultyAgents.empty()) {
            message += fmt::format(
                "The following agents would be outside of the new geometry: {}\n",
                fmt::join(faultyAgents, ", "));
        }
        if(!faultyStages.empty()) {
            message += fmt::format(
                "The following stages would be outside of the new geometry: {}",
                fmt::join(faultyStages, ", "));
        }

        throw GeometrySwitchError(message.c_str(), faultyAgents, faultyStages);
    }
}
