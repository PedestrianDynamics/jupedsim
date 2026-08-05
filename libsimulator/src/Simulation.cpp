// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"

#include "GenericAgent.hpp"
#include "IteratorPair.hpp"
#include "Journey.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "RoutingEngine.hpp"
#include "SimulationClock.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "Tracing.hpp"
#include "Visitor.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace
{
/// RAII setter for Simulation::_iterating: set on construction, cleared on scope exit
/// (including exception unwinding out of the iteration pipeline).
class IterationScope
{
    bool& _flag;

public:
    explicit IterationScope(bool& flag) : _flag(flag) { _flag = true; }
    ~IterationScope() { _flag = false; }
    IterationScope(const IterationScope&) = delete;
    IterationScope& operator=(const IterationScope&) = delete;
};

/// Selects routing engine based on availability of 2D polygon.
/// For backwards compatibility reason we use TA*+funnel in 2D case.
std::unique_ptr<RoutingEngine3D> pick_routing_engine(const Geometry3D& geometry)
{
    if(const auto* flat = geometry.geometry_2d(); flat != nullptr) {
        return std::make_unique<RoutingEngine>(flat->Polygon());
    }
    return std::make_unique<SurfaceMeshShortestPathRoutingEngine>(geometry);
}
} // namespace

void Simulation::ThrowIfIterating(const char* operation) const
{
    if(_iterating) {
        throw SimulationError(
            "{} is not allowed during iteration, e.g. from a custom model callback while "
            "Iterate() is running",
            operation);
    }
}

Simulation::Simulation(
    std::unique_ptr<OperationalModel>&& operationalModel,
    std::unique_ptr<Geometry3D>&& geometry,
    double dT)
    : _clock(dT)
    , _operationalDecisionSystem(std::move(operationalModel))
    , _geometry(std::move(geometry))
    , _routingEngine(pick_routing_engine(*_geometry))
{
}

Simulation::Simulation(
    std::unique_ptr<OperationalModel>&& operationalModel,
    std::unique_ptr<Geometry2D>&& geometry,
    double dT)
    : Simulation(std::move(operationalModel), std::make_unique<Geometry3D>(geometry->Polygon()), dT)
{
}

const SimulationClock& Simulation::Clock() const
{
    return _clock;
}

void Simulation::SetTracing(bool status)
{
    if(status) {
        Profiler::instance().enable();
    } else {
        Profiler::instance().disable();
    }
};

void Simulation::Iterate()
{
    ThrowIfIterating("Iterate");
    IterationScope iterationScope(_iterating);
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Total Iteration", General);

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agent Removal System", Detailed);
        _agentRemovalSystem.Run(_agents, _removedAgentsInLastIteration, _stageManager);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Neighborhood Search", Detailed);
        _neighborhoodSearch.Update(_agents);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Stage System", Detailed);
        _stageSystem.Run(_stageManager, _neighborhoodSearch, *_geometry);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Strategical Decision System", General);
        _stategicalDecisionSystem.Run(_journeys, _agents, _stageManager);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Tactical Decision System", General);
        _tacticalDecisionSystem.Run(*_routingEngine, _agents);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Operational Decision System", General);
        _operationalDecisionSystem.Run(
            _clock.dT(), _clock.ElapsedTime(), _neighborhoodSearch, *_geometry, _agents);
        // Agents moved during the operational step; rebuild the grid so cell membership
        // reflects the new positions for queries before the next iteration (AgentsInRange,
        // AddAgent validation).
        _neighborhoodSearch.Update(_agents);
    }
    _clock.Advance();
}

Journey::ID Simulation::AddJourney(const std::map<BaseStage::ID, TransitionDescription>& stages)
{
    ThrowIfIterating("AddJourney");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Journey", Detailed);
    std::map<BaseStage::ID, JourneyNode> nodes;
    bool containsDirectSteering =
        std::find_if(std::begin(stages), std::end(stages), [this](auto const& pair) {
            return std::holds_alternative<DirectSteeringProxy>(Stage(pair.first));
        }) != std::end(stages);

    if(containsDirectSteering && stages.size() > 1) {
        throw SimulationError(
            "Journeys containing a DirectSteeringStage, may only contain this stage.");
    }

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

BaseStage::ID Simulation::AddStage(const StageDescription stageDescription, double z_hint)
{
    ThrowIfIterating("AddStage");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Stage", Detailed);
    return _stageManager.AddStage(
        stageDescription, _removedAgentsInLastIteration, *_geometry, z_hint);
}

GenericAgent::ID Simulation::AddAgent(
    Journey::ID journeyId,
    BaseStage::ID stageId,
    Point position,
    OperationalModelState model,
    double z_hint)
{
    ThrowIfIterating("AddAgent");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Agent", Detailed);
    const auto location = _geometry->get_location(position.x, position.y, z_hint);
    if(!location) {
        throw SimulationError("Agent {} not inside walkable area", position);
    }
    if(_journeys.count(journeyId) == 0) {
        throw SimulationError("Unknown journey id: {}", journeyId);
    }

    if(!_journeys.at(journeyId)->ContainsStage(stageId)) {
        throw SimulationError("Unknown stage id: {}", stageId);
    }

    if(const auto agentModelType = ModelTypeOf(model);
       agentModelType != _operationalDecisionSystem.ModelType()) {
        throw SimulationError(
            "Agent model data of type '{}' does not match the simulation's operational model "
            "'{}'",
            ToString(agentModelType),
            ToString(_operationalDecisionSystem.ModelType()));
    }

    GenericAgent agent{GenericAgent::ID::Invalid, journeyId, stageId, *location, std::move(model)};

    _operationalDecisionSystem.ValidateAgent(agent, _neighborhoodSearch, *_geometry);

    _stageManager.HandleNewAgent(agent.stageId);
    _agents.emplace_back(std::move(agent));
    _neighborhoodSearch.AddAgent(_agents.back());

    auto v = IteratorPair(std::prev(std::end(_agents)), std::end(_agents));
    _stategicalDecisionSystem.Run(_journeys, v, _stageManager);
    _tacticalDecisionSystem.Run(*_routingEngine, v);
    return _agents.back().id.getID();
}

void Simulation::SetAgentTarget(GenericAgent::ID id, Point target)
{
    auto& agent = Agent(id);
    const auto located = _geometry->get_location(
        target.x, target.y, agent.location.z(), std::numeric_limits<double>::max());
    if(!located) {
        throw SimulationError("Point {} is outside of accessible area", target);
    }
    agent.finalTarget = *located;
}

void Simulation::MarkAgentForRemoval(GenericAgent::ID id)
{
    ThrowIfIterating("MarkAgentForRemoval");
    JPS_TRACE_FUNC;
    const auto iter = std::find_if(
        std::begin(_agents), std::end(_agents), [id](auto& agent) { return agent.id == id; });
    if(iter == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }

    _removedAgentsInLastIteration.push_back(id);
}

const GenericAgent& Simulation::Agent(GenericAgent::ID id) const
{
    JPS_TRACE_FUNC;
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

GenericAgent& Simulation::Agent(GenericAgent::ID id)
{
    JPS_TRACE_FUNC;
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

AgentContainer<GenericAgent>& Simulation::Agents()
{
    return _agents;
};

void Simulation::SwitchAgentJourney(
    GenericAgent::ID agent_id,
    Journey::ID journey_id,
    BaseStage::ID stage_id)
{
    ThrowIfIterating("SwitchAgentJourney");
    JPS_TRACE_FUNC;
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
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Range", Debug);
    std::vector<GenericAgent::ID> neighborIds{};
    _neighborhoodSearch.ForEachInRange(p, distance, [&neighborIds](const GenericAgent& agent) {
        neighborIds.push_back(agent.id);
    });
    return neighborIds;
}

std::vector<GenericAgent::ID> Simulation::AgentsInPolygon(const std::vector<Point>& polygon)
{
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Polygon", Debug);
    const Polygon poly{polygon};
    if(!poly.IsConvex()) {
        throw SimulationError("Polygon needs to be simple and convex");
    }
    const auto [p, dist] = poly.ContainingCircle();

    std::vector<GenericAgent::ID> result{};
    _neighborhoodSearch.ForEachInRange(p, dist, [&result, &poly](const GenericAgent& agent) {
        if(poly.IsInside(agent.location.xy())) {
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
Geometry2D Simulation::Geo() const
{
    const auto* flat = _geometry->geometry_2d();
    if(flat == nullptr) {
        throw SimulationError(
            "This simulation was built from a surface mesh, which has no polygon underneath.");
    }
    return *flat;
}

void Simulation::PushTimer(const std::string_view name, size_t probe_log_level)
{
    _timer.pushTimerProbe(name, probe_log_level);
}

void Simulation::PopTimer(const std::string_view name)
{
    _timer.popTimerProbe(name);
}

TimerEntry::duration_type Simulation::GetTimerDuration(const std::string_view name) const
{
    return _timer.getDuration(name);
}

std::map<std::string, TimerEntry::duration_type> Simulation::GetTimerDurations() const
{
    return _timer.getDurations();
}
