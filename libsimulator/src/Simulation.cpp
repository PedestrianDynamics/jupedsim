// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"

#include "GenericAgent.hpp"
#include "IteratorPair.hpp"
#include "Journey.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SimulationClock.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
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
} // namespace

void Simulation::throw_if_iterating(const char* operation) const
{
    if(_iterating) {
        throw SimulationError(
            "{} is not allowed during iteration, e.g. from a custom model callback while "
            "Iterate() is running",
            operation);
    }
}

Simulation::Simulation(
    std::unique_ptr<OperationalModel>&& operational_model,
    std::unique_ptr<Geometry>&& geometry,
    double dt)
    : _clock(dt)
    , _operational_decision_system(std::move(operational_model))
    , _geometry(std::move(geometry))
    , _routing_engine(std::make_unique<RoutingEngine>(*_geometry))
{
}

const SimulationClock& Simulation::clock() const
{
    return _clock;
}

void Simulation::set_tracing(bool status)
{
    if(status) {
        Profiler::instance().enable();
    } else {
        Profiler::instance().disable();
    }
};

void Simulation::iterate()
{
    throw_if_iterating("Iterate");
    IterationScope iteration_scope(_iterating);
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Total Iteration", General);

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agent Removal System", Detailed);
        _agent_removal_system.run(_agents, _removed_agents_in_last_iteration, _stage_manager);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Neighborhood Search", Detailed);
        _neighborhood_search.update(_agents);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Strategical Decision System", General);
        _stategical_decision_system.run(_journeys, _agents, _stage_manager);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Tactical Decision System", General);
        _tactical_decision_system.run(*_routing_engine, _agents);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Operational Decision System", General);
        _operational_decision_system.run(
            _clock.dt(), _clock.elapsed_time(), _neighborhood_search, *_geometry, _agents);
        // Agents moved during the operational step; rebuild the grid so cell membership
        // reflects the new positions for queries before the next iteration (agents_in_range,
        // add_agent validation).
        _neighborhood_search.update(_agents);
    }
    _clock.advance();
}

Journey::ID Simulation::add_journey(const std::map<BaseStage::ID, TransitionDescription>& stages)
{
    throw_if_iterating("AddJourney");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Journey", Detailed);
    std::map<BaseStage::ID, JourneyNode> nodes;
    bool contains_direct_steering =
        std::find_if(std::begin(stages), std::end(stages), [this](auto const& pair) {
            return std::holds_alternative<DirectSteeringProxy>(stage(pair.first));
        }) != std::end(stages);

    if(contains_direct_steering && stages.size() > 1) {
        throw SimulationError(
            "Journeys containing a DirectSteeringStage, may only contain this stage.");
    }

    std::transform(
        std::begin(stages),
        std::end(stages),
        std::inserter(nodes, std::end(nodes)),
        [this](auto const& pair) -> std::pair<BaseStage::ID, JourneyNode> {
            const auto& [id, desc] = pair;
            auto stage = _stage_manager.stage(id);
            return {
                id,
                JourneyNode{
                    stage,
                    std::visit(
                        Overloaded{
                            [stage](
                                const NonTransitionDescription&) -> std::unique_ptr<Transition> {
                                return std::make_unique<FixedTransition>(stage);
                            },
                            [this](const FixedTransitionDescription& d)
                                -> std::unique_ptr<Transition> {
                                return std::make_unique<FixedTransition>(
                                    _stage_manager.stage(d.next_id()));
                            },
                            [this](const RoundRobinTransitionDescription& d)
                                -> std::unique_ptr<Transition> {
                                std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages{};
                                weighted_stages.reserve(d.weighted_stages().size());

                                std::transform(
                                    std::begin(d.weighted_stages()),
                                    std::end(d.weighted_stages()),
                                    std::back_inserter(weighted_stages),
                                    [this](auto const& pair) -> std::tuple<BaseStage*, uint64_t> {
                                        const auto& [id, weight] = pair;
                                        return {_stage_manager.stage(id), weight};
                                    });

                                return std::make_unique<RoundRobinTransition>(weighted_stages);
                            },
                            [this](const LeastTargetedTransitionDescription& d)
                                -> std::unique_ptr<Transition> {
                                std::vector<BaseStage*> candidates{};
                                candidates.reserve(d.target_candidates().size());

                                std::transform(
                                    std::begin(d.target_candidates()),
                                    std::end(d.target_candidates()),
                                    std::back_inserter(candidates),
                                    [this](auto const& id) -> BaseStage* {
                                        return _stage_manager.stage(id);
                                    });

                                return std::make_unique<LeastTargetedTransition>(candidates);
                            }},
                        desc)}};
        });

    auto journey = std::make_unique<Journey>(std::move(nodes));
    const auto id = journey->id();
    _journeys.emplace(id, std::move(journey));
    return id;
}

// Each stage checks its description and cuts its area before it registers the destination, so
// that a stage which cannot be added leaves nothing behind in the routing engine.
BaseStage::ID Simulation::add_stage(const StageDescription& stage_description)
{
    throw_if_iterating("AddStage");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Stage", Detailed);
    auto stage = std::visit(
        Overloaded{
            [this](const WaypointDescription& d) -> std::unique_ptr<BaseStage> {
                if(d.distance <= 0.0) {
                    throw SimulationError("Waypoint distance must be positive, got {}", d.distance);
                }
                const auto pieces = _geometry->split_into_region_pieces(
                    Polygon::from_circle(d.position, d.distance), d.region_id);
                return std::make_unique<Waypoint>(_routing_engine->register_destination(pieces));
            },
            [this](const ExitDescription& d) -> std::unique_ptr<BaseStage> {
                const auto pieces = _geometry->split_into_region_pieces(d.polygon, d.region_id);
                return std::make_unique<Exit>(
                    _routing_engine->register_destination(pieces),
                    _removed_agents_in_last_iteration);
            },
            [](const DirectSteeringDescription&) -> std::unique_ptr<BaseStage> {
                return std::make_unique<DirectSteering>();
            }},
        stage_description);
    return _stage_manager.add_stage(std::move(stage));
}

GenericAgent::ID Simulation::add_agent(
    Journey::ID journey_id,
    BaseStage::ID stage_id,
    Point position,
    OperationalModelState model,
    std::size_t region_id)
{
    throw_if_iterating("AddAgent");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Agent", Detailed);
    const auto location = _geometry->get_location(position.x, position.y, region_id);
    if(_journeys.count(journey_id) == 0) {
        throw SimulationError("Unknown journey id: {}", journey_id);
    }

    if(!_journeys.at(journey_id)->contains_stage(stage_id)) {
        throw SimulationError("Unknown stage id: {}", stage_id);
    }

    if(const auto agent_model_type = model_type_of(model);
       agent_model_type != _operational_decision_system.model_type()) {
        throw SimulationError(
            "Agent model data of type '{}' does not match the simulation's operational model "
            "'{}'",
            to_string(agent_model_type),
            to_string(_operational_decision_system.model_type()));
    }

    GenericAgent agent{GenericAgent::ID::invalid, journey_id, stage_id, location, std::move(model)};

    _operational_decision_system.validate_agent(agent, _neighborhood_search, *_geometry);

    _stage_manager.handle_new_agent(agent.stage_id);
    _agents.emplace_back(std::move(agent));
    _neighborhood_search.add_agent(_agents.back());

    auto v = IteratorPair(std::prev(std::end(_agents)), std::end(_agents));
    _stategical_decision_system.run(_journeys, v, _stage_manager);
    _tactical_decision_system.run(*_routing_engine, v);
    return _agents.back().id.get_id();
}

Location Simulation::get_location(double x, double y, std::size_t region_id) const
{
    return _geometry->get_location(x, y, region_id);
}

void Simulation::set_agent_target(GenericAgent::ID id, Point target)
{
    auto& agent = this->agent(id);
    const auto located = _geometry->get_location_near_z(
        target.x, target.y, agent.location.z(), std::numeric_limits<double>::max());
    if(!located) {
        throw SimulationError("Point {} is outside of accessible area", target);
    }
    agent.final_target = *located;
}

void Simulation::set_agent_target(GenericAgent::ID id, const Location& target)
{
    agent(id).final_target = target;
}

void Simulation::mark_agent_for_removal(GenericAgent::ID id)
{
    throw_if_iterating("MarkAgentForRemoval");
    JPS_TRACE_FUNC;
    const auto iter = std::find_if(
        std::begin(_agents), std::end(_agents), [id](auto& agent) { return agent.id == id; });
    if(iter == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }

    _removed_agents_in_last_iteration.push_back(id);
}

const GenericAgent& Simulation::agent(GenericAgent::ID id) const
{
    JPS_TRACE_FUNC;
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

GenericAgent& Simulation::agent(GenericAgent::ID id)
{
    JPS_TRACE_FUNC;
    const auto iter =
        std::find_if(_agents.begin(), _agents.end(), [id](auto& ped) { return id == ped.id; });
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

const std::vector<GenericAgent::ID>& Simulation::removed_agents() const
{
    return _removed_agents_in_last_iteration;
}

double Simulation::elapsed_time() const
{
    return _clock.elapsed_time();
}

double Simulation::dt() const
{
    return _clock.dt();
}

uint64_t Simulation::iteration() const
{
    return _clock.iteration();
}

size_t Simulation::agent_count() const
{
    return _agents.size();
}

AgentContainer<GenericAgent>& Simulation::agents()
{
    return _agents;
};

void Simulation::switch_agent_journey(
    GenericAgent::ID agent_id,
    Journey::ID journey_id,
    BaseStage::ID stage_id)
{
    throw_if_iterating("SwitchAgentJourney");
    JPS_TRACE_FUNC;
    const auto find_iter = _journeys.find(journey_id);
    if(find_iter == std::end(_journeys)) {
        throw SimulationError("Unknown Journey id {}", journey_id);
    }
    auto& journey = find_iter->second;
    if(!journey->contains_stage(stage_id)) {
        throw SimulationError("Stage {} not part of Journey {}", stage_id, journey_id);
    }
    auto& agent = this->agent(agent_id);
    agent.journey_id = journey_id;
    _stage_manager.migrate_agent(agent.stage_id, stage_id);
    agent.stage_id = stage_id;
}

std::vector<GenericAgent::ID> Simulation::agents_in_range(Point p, double distance)
{
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Range", Debug);
    std::vector<GenericAgent::ID> neighbor_ids{};
    _neighborhood_search.for_each_in_range(p, distance, [&neighbor_ids](const GenericAgent& agent) {
        neighbor_ids.push_back(agent.id);
    });
    return neighbor_ids;
}

std::vector<GenericAgent::ID> Simulation::agents_in_polygon(const std::vector<Point>& polygon)
{
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Polygon", Debug);
    const Polygon poly{polygon};
    if(!poly.is_convex()) {
        throw SimulationError("Polygon needs to be simple and convex");
    }
    const auto [p, dist] = poly.containing_circle();

    std::vector<GenericAgent::ID> result{};
    _neighborhood_search.for_each_in_range(p, dist, [&result, &poly](const GenericAgent& agent) {
        if(poly.is_inside(agent.location.xy())) {
            result.push_back(agent.id);
        }
    });
    return result;
}

OperationalModelType Simulation::model_type() const
{
    return _operational_decision_system.model_type();
}

StageProxy Simulation::stage(BaseStage::ID stage_id)
{
    return _stage_manager.stage(stage_id)->proxy(this);
}
const Geometry& Simulation::geo() const
{
    return *_geometry;
}

void Simulation::push_timer(const std::string_view name, size_t probe_log_level)
{
    _timer.push_timer_probe(name, probe_log_level);
}

void Simulation::pop_timer(const std::string_view name)
{
    _timer.pop_timer_probe(name);
}

TimerEntry::DurationType Simulation::get_timer_duration(const std::string_view name) const
{
    return _timer.get_duration(name);
}

std::map<std::string, TimerEntry::DurationType> Simulation::get_timer_durations() const
{
    return _timer.get_durations();
}
