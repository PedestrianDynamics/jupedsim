// SPDX-License-Identifier: LGPL-3.0-or-later
#include "simulation.hpp"

#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/location.hpp"
#include "iterator_pair.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"
#include "polygon.hpp"
#include "route.hpp"
#include "routing_engine.hpp"
#include "simulation_clock.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "strategical_desicion_system.hpp"
#include "tactical_decision_system.hpp"
#include "timing.hpp"
#include "tracing.hpp"
#include "visitor.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace
{
/// Binary search by id: agents are appended with increasing ids and removal keeps their order.
template <typename Container>
auto find_agent(Container& agents, GenericAgent::ID id)
{
    const auto iter = std::lower_bound(
        std::begin(agents), std::end(agents), id, [](const auto& agent, GenericAgent::ID value) {
            return agent.id < value;
        });
    if(iter == std::end(agents) || iter->id != id) {
        return std::end(agents);
    }
    return iter;
}

std::vector<AreaPiece> pieces_of(const Geometry& geometry, const std::vector<StageArea>& areas)
{
    if(areas.empty()) {
        throw SimulationError("A stage needs at least one area");
    }
    std::vector<AreaPiece> pieces{};
    for(const auto& area : areas) {
        const auto clipped = geometry.clip_to_region(area.polygon, area.region_id);
        pieces.insert(pieces.end(), clipped.begin(), clipped.end());
    }
    return pieces;
}

/// Starts @p agent at @p stage, which is its decision stage until a rule decides. A closed exit
/// strands it at once.
void start_at(GenericAgent& agent, const BaseStage& stage)
{
    agent.stranded = false;
    agent.held = false;
    agent.arrived = false;
    agent.decision_stage = stage.id();
    if(dynamic_cast<const DirectSteering*>(&stage) != nullptr) {
        agent.decision_stage = std::nullopt;
    }
    if(stage.closed()) {
        StrategicalDecisionSystem::strand(agent);
    }
}
} // namespace

void Simulation::throw_if_state(State forbidden, const char* operation) const
{
    if((_state & forbidden) != State{}) {
        throw SimulationError("{} is not allowed in state {}", operation, _state);
    }
}

Simulation::Simulation(
    std::unique_ptr<OperationalModel>&& operational_model,
    std::unique_ptr<Geometry>&& geometry,
    double dt,
    FloorfieldOptions floorfield_options)
    : _clock(dt)
    , _operational_decision_system(std::move(operational_model))
    , _geometry(std::move(geometry))
    , _routing_engine(std::make_unique<RoutingEngine>(*_geometry, floorfield_options))
{
}

const SimulationClock& Simulation::clock() const
{
    throw_if_state(State::Closed, "clock");
    return _clock;
}

void Simulation::set_tracing(bool status)
{
    throw_if_state(State::Closed, "set_tracing");
    if(status) {
        Profiler::instance().enable();
    } else {
        Profiler::instance().disable();
    }
};

void Simulation::iterate()
{
    throw_if_state(State::Iterating | State::Closed, "iterate");
    IterationScope iteration_scope(_state);
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Total Iteration", General);

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agent Removal System", Detailed);
        _agent_removal_system.run(
            _agents, _removed_agents_in_last_iteration, _marked_for_removal, _stage_manager);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Neighborhood Search", Detailed);
        _neighborhood_search.update(_agents);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Strategical Decision System", General);
        _stategical_decision_system.run(
            _routes, _agents, _stage_manager, *_routing_engine, _clock.iteration());
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

void Simulation::close()
{
    throw_if_state(State::Iterating, "close");
    _state = State::Closed;
    _routing_engine.reset();
}

Route::ID Simulation::add_route(const std::map<BaseStage::ID, RuleDescription>& stages)
{
    throw_if_state(State::Iterating | State::Closed, "add_route");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Route", Detailed);
    std::map<BaseStage::ID, RouteNode> nodes;
    bool contains_direct_steering =
        std::find_if(std::begin(stages), std::end(stages), [this](auto const& pair) {
            return dynamic_cast<const DirectSteering*>(stage(pair.first)) != nullptr;
        }) != std::end(stages);

    if(contains_direct_steering && stages.size() > 1) {
        throw SimulationError(
            "Routes containing a DirectSteeringStage, may only contain this stage.");
    }

    std::transform(
        std::begin(stages),
        std::end(stages),
        std::inserter(nodes, std::end(nodes)),
        [this](auto const& pair) -> std::pair<BaseStage::ID, RouteNode> {
            const auto& [id, desc] = pair;
            auto stage = _stage_manager.stage(id);
            return {
                id,
                RouteNode{
                    stage,
                    std::visit(
                        Overloaded{
                            [stage](const NoRuleDescription&) -> std::unique_ptr<Rule> {
                                return std::make_unique<FixedRule>(stage);
                            },
                            [stage](const StayDescription&) -> std::unique_ptr<Rule> {
                                return std::make_unique<Stay>(stage);
                            },
                            [this](const FixedRuleDescription& d) -> std::unique_ptr<Rule> {
                                return std::make_unique<FixedRule>(
                                    _stage_manager.stage(d.next_id()));
                            },
                            [this](const RoundRobinDescription& d) -> std::unique_ptr<Rule> {
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

                                return std::make_unique<RoundRobin>(weighted_stages);
                            },
                            [this](const LeastTargetedDescription& d) -> std::unique_ptr<Rule> {
                                std::vector<BaseStage*> candidates{};
                                candidates.reserve(d.target_candidates().size());

                                std::transform(
                                    std::begin(d.target_candidates()),
                                    std::end(d.target_candidates()),
                                    std::back_inserter(candidates),
                                    [this](auto const& id) -> BaseStage* {
                                        return _stage_manager.stage(id);
                                    });

                                return std::make_unique<LeastTargeted>(candidates);
                            },
                            [this](const LeastTravelTimeDescription& d) -> std::unique_ptr<Rule> {
                                std::vector<BaseStage*> candidates{};
                                candidates.reserve(d.target_candidates().size());

                                std::transform(
                                    std::begin(d.target_candidates()),
                                    std::end(d.target_candidates()),
                                    std::back_inserter(candidates),
                                    [this](auto const& id) -> BaseStage* {
                                        return _stage_manager.stage(id);
                                    });

                                return std::make_unique<LeastTravelTime>(candidates);
                            }},
                        desc)}};
        });

    auto route = std::make_unique<Route>(std::move(nodes));
    const auto id = route->id();
    _routes.emplace(id, std::move(route));
    return id;
}

BaseStage::ID Simulation::add_stage(const StageDescription& stage_description)
{
    throw_if_state(State::Iterating | State::Closed, "add_stage");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Stage", Detailed);
    auto stage = std::visit(
        Overloaded{
            [this](const WaypointDescription& d) -> std::unique_ptr<BaseStage> {
                const auto pieces = pieces_of(*_geometry, d.areas);
                return std::make_unique<Waypoint>(_routing_engine->register_destination(pieces));
            },
            [this](const ExitDescription& d) -> std::unique_ptr<BaseStage> {
                const auto pieces = pieces_of(*_geometry, d.areas);
                return std::make_unique<Exit>(
                    _routing_engine->register_destination(pieces),
                    _removed_agents_in_last_iteration,
                    _clock);
            },
            [](const DirectSteeringDescription&) -> std::unique_ptr<BaseStage> {
                return std::make_unique<DirectSteering>();
            }},
        stage_description);
    return _stage_manager.add_stage(std::move(stage));
}

GenericAgent::ID Simulation::add_agent(
    Route::ID route_id,
    BaseStage::ID stage_id,
    Point position,
    OperationalModelState model,
    std::size_t region_id)
{
    throw_if_state(State::Iterating | State::Closed, "add_agent");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Agent", Detailed);
    const auto location = _geometry->get_location(position.x(), position.y(), region_id);
    if(_routes.count(route_id) == 0) {
        throw SimulationError("Unknown route id: {}", route_id);
    }

    if(!_routes.at(route_id)->contains_stage(stage_id)) {
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

    GenericAgent agent{GenericAgent::ID::invalid, route_id, stage_id, location, std::move(model)};

    _operational_decision_system.validate_agent(agent, _neighborhood_search, *_geometry);
    start_at(agent, *_stage_manager.stage(stage_id));

    _stage_manager.handle_new_agent(agent.stage_id);
    _agents.emplace_back(std::move(agent));
    _neighborhood_search.add_agent(_agents.back());

    // Stages are completed in iterate(): an agent added inside its first stage targets it until
    // then, so holds placed before the first iterate() apply and exit records are not lost.
    auto& added = _agents.back();
    if(!added.stranded) {
        added.final_target = _stage_manager.stage(added.stage_id)->target(added);
    }
    auto v = IteratorPair(std::prev(std::end(_agents)), std::end(_agents));
    _tactical_decision_system.run(*_routing_engine, v);
    return added.id.get_id();
}

Location Simulation::get_location(double x, double y, std::size_t region_id) const
{
    throw_if_state(State::Closed, "get_location");
    return _geometry->get_location(x, y, region_id);
}

void Simulation::set_agent_target(GenericAgent::ID id, Point target)
{
    throw_if_state(State::Closed, "set_agent_target");
    auto& agent = this->agent(id);
    const auto located = _geometry->get_location_near_z(
        target.x(), target.y(), agent.location.z(), std::numeric_limits<double>::max());
    if(!located) {
        throw SimulationError("Point {} is outside of accessible area", target);
    }
    agent.final_target = *located;
    agent.arrived = false;
    // A step taken toward the previous target does not reach the new one.
    agent.last_step = Vector{};
    TacticalDecisionSystem::orient(*_routing_engine, agent);
}

void Simulation::set_agent_target(GenericAgent::ID id, const Location& target)
{
    throw_if_state(State::Closed, "set_agent_target");
    auto& steered = agent(id);
    steered.final_target = target;
    steered.arrived = false;
    // A step taken toward the previous target does not reach the new one.
    steered.last_step = Vector{};
    TacticalDecisionSystem::orient(*_routing_engine, steered);
}

void Simulation::mark_agent_for_removal(GenericAgent::ID id)
{
    throw_if_state(State::Iterating | State::Closed, "mark_agent_for_removal");
    JPS_TRACE_FUNC;
    const auto iter = find_agent(_agents, id);
    if(iter == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }

    if(is_leaving(id)) {
        return;
    }
    _marked_for_removal.push_back({id, std::nullopt, _clock.elapsed_time()});
}

const GenericAgent& Simulation::agent(GenericAgent::ID id) const
{
    throw_if_state(State::Closed, "agent");
    JPS_TRACE_FUNC;
    const auto iter = find_agent(_agents, id);
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

GenericAgent& Simulation::agent(GenericAgent::ID id)
{
    throw_if_state(State::Closed, "agent");
    JPS_TRACE_FUNC;
    const auto iter = find_agent(_agents, id);
    if(iter == _agents.end()) {
        throw SimulationError("Trying to access unknown Agent {}", id);
    }
    return *iter;
}

const std::vector<RemovalRecord>& Simulation::removed_agents() const
{
    throw_if_state(State::Closed, "removed_agents");
    return _removed_agents_in_last_iteration;
}

double Simulation::elapsed_time() const
{
    throw_if_state(State::Closed, "elapsed_time");
    return _clock.elapsed_time();
}

double Simulation::dt() const
{
    throw_if_state(State::Closed, "dt");
    return _clock.dt();
}

uint64_t Simulation::iteration() const
{
    throw_if_state(State::Closed, "iteration");
    return _clock.iteration();
}

size_t Simulation::agent_count() const
{
    throw_if_state(State::Closed, "agent_count");
    return _agents.size();
}

AgentContainer<GenericAgent>& Simulation::agents()
{
    throw_if_state(State::Closed, "agents");
    return _agents;
};

void Simulation::switch_agent_route(
    GenericAgent::ID agent_id,
    Route::ID route_id,
    BaseStage::ID stage_id)
{
    throw_if_state(State::Iterating | State::Closed, "switch_agent_route");
    JPS_TRACE_FUNC;
    const auto find_iter = _routes.find(route_id);
    if(find_iter == std::end(_routes)) {
        throw SimulationError("Unknown Route id {}", route_id);
    }
    auto& route = find_iter->second;
    if(!route->contains_stage(stage_id)) {
        throw SimulationError("Stage {} not part of Route {}", stage_id, route_id);
    }
    auto& agent = this->agent(agent_id);
    agent.route_id = route_id;
    _stage_manager.migrate_agent(agent.stage_id, stage_id);
    agent.stage_id = stage_id;
    auto& stage = *_stage_manager.stage(stage_id);
    start_at(agent, stage);
    // A stage's target is cheap to read, a point target is not: set_agent_target orients
    // steering agents.
    if(dynamic_cast<const DirectSteering*>(&stage) == nullptr) {
        if(!agent.stranded) {
            agent.final_target = stage.target(agent);
        }
        TacticalDecisionSystem::orient(*_routing_engine, agent);
    }
}

std::vector<GenericAgent::ID> Simulation::close_exit(BaseStage::ID exit_id, bool informed)
{
    throw_if_state(State::Iterating | State::Closed, "close_exit");
    auto& exit = exit_of(exit_id);
    if(exit.closed()) {
        return {};
    }
    exit.close();
    if(!informed) {
        return {};
    }
    std::vector<GenericAgent::ID> stranded{};
    for(auto& agent : _agents) {
        if(agent.stranded || agent.stage_id != exit_id || is_leaving(agent.id)) {
            continue;
        }
        auto* next = decide_again(agent);
        if(next == nullptr) {
            StrategicalDecisionSystem::strand(agent);
            stranded.push_back(agent.id);
        } else {
            StrategicalDecisionSystem::assign(agent, *next, _stage_manager);
        }
        TacticalDecisionSystem::orient(*_routing_engine, agent);
    }
    return stranded;
}

std::vector<GenericAgent::ID> Simulation::open_exit(BaseStage::ID exit_id)
{
    throw_if_state(State::Iterating | State::Closed, "open_exit");
    auto& exit = exit_of(exit_id);
    if(!exit.closed()) {
        return {};
    }
    exit.open();
    // Only agents whose rule targets the opened exit decide again: weights may have changed
    // since others were stranded, and those wait for an exit of their own rule.
    std::vector<GenericAgent::ID> resumed{};
    for(auto& agent : _agents) {
        if(!agent.stranded || !decides_for(agent, exit) || is_leaving(agent.id)) {
            continue;
        }
        auto* next = decide_again(agent);
        if(next == nullptr) {
            continue;
        }
        agent.stranded = false;
        StrategicalDecisionSystem::assign(agent, *next, _stage_manager);
        TacticalDecisionSystem::orient(*_routing_engine, agent);
        resumed.push_back(agent.id);
    }
    return resumed;
}

void Simulation::set_round_robin_weights(
    Route::ID route_id,
    BaseStage::ID stage_id,
    const std::map<BaseStage::ID, uint64_t>& weights)
{
    throw_if_state(State::Iterating | State::Closed, "set_round_robin_weights");
    const auto find_iter = _routes.find(route_id);
    if(find_iter == std::end(_routes) || !find_iter->second->contains_stage(stage_id)) {
        throw SimulationError("Stage {} not part of Route {}", stage_id, route_id);
    }
    auto* round_robin = dynamic_cast<RoundRobin*>(&find_iter->second->rule(stage_id));
    if(round_robin == nullptr) {
        throw SimulationError("Stage {} of Route {} has no round-robin rule.", stage_id, route_id);
    }
    round_robin->set_weights(weights);
}

void Simulation::hold(Route::ID route_id, BaseStage::ID stage_id)
{
    throw_if_state(State::Iterating | State::Closed, "hold");
    auto& route = route_with(route_id, stage_id);
    if(dynamic_cast<const Exit*>(_stage_manager.stage(stage_id)) != nullptr) {
        throw SimulationError("Stage {} is an exit; exits cannot be held.", stage_id);
    }
    route.hold(stage_id);
}

void Simulation::release(Route::ID route_id, BaseStage::ID stage_id)
{
    throw_if_state(State::Iterating | State::Closed, "release");
    auto& route = route_with(route_id, stage_id);
    if(!route.holds(stage_id)) {
        throw SimulationError("Stage {} of Route {} is not held.", stage_id, route_id);
    }
    route.release(stage_id);
    // One agent at a time, so each decision sees the ones before it (counters, targeted counts).
    for(auto* agent : held_at(route_id, stage_id)) {
        agent->held = false;
        // An agent marked for removal leaves next iterate(): it takes no slot.
        if(is_leaving(agent->id)) {
            continue;
        }
        agent->decision_stage = stage_id;
        auto* next = route.decide(*agent, stage_id, *_routing_engine);
        if(next == nullptr) {
            StrategicalDecisionSystem::strand(*agent);
        } else {
            StrategicalDecisionSystem::assign(*agent, *next, _stage_manager);
        }
        TacticalDecisionSystem::orient(*_routing_engine, *agent);
    }
}

bool Simulation::is_held(Route::ID route_id, BaseStage::ID stage_id) const
{
    throw_if_state(State::Closed, "is_held");
    return route_with(route_id, stage_id).holds(stage_id);
}

std::vector<GenericAgent::ID> Simulation::held_agents(Route::ID route_id, BaseStage::ID stage_id)
{
    throw_if_state(State::Closed, "held_agents");
    route_with(route_id, stage_id);
    std::vector<GenericAgent::ID> ids{};
    for(const auto* agent : held_at(route_id, stage_id)) {
        ids.push_back(agent->id);
    }
    return ids;
}

Route& Simulation::route_with(Route::ID route_id, BaseStage::ID stage_id) const
{
    const auto find_iter = _routes.find(route_id);
    if(find_iter == std::end(_routes) || !find_iter->second->contains_stage(stage_id)) {
        throw SimulationError("Stage {} not part of Route {}", stage_id, route_id);
    }
    return *find_iter->second;
}

std::vector<GenericAgent*> Simulation::held_at(Route::ID route_id, BaseStage::ID stage_id)
{
    std::vector<GenericAgent*> held{};
    for(auto& agent : _agents) {
        if(agent.held && agent.route_id == route_id && agent.stage_id == stage_id) {
            held.push_back(&agent);
        }
    }
    std::sort(std::begin(held), std::end(held), [](const auto* lhs, const auto* rhs) {
        return std::tie(lhs->held_since, lhs->id) < std::tie(rhs->held_since, rhs->id);
    });
    return held;
}

Exit& Simulation::exit_of(BaseStage::ID stage_id)
{
    auto* exit = dynamic_cast<Exit*>(_stage_manager.stage(stage_id));
    if(exit == nullptr) {
        throw SimulationError("Stage {} is not an exit.", stage_id);
    }
    return *exit;
}

bool Simulation::is_leaving(GenericAgent::ID id) const
{
    const auto of_agent = [id](const auto& record) { return record.id == id; };
    return std::any_of(
               std::begin(_removed_agents_in_last_iteration),
               std::end(_removed_agents_in_last_iteration),
               of_agent) ||
           std::any_of(std::begin(_marked_for_removal), std::end(_marked_for_removal), of_agent);
}

BaseStage* Simulation::decide_again(const GenericAgent& agent) const
{
    const auto at = agent.decision_stage.value_or(agent.stage_id);
    return _routes.at(agent.route_id)->decide(agent, at, *_routing_engine);
}

bool Simulation::decides_for(const GenericAgent& agent, const BaseStage& stage) const
{
    const auto at = agent.decision_stage.value_or(agent.stage_id);
    return _routes.at(agent.route_id)->rule(at).targets(stage);
}

std::vector<GenericAgent::ID> Simulation::agents_in_range(Point p, double distance)
{
    throw_if_state(State::Closed, "agents_in_range");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Range", Debug);
    std::vector<GenericAgent::ID> neighbor_ids{};
    _neighborhood_search.for_each_in_range(p, distance, [&neighbor_ids](const GenericAgent& agent) {
        neighbor_ids.push_back(agent.id);
    });
    return neighbor_ids;
}

std::vector<GenericAgent::ID> Simulation::agents_in_polygon(const std::vector<Point>& polygon)
{
    throw_if_state(State::Closed, "agents_in_polygon");
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
    throw_if_state(State::Closed, "model_type");
    return _operational_decision_system.model_type();
}

const BaseStage* Simulation::stage(BaseStage::ID stage_id) const
{
    throw_if_state(State::Closed, "stage");
    return _stage_manager.stage(stage_id);
}
std::vector<BaseStage::ID> Simulation::stage_ids() const
{
    throw_if_state(State::Closed, "stage_ids");
    std::vector<BaseStage::ID> ids{};
    ids.reserve(_stage_manager.stages().size());
    for(const auto& [id, stage] : _stage_manager.stages()) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::map<std::size_t, TravelTimeGrid> Simulation::travel_times(BaseStage::ID stage_id)
{
    throw_if_state(State::Iterating | State::Closed, "travel_times");
    const auto* destination = _stage_manager.stage(stage_id)->destination();
    std::map<std::size_t, TravelTimeGrid> grids{};
    if(destination == nullptr) {
        return grids; // nothing to route to, so no region has travel times to it
    }
    for(std::size_t region_id = 0; region_id < _geometry->region_count(); ++region_id) {
        grids.emplace(region_id, destination->travel_times(region_id));
    }
    return grids;
}

double Simulation::travel_time(BaseStage::ID stage_id, const Location& where)
{
    throw_if_state(State::Iterating | State::Closed, "travel_time");
    const auto* destination = _stage_manager.stage(stage_id)->destination();
    if(destination == nullptr) {
        throw SimulationError("Stage {} has no area to route to.", stage_id);
    }
    return destination->travel_time(where);
}

double Simulation::path_length(const Location& from, const Location& to)
{
    throw_if_state(State::Iterating | State::Closed, "path_length");
    return _routing_engine->path_length(from, to);
}

std::map<std::size_t, SpeedGrid> Simulation::default_speed_fields() const
{
    throw_if_state(State::Closed, "default_speed_fields");
    std::map<std::size_t, SpeedGrid> grids{};
    for(std::size_t region_id = 0; region_id < _geometry->region_count(); ++region_id) {
        grids.emplace(region_id, _routing_engine->default_speed(region_id));
    }
    return grids;
}

GridInfo Simulation::grid_info(std::size_t region_id) const
{
    throw_if_state(State::Closed, "grid_info");
    return _routing_engine->grid_info(region_id);
}

void Simulation::set_default_speed_field(std::size_t region_id, const std::vector<double>& values)
{
    throw_if_state(State::Iterating | State::Closed, "set_default_speed_field");
    _routing_engine->set_default_speed(region_id, values);
}

std::map<std::size_t, std::optional<SpeedGrid>>
Simulation::custom_speed_fields(BaseStage::ID stage_id) const
{
    throw_if_state(State::Closed, "custom_speed_fields");
    const auto* destination = _stage_manager.stage(stage_id)->destination();
    std::map<std::size_t, std::optional<SpeedGrid>> grids{};
    if(destination == nullptr) {
        return grids; // nothing to route to, so no speed field to route with
    }
    for(std::size_t region_id = 0; region_id < _geometry->region_count(); ++region_id) {
        // With an override in place, the destination's speed field is that override.
        grids.emplace(
            region_id,
            destination->has_speed_override(region_id) ?
                std::optional{destination->speed(region_id)} :
                std::nullopt);
    }
    return grids;
}

void Simulation::set_custom_speed_field(
    BaseStage::ID stage_id,
    std::size_t region_id,
    const std::vector<double>& values)
{
    throw_if_state(State::Iterating | State::Closed, "set_custom_speed_field");
    destination_of(stage_id).set_speed_override(region_id, values);
}

void Simulation::clear_custom_speed_field(BaseStage::ID stage_id, std::size_t region_id)
{
    throw_if_state(State::Iterating | State::Closed, "clear_custom_speed_field");
    destination_of(stage_id).clear_speed_override(region_id);
}

const Destination& Simulation::destination_of(BaseStage::ID stage_id) const
{
    const auto* destination = _stage_manager.stage(stage_id)->destination();
    if(destination == nullptr) {
        throw SimulationError("Stage {} has no area to route to.", stage_id);
    }
    return *destination;
}

const Geometry& Simulation::geo() const
{
    throw_if_state(State::Closed, "geo");
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
