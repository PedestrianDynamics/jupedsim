// SPDX-License-Identifier: LGPL-3.0-or-later
#include "simulation.hpp"

#include "agent_view.hpp"
#include "destination.hpp"
#include "environment_query.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/location.hpp"
#include "iterator_pair.hpp"
#include "journey.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"
#include "polygon.hpp"
#include "routing_engine.hpp"
#include "simulation_clock.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "thread_pool.hpp"
#include "timing.hpp"
#include "tracing.hpp"
#include "visitor.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
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
        throw SimulationError("A stage needs at least one area");
    }
    std::vector<AreaPiece> pieces{};
    for(const auto& area : areas) {
        const auto clipped = geometry.clip_to_region(area.polygon, area.region_id);
        pieces.insert(pieces.end(), clipped.begin(), clipped.end());
    }
    return pieces;
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
    // SPIKE: default thread count from the environment, so existing test suites can run the
    // parallel path unchanged.
    if(const char* env = std::getenv("JPS_NUM_THREADS"); env != nullptr) {
        const auto threads = std::strtoul(env, nullptr, 10);
        if(threads > 1) {
            _operational_decision_system.set_num_threads(threads);
        }
    }
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

    const auto agents_before_removal = _agents.size();
    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Agent Removal System", Detailed);
        _agent_removal_system.run(_agents, _removed_agents_in_last_iteration, _stage_manager);
    }

    // SPIKE: JPS_SORT=1 orders agents along a Morton curve of 2 m cells, so that contiguous
    // index blocks are spatially compact and a thread's neighbours are mostly its own agents.
    static const bool sort_agents = [] {
        const char* env = std::getenv("JPS_SORT");
        return env != nullptr && env[0] == '1';
    }();
    bool reordered = false;
    if(sort_agents) {
        const auto spread = [](std::uint32_t v) {
            std::uint64_t x = v;
            x = (x | (x << 16U)) & 0x0000FFFF0000FFFFULL;
            x = (x | (x << 8U)) & 0x00FF00FF00FF00FFULL;
            x = (x | (x << 4U)) & 0x0F0F0F0F0F0F0F0FULL;
            x = (x | (x << 2U)) & 0x3333333333333333ULL;
            x = (x | (x << 1U)) & 0x5555555555555555ULL;
            return x;
        };
        const auto key = [&](const GenericAgent& a) {
            const auto cx = static_cast<std::uint32_t>(std::max(0.0, a.location.xy().x() / 2.0));
            const auto cy = static_cast<std::uint32_t>(std::max(0.0, a.location.xy().y() / 2.0));
            return spread(cx) | (spread(cy) << 1U);
        };
        std::stable_sort(_agents.begin(), _agents.end(), [&](const auto& a, const auto& b) {
            return key(a) < key(b);
        });
        reordered = true;
    }

    // The grid is current: the last iteration rebuilt it after moving the agents, and
    // add_agent() inserts into it. Only a removal, which shifts agents inside the container,
    // leaves it pointing at the wrong ones.
    if(reordered || _agents.size() != agents_before_removal || _neighborhood_search.has_pending()) {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Neighborhood Search", Detailed);
        _neighborhood_search.update(_agents, _operational_decision_system.pool());
    }

    // SPIKE: JPS_PAR_MIN is the crowd size from which strategical runs in parallel; below it a
    // parallel loop costs more than the work.
    static const std::size_t parallel_min = [] {
        const char* env = std::getenv("JPS_PAR_MIN");
        return env != nullptr ? static_cast<std::size_t>(std::strtoul(env, nullptr, 10)) : 1000;
    }();
    const bool parallel_strategical =
        _operational_decision_system.pool() != nullptr && _agents.size() >= parallel_min;
    bool location_targets = true;
    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Strategical Decision System", General);
        if(parallel_strategical) {
            location_targets = _stategical_decision_system.run_parallel(
                _journeys, _agents, _stage_manager, *_operational_decision_system.pool());
        } else {
            _stategical_decision_system.run(_journeys, _agents, _stage_manager);
        }
    }

    // With threads, routing each agent is fused into the parallel operational loop: the lookup
    // is cheap (~0.1 us), so a loop of its own would cost more in synchronisation and in moving
    // agent data between cores than it saves. Only the serial preparation stays here.
    const bool fused_routing = _operational_decision_system.pool() != nullptr;
    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Tactical Decision System", General);
        if(fused_routing && parallel_strategical) {
            // Strategical left every floor-field destination prepared and said whether any agent
            // heads for a plain location.
            if(location_targets) {
                _tactical_decision_system.route_locations(*_routing_engine, _agents);
            }
        } else if(fused_routing) {
            _tactical_decision_system.prepare(*_routing_engine, _agents);
        } else {
            _tactical_decision_system.run(*_routing_engine, _agents);
        }
    }

    {
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Operational Decision System", General);
        static const std::function<void(GenericAgent&)> no_routing{};
        static const std::function<void(GenericAgent&)> route{
            &TacticalDecisionSystem::route_prepared};
        // With threads, each agent's next cell is computed right after it moved, while its
        // position is still in that core's cache; the rebuild then only counts and scatters.
        auto* next_cells = fused_routing ? &_neighborhood_search.keys_for(_agents.size()) : nullptr;
        _operational_decision_system.run(
            _clock.dt(),
            _clock.iteration(),
            _neighborhood_search,
            *_geometry,
            _agents,
            fused_routing ? route : no_routing,
            next_cells);
        // Agents moved during the operational step; rebuild the grid so cell membership
        // reflects the new positions for queries before the next iteration (agents_in_range,
        // add_agent validation).
        JPS_SCOPED_TIMER_AND_TRACE(_timer, "Neighborhood Search (post)", Detailed);
        if(next_cells != nullptr) {
            _neighborhood_search.update_from_keys(_agents);
        } else {
            _neighborhood_search.update(_agents);
        }
    }
    _clock.advance();
}

void Simulation::close()
{
    throw_if_state(State::Iterating, "close");
    _state = State::Closed;
    _routing_engine.reset();
}

Journey::ID Simulation::add_journey(const std::map<BaseStage::ID, TransitionDescription>& stages)
{
    throw_if_state(State::Iterating | State::Closed, "add_journey");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Journey", Detailed);
    std::map<BaseStage::ID, JourneyNode> nodes;
    bool contains_direct_steering =
        std::find_if(std::begin(stages), std::end(stages), [this](auto const& pair) {
            return dynamic_cast<const DirectSteering*>(stage(pair.first)) != nullptr;
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
    throw_if_state(State::Iterating | State::Closed, "add_agent");
    JPS_SCOPED_TIMER_AND_TRACE(_timer, "Add Agent", Detailed);
    const auto location = _geometry->get_location(position.x(), position.y(), region_id);
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
}

void Simulation::set_agent_target(GenericAgent::ID id, const Location& target)
{
    throw_if_state(State::Closed, "set_agent_target");
    agent(id).final_target = target;
}

void Simulation::mark_agent_for_removal(GenericAgent::ID id)
{
    throw_if_state(State::Iterating | State::Closed, "mark_agent_for_removal");
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
    throw_if_state(State::Closed, "agent");
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
    throw_if_state(State::Closed, "agent");
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

void Simulation::switch_agent_journey(
    GenericAgent::ID agent_id,
    Journey::ID journey_id,
    BaseStage::ID stage_id)
{
    throw_if_state(State::Iterating | State::Closed, "switch_agent_journey");
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

void Simulation::set_num_threads(std::size_t threads)
{
    throw_if_state(State::Iterating | State::Closed, "set_num_threads");
    _operational_decision_system.set_num_threads(threads);
}

double Simulation::spike_probe(int kind, std::size_t threads, std::size_t reps)
{
    ThreadPool pool{threads};
    const EnvironmentQuery env{*_geometry, _neighborhood_search};
    std::vector<double> sink(_agents.size(), 0.0);
    const auto body = [&](std::size_t i) {
        const auto& agent = _agents[i];
        const AgentStep step{env, agent, _clock.dt(), nullptr, 0, i};
        double acc = 0.0;
        switch(kind) {
            case 0: { // neighbours with the visibility filter, as the models ask
                const auto n = step.other_agents_in_range(
                    3.0, [&step](const NeighborView& v) { return step.no_geometry_between(v); });
                acc += static_cast<double>(n.size());
                break;
            }
            case 1: { // neighbours only
                const auto n = step.other_agents_in_range(3.0);
                acc += static_cast<double>(n.size());
                break;
            }
            case 2: { // wall query
                for(const auto& w : step.walls_in_range(4.0)) {
                    acc += w.distance;
                }
                break;
            }
            case 3: { // visibility along a fixed direction
                acc += step.no_geometry_between(Vector{0.5, 0.2}) ? 1.0 : 0.0;
                break;
            }
            case 4: { // straight walk on the mesh
                const auto moved = agent.location.try_move_on_surface(Vector{0.01, 0.0});
                acc += moved ? moved->xy().x() : 0.0;
                break;
            }
            case 5: { // control: arithmetic only
                double x = static_cast<double>(i);
                for(int k = 0; k < 2000; ++k) {
                    x = std::sqrt(x * x + 1.0) * 0.999;
                }
                acc += x;
                break;
            }
            case 6: { // control: 10 small heap allocations
                for(int k = 0; k < 10; ++k) {
                    std::vector<double> v(16 + k, 1.0);
                    acc += v.back();
                }
                break;
            }
            default:
                break;
        }
        sink[i] += acc;
    };
    const auto t0 = std::chrono::steady_clock::now();
    for(std::size_t r = 0; r < reps; ++r) {
        pool.parallel_for(_agents.size(), 1, body);
    }
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    volatile double guard = std::accumulate(sink.begin(), sink.end(), 0.0);
    (void) guard;
    return elapsed;
}

std::size_t Simulation::num_threads() const
{
    return _operational_decision_system.num_threads();
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
