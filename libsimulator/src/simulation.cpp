// SPDX-License-Identifier: LGPL-3.0-or-later
#include "simulation.hpp"

#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/location.hpp"
#include "id_and_name.hpp"
#include "iterator_pair.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"
#include "polygon.hpp"
#include "records.hpp"
#include "route.hpp"
#include "routing_engine.hpp"
#include "simulation_clock.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "stage_manager.hpp"
#include "timing.hpp"
#include "tracing.hpp"
#include "visitor.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
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
std::vector<AreaPiece> pieces_of(const Geometry& geometry, const std::vector<StageArea>& areas)
{
    if(areas.empty()) {
        throw SimulationError("A target needs at least one area.");
    }
    std::vector<AreaPiece> pieces{};
    for(const auto& area : areas) {
        const auto clipped = geometry.clip_to_region(area.polygon, area.region_id);
        pieces.insert(pieces.end(), clipped.begin(), clipped.end());
    }
    return pieces;
}

/// The agent with @p id in @p agents, or their end. Agents are sorted by id: ids only grow,
/// agents are appended, and removal keeps their order.
template <typename Agents>
auto find_agent(Agents& agents, GenericAgent::ID id)
{
    const auto iter = std::lower_bound(
        std::begin(agents),
        std::end(agents),
        id,
        [](const GenericAgent& agent, GenericAgent::ID wanted) { return agent.id < wanted; });
    return iter != std::end(agents) && iter->id == id ? iter : std::end(agents);
}

/// Whether @p agents are sorted by id, as find_agent() requires.
template <typename Agents>
bool sorted_by_id(const Agents& agents)
{
    return std::is_sorted(
        std::begin(agents), std::end(agents), [](const GenericAgent& a, const GenericAgent& b) {
            return a.id < b.id;
        });
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
    std::shared_ptr<const Geometry> geometry,
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

    _target_completions.clear();
    _removals.clear();
    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agent Removal System", Detailed);
        for(const auto id : _marked_for_removal) {
            _removals.push_back({id, std::nullopt, _clock.elapsed_time()});
        }
        _to_remove.insert(_to_remove.end(), _marked_for_removal.begin(), _marked_for_removal.end());
        _marked_for_removal.clear();
        // AgentRemovalSystem::run needs them sorted.
        std::ranges::sort(_to_remove);
        for(const auto id : _to_remove) {
            if(const auto named = _agent_names.find(id); named != std::end(_agent_names)) {
                _agent_ids_by_name.erase(named->second);
                _agent_names.erase(named);
            }
        }
        _agent_removal_system.run(_agents, _to_remove, _stage_manager);
        assert(sorted_by_id(_agents));
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Neighborhood Search", Detailed);
        _neighborhood_search.update(_agents);
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Strategical Decision System", General);
        _stategical_decision_system.run(
            _routes,
            _agents,
            _stage_manager,
            _clock.elapsed_time(),
            _target_completions,
            _removals,
            _to_remove);
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
        assert(sorted_by_id(_agents));
    }
    _clock.advance();
}

void Simulation::close()
{
    throw_if_state(State::Iterating, "close");
    _state = State::Closed;
    _routing_engine.reset();
}

BaseStage::ID
Simulation::add_stage(const StageDescription& stage_description, std::optional<std::string> name)
{
    throw_if_state(State::Iterating | State::Closed, "add_target");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Target", Detailed);
    if(name) {
        for(const auto& [_, stage] : _stage_manager.stages()) {
            if(stage->name() == name) {
                throw SimulationError("The target name '{}' is already used.", *name);
            }
        }
    }
    auto stage = std::visit(
        Overloaded{
            [this, &name](const WaypointDescription& d) -> std::unique_ptr<BaseStage> {
                const auto pieces = pieces_of(*_geometry, d.areas);
                return std::make_unique<Waypoint>(
                    _routing_engine->register_destination(pieces), std::move(name));
            },
            [this, &name](const ExitDescription& d) -> std::unique_ptr<BaseStage> {
                const auto pieces = pieces_of(*_geometry, d.areas);
                return std::make_unique<Exit>(
                    _routing_engine->register_destination(pieces), std::move(name));
            }},
        stage_description);
    return _stage_manager.add_stage(std::move(stage));
}

Route::ID Simulation::add_route(
    const std::vector<RouteTransitionDescription>& transitions,
    std::optional<std::string> name)
{
    throw_if_state(State::Iterating | State::Closed, "add_route");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Route", Detailed);
    if(name) {
        for(const auto& [_, route] : _routes) {
            if(route->name() == name) {
                throw SimulationError("The route name '{}' is already used.", *name);
            }
        }
    }
    auto route = std::make_unique<Route>(transitions, std::move(name), _stage_manager);
    const auto id = route->id();
    _routes.emplace(id, std::move(route));
    return id;
}

GenericAgent::ID Simulation::add_agent(
    const Location& location,
    Route::ID route_id,
    std::optional<BaseStage::ID> first_stage,
    OperationalModelState model,
    std::optional<std::string> name)
{
    throw_if_state(State::Iterating | State::Closed, "add_agent");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Agent", Detailed);
    check_on_own_geometry(location, "location");
    check_agent_name(name);
    auto& route = this->route(route_id);
    auto* stage = first_stage ? stage_in(route, *first_stage) : nullptr;
    GenericAgent agent{
        GenericAgent::ID::invalid,
        Route::ID::invalid,
        BaseStage::ID::invalid,
        location,
        std::move(model)};
    validate_new_agent(agent);
    // Only now, so that a rejected agent takes no turn in the route's first transition.
    follow(agent, route_id, stage != nullptr ? *stage : *route.first_stage());
    return emplace_agent(std::move(agent), std::move(name));
}

GenericAgent::ID Simulation::add_agent(
    const Location& location,
    const Location& steer_to,
    OperationalModelState model,
    std::optional<std::string> name)
{
    throw_if_state(State::Iterating | State::Closed, "add_agent");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Agent", Detailed);
    check_on_own_geometry(location, "location");
    check_on_own_geometry(steer_to, "steer_to");
    check_agent_name(name);
    GenericAgent agent{
        GenericAgent::ID::invalid,
        Route::ID::invalid,
        BaseStage::ID::invalid,
        location,
        std::move(model)};
    agent.final_target = steer_to;
    validate_new_agent(agent);
    return emplace_agent(std::move(agent), std::move(name));
}

void Simulation::validate_new_agent(const GenericAgent& agent) const
{
    if(const auto agent_model_type = model_type_of(agent.state);
       agent_model_type != _operational_decision_system.model_type()) {
        throw SimulationError(
            "Agent model data of type '{}' does not match the simulation's operational model "
            "'{}'",
            to_string(agent_model_type),
            to_string(_operational_decision_system.model_type()));
    }
    _operational_decision_system.validate_agent(agent, _neighborhood_search, *_geometry);
}

void Simulation::check_agent_name(const std::optional<std::string>& name) const
{
    if(name && agent_named(*name)) {
        throw SimulationError("The agent name '{}' is already used.", *name);
    }
}

std::optional<std::string> Simulation::agent_name(GenericAgent::ID id) const
{
    throw_if_state(State::Closed, "agent_name");
    const auto iter = _agent_names.find(id);
    return iter == std::end(_agent_names) ? std::nullopt : std::optional{iter->second};
}

std::optional<GenericAgent::ID> Simulation::agent_named(const std::string& name) const
{
    throw_if_state(State::Closed, "agent_named");
    const auto iter = _agent_ids_by_name.find(name);
    return iter == std::end(_agent_ids_by_name) ? std::nullopt : std::optional{iter->second};
}

GenericAgent::ID Simulation::emplace_agent(GenericAgent&& agent, std::optional<std::string> name)
{
    // Only once validated, so that a rejected agent takes no stream from the sequence.
    _operational_decision_system.seed_agent_state(agent.state);
    if(name) {
        _agent_ids_by_name.emplace(*name, agent.id);
        _agent_names.emplace(agent.id, std::move(*name));
    }
    _agents.emplace_back(std::move(agent));
    assert(sorted_by_id(_agents));
    _neighborhood_search.add_agent(_agents.back());
    _tactical_decision_system.run(
        *_routing_engine, IteratorPair(std::prev(std::end(_agents)), std::end(_agents)));
    return _agents.back().id;
}

Location Simulation::get_location(double x, double y, std::size_t region_id) const
{
    throw_if_state(State::Closed, "get_location");
    return _geometry->get_location(x, y, region_id);
}

void Simulation::reroute(
    GenericAgent::ID agent_id,
    Route::ID route_id,
    std::optional<BaseStage::ID> first_stage)
{
    throw_if_state(State::Iterating | State::Closed, "reroute");
    auto& agent = this->agent(agent_id);
    auto& route = this->route(route_id);
    follow(agent, route_id, first_stage ? *stage_in(route, *first_stage) : *route.first_stage());
}

void Simulation::reroute(GenericAgent::ID agent_id, BaseStage::ID first_stage)
{
    throw_if_state(State::Iterating | State::Closed, "reroute");
    auto& agent = this->agent(agent_id);
    if(agent.route_id == Route::ID::invalid) {
        throw SimulationError("The agent has no route; pass route=.");
    }
    follow(agent, agent.route_id, *stage_in(route(agent.route_id), first_stage));
}

void Simulation::reroute(GenericAgent::ID agent_id, const Location& steer_to)
{
    throw_if_state(State::Iterating | State::Closed, "reroute");
    check_on_own_geometry(steer_to, "steer_to");
    auto& agent = this->agent(agent_id);
    if(agent.stage_id != BaseStage::ID::invalid) {
        _stage_manager.handle_remove_agent(agent.stage_id);
    }
    agent.route_id = Route::ID::invalid;
    agent.stage_id = BaseStage::ID::invalid;
    agent.final_target = steer_to;
    agent.target_completed = false;
}

void Simulation::follow(GenericAgent& agent, Route::ID route_id, BaseStage& stage)
{
    if(agent.stage_id != BaseStage::ID::invalid) {
        _stage_manager.migrate_agent(agent.stage_id, stage.id());
    } else {
        _stage_manager.handle_new_agent(stage.id());
    }
    agent.route_id = route_id;
    agent.stage_id = stage.id();
    agent.final_target = stage.destination();
    agent.target_completed = false;
}

void Simulation::check_on_own_geometry(const Location& location, const char* what) const
{
    if(!location.belongs_to(*_geometry)) {
        throw SimulationError("{} {} belongs to another geometry.", what, location);
    }
}

Route& Simulation::route(Route::ID route_id)
{
    const auto iter = _routes.find(route_id);
    if(iter == std::end(_routes)) {
        throw SimulationError("Unknown route id {}.", route_id);
    }
    return *iter->second;
}

const Route& Simulation::route(Route::ID route_id) const
{
    throw_if_state(State::Closed, "route");
    const auto iter = _routes.find(route_id);
    if(iter == std::end(_routes)) {
        throw SimulationError("Unknown route id {}.", route_id);
    }
    return *iter->second;
}

std::vector<Route::ID> Simulation::route_ids() const
{
    throw_if_state(State::Closed, "route_ids");
    std::vector<Route::ID> ids{};
    ids.reserve(_routes.size());
    for(const auto& [id, _] : _routes) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

BaseStage* Simulation::stage_in(const Route& route, BaseStage::ID stage_id) const
{
    auto* stage = _stage_manager.stage(stage_id);
    if(!route.contains(stage_id)) {
        throw SimulationError(
            "Target {} is not part of route {}.", id_and_name(*stage), id_and_name(route));
    }
    return stage;
}

void Simulation::mark_agent_for_removal(GenericAgent::ID id)
{
    throw_if_state(State::Iterating | State::Closed, "mark_agent_for_removal");
    JPS_TRACE_FUNC;
    const auto agent = find_agent(_agents, id);
    if(agent == std::end(_agents)) {
        throw SimulationError("Unknown agent id {}", id);
    }
    if(!agent->leaving) {
        agent->leaving = true;
        _marked_for_removal.push_back(id);
    }
}

const std::vector<TargetCompletionRecord>& Simulation::target_completions() const
{
    throw_if_state(State::Closed, "target_completions");
    return _target_completions;
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
    return _removals;
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

std::vector<GenericAgent::ID> Simulation::agents_in_range(const Location& from, double distance)
{
    throw_if_state(State::Closed, "agents_in_range");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Range", Debug);
    check_on_own_geometry(from, "location");
    std::vector<GenericAgent::ID> neighbor_ids{};
    _neighborhood_search.for_each_in_range(
        from.xy(), distance, [this, &from, &neighbor_ids](const GenericAgent& agent) {
            if(_geometry->no_geometry_between(from, agent.location)) {
                neighbor_ids.push_back(agent.id);
            }
        });
    return neighbor_ids;
}

std::vector<GenericAgent::ID>
Simulation::agents_in_polygon(const Polygon& polygon, std::size_t region_id)
{
    throw_if_state(State::Closed, "agents_in_polygon");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agents in Polygon", Debug);
    std::vector<GenericAgent::ID> result{};
    for(const auto& piece : _geometry->clip_to_region(polygon, region_id)) {
        const Polygon clipped{piece.polygon};
        const auto [center, radius] = clipped.containing_circle();
        _neighborhood_search.for_each_in_range(
            center, radius, [region_id, &clipped, &result](const GenericAgent& agent) {
                if(agent.location.region() == region_id && clipped.is_inside(agent.location.xy())) {
                    result.push_back(agent.id);
                }
            });
    }
    return result;
}

OperationalModelType Simulation::model_type() const
{
    throw_if_state(State::Closed, "model_type");
    return _operational_decision_system.model_type();
}

const BaseStage* Simulation::stage(BaseStage::ID stage_id) const
{
    throw_if_state(State::Closed, "target");
    return _stage_manager.stage(stage_id);
}
std::vector<BaseStage::ID> Simulation::stage_ids() const
{
    throw_if_state(State::Closed, "targets");
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
    const auto& destination = _stage_manager.stage(stage_id)->destination();
    std::map<std::size_t, TravelTimeGrid> grids{};
    for(std::size_t region_id = 0; region_id < _geometry->region_count(); ++region_id) {
        grids.emplace(region_id, destination.travel_times(region_id));
    }
    return grids;
}

double Simulation::travel_time(BaseStage::ID stage_id, const Location& from)
{
    throw_if_state(State::Iterating | State::Closed, "travel_time");
    check_on_own_geometry(from, "location");
    return _stage_manager.stage(stage_id)->destination().travel_time(from);
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
    const auto& destination = _stage_manager.stage(stage_id)->destination();
    std::map<std::size_t, std::optional<SpeedGrid>> grids{};
    for(std::size_t region_id = 0; region_id < _geometry->region_count(); ++region_id) {
        // With an override in place, the destination's speed field is that override.
        grids.emplace(
            region_id,
            destination.has_speed_override(region_id) ?
                std::optional{destination.speed(region_id)} :
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
    _stage_manager.stage(stage_id)->destination().set_speed_override(region_id, values);
}

bool Simulation::has_custom_speed_field(BaseStage::ID stage_id, std::size_t region_id) const
{
    throw_if_state(State::Closed, "has_custom_speed_field");
    return _stage_manager.stage(stage_id)->destination().has_speed_override(region_id);
}

void Simulation::clear_custom_speed_field(BaseStage::ID stage_id, std::size_t region_id)
{
    throw_if_state(State::Iterating | State::Closed, "clear_custom_speed_field");
    _stage_manager.stage(stage_id)->destination().clear_speed_override(region_id);
}

const std::shared_ptr<const Geometry>& Simulation::geo() const
{
    throw_if_state(State::Closed, "geo");
    return _geometry;
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
