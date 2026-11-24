// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "agent_removal_system.hpp"
#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "neighborhood_search.hpp"
#include "operational_decision_system.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"
#include "polygon.hpp"
#include "route.hpp"
#include "routing_engine.hpp"
#include "simulation_clock.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "stage_manager.hpp"
#include "strategical_desicion_system.hpp"
#include "tactical_decision_system.hpp"
#include "timing.hpp"

#include <fmt/format.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class Simulation
{
public:
    /// Ready allows all operations. Mutating entry points must not run while the iteration
    /// pipeline works on the agent containers (Iterating). Closed blocks all entry points.
    /// The simulation is in exactly one state; combine states with `|` to form a mask.
    enum class State : uint8_t { Ready = 1 << 0, Iterating = 1 << 1, Closed = 1 << 2 };

private:
    /// RAII setter for `_state`: Iterating on construction, Ready on scope exit (including
    /// exception unwinding out of the iteration pipeline).
    class IterationScope
    {
        State& _state;

    public:
        explicit IterationScope(State& state) : _state(state) { _state = State::Iterating; }
        ~IterationScope() { _state = State::Ready; }
        IterationScope(const IterationScope&) = delete;
        IterationScope& operator=(const IterationScope&) = delete;
    };

    SimulationClock _clock;
    StrategicalDecisionSystem _stategical_decision_system{};
    TacticalDecisionSystem _tactical_decision_system{};
    OperationalDecisionSystem _operational_decision_system;
    AgentRemovalSystem<GenericAgent> _agent_removal_system{};
    StageManager _stage_manager{};
    NeighborhoodSearch<GenericAgent> _neighborhood_search{2.2};
    std::shared_ptr<const Geometry> _geometry{};
    std::unique_ptr<RoutingEngine> _routing_engine{};
    AgentContainer<GenericAgent> _agents;
    /// Agents that reached an exit or were marked, removed at the beginning of the next
    /// iteration.
    std::vector<GenericAgent::ID> _marked_for_removal;
    std::unordered_map<Route::ID, std::unique_ptr<Route>> _routes;
    /// Kept apart from the agents, which are copied every iteration; only named agents.
    std::unordered_map<GenericAgent::ID, std::string> _agent_names;
    std::unordered_map<std::string, GenericAgent::ID> _agent_ids_by_name;
    Timer _timer{};
    State _state{State::Ready};
    enum LogLevel { General = 1, Detailed = 2, Debug = 3 };

    /// Throws SimulationError if the current state is in the mask @p forbidden.
    void throw_if_state(State forbidden, const char* operation) const;
    /// Throws unless @p location lies on this simulation's geometry.
    void check_on_own_geometry(const Location& location, const char* what) const;
    Route& route(Route::ID route_id);
    /// The stage @p stage_id of @p route; throws if it is not part of it.
    BaseStage* stage_in(const Route& route, BaseStage::ID stage_id) const;
    /// The agent @p agent_id; throws if it leaves the simulation.
    GenericAgent& reroutable(GenericAgent::ID agent_id);
    /// Lets @p agent follow @p route_id from @p stage on.
    void follow(GenericAgent& agent, Route::ID route_id, BaseStage& stage);
    void validate_new_agent(const GenericAgent& agent) const;
    /// Throws if another agent has @p name.
    void check_agent_name(const std::optional<std::string>& name) const;
    /// Adds @p agent, with its orientation, and returns its id.
    GenericAgent::ID emplace_agent(GenericAgent&& agent, std::optional<std::string> name);

public:
    /// Shares ownership of the geometry, which is never modified: several simulations, or a
    /// simulation and a routing engine, may use the same geometry. `geo()` shares it too.
    Simulation(
        std::unique_ptr<OperationalModel>&& operational_model,
        std::shared_ptr<const Geometry> geometry,
        double dt,
        FloorfieldOptions floorfield_options);

    Simulation(const Simulation& other) = delete;
    Simulation& operator=(const Simulation& other) = delete;
    Simulation(Simulation&& other) = delete;
    Simulation& operator=(Simulation&& other) = delete;
    ~Simulation() = default;
    const SimulationClock& clock() const;
    void set_tracing(bool on);
    void iterate();
    /// Closes the simulation freeing memory. Every entry point (except timer's) throws afterwards.
    void close();
    /// @p name has to be unique among the stages, if given.
    BaseStage::ID
    add_stage(const StageDescription& stage_description, std::optional<std::string> name = {});
    /// See Route::Route for what @p transitions must hold. @p name has to be unique among the
    /// routes, if given.
    Route::ID add_route(
        const std::vector<RouteTransitionDescription>& transitions,
        std::optional<std::string> name = {});
    /// Removes the agent at the beginning of the next iteration. Nothing changes for an agent
    /// that is about to leave already.
    void mark_agent_for_removal(GenericAgent::ID id);
    /// The agents removed at the beginning of the next iteration.
    const std::vector<GenericAgent::ID>& removed_agents() const;
    size_t agent_count() const;
    double elapsed_time() const;
    double dt() const;
    /// Lets the agent follow @p route_id, from @p first_stage on, or from where the route's
    /// first transition leads if none is given. The agent reaches its new target after the next
    /// step at the earliest, even if it stands in it already. Every reroute() throws for an agent
    /// that leaves the simulation: it reached an exit, or was marked for removal.
    void reroute(
        GenericAgent::ID agent_id,
        Route::ID route_id,
        std::optional<BaseStage::ID> first_stage = {});
    /// Lets the agent go on from @p first_stage along the route it follows.
    void reroute(GenericAgent::ID agent_id, BaseStage::ID first_stage);
    /// Steers the agent to @p steer_to, off its route.
    void reroute(GenericAgent::ID agent_id, const Location& steer_to);
    uint64_t iteration() const;
    /// Ids of the agents within @p distance of @p from that are neighbors in the sense of the
    /// operational models: in plain sight over the surface, see Geometry::no_geometry_between.
    /// Agents on a floor above or below are not.
    std::vector<GenericAgent::ID> agents_in_range(const Location& from, double distance);
    /// Ids of the agents inside @p polygon, clipped to region @p region_id as a stage area is.
    /// Throws if @p polygon does not intersect that region.
    std::vector<GenericAgent::ID> agents_in_polygon(const Polygon& polygon, std::size_t region_id);
    /// Adds an agent at @p location that follows @p route_id, see reroute(). An agent added
    /// inside its first stage has reached it; it moves on in the next iteration.
    /// @p name has to be unique among the agents, if given; it is free again once its agent has
    /// left.
    GenericAgent::ID add_agent(
        const Location& location,
        Route::ID route_id,
        std::optional<BaseStage::ID> first_stage,
        OperationalModelState model,
        std::optional<std::string> name = {});
    /// Adds an agent at @p location that is steered to @p steer_to.
    GenericAgent::ID add_agent(
        const Location& location,
        const Location& steer_to,
        OperationalModelState model,
        std::optional<std::string> name = {});
    std::optional<std::string> agent_name(GenericAgent::ID id) const;
    /// The agent named @p name, if there is one.
    std::optional<GenericAgent::ID> agent_named(const std::string& name) const;
    /// See `Geometry::get_location`.
    Location get_location(double x, double y, std::size_t region_id) const;
    const GenericAgent& agent(GenericAgent::ID id) const;
    GenericAgent& agent(GenericAgent::ID id);
    AgentContainer<GenericAgent>& agents();
    OperationalModelType model_type() const;
    const BaseStage* stage(BaseStage::ID stage_id) const;
    /// Ids of all stages, in ascending order.
    std::vector<BaseStage::ID> stage_ids() const;
    const Route& route(Route::ID route_id) const;
    /// Ids of all routes, in ascending order.
    std::vector<Route::ID> route_ids() const;
    /// Travel times to the area of stage @p stage_id over the grid of every region, by region id.
    std::map<std::size_t, TravelTimeGrid> travel_times(BaseStage::ID stage_id);
    /// Travel time in seconds from @p from to the area of stage @p stage_id, see
    /// FloorfieldRoutingEngine::travel_time.
    double travel_time(BaseStage::ID stage_id, const Location& from);
    /// Speed fields every stage without an override is routed with, by region id.
    std::map<std::size_t, SpeedGrid> default_speed_fields() const;
    /// Placement of the floor field grid of @p region_id, which its travel times and speed
    /// fields lie on. Throws for an unknown region.
    GridInfo grid_info(std::size_t region_id) const;
    /// Replaces the default speed field of @p region_id, see
    /// FloorfieldRoutingEngine::set_default_speed for what @p values must hold.
    void set_default_speed_field(std::size_t region_id, const std::vector<double>& values);
    /// The speed fields of stage @p stage_id's own, by region id: empty (std::nullopt) where it
    /// is routed with the default.
    std::map<std::size_t, std::optional<SpeedGrid>>
    custom_speed_fields(BaseStage::ID stage_id) const;
    /// Whether stage @p stage_id has a speed field of its own in @p region_id, without copying
    /// it. Throws for an unknown region.
    bool has_custom_speed_field(BaseStage::ID stage_id, std::size_t region_id) const;
    /// Routes agents to the area of stage @p stage_id with @p values in @p region_id instead of
    /// the default speed field. Other stages are not affected.
    void set_custom_speed_field(
        BaseStage::ID stage_id,
        std::size_t region_id,
        const std::vector<double>& values);
    /// Routes agents to the area of stage @p stage_id with the default speed field of
    /// @p region_id again.
    void clear_custom_speed_field(BaseStage::ID stage_id, std::size_t region_id);
    /// The geometry this simulation runs on. Borrowed: it lives as long as the simulation.
    const std::shared_ptr<const Geometry>& geo() const;
    void push_timer(const std::string_view name, size_t probe_log_level = 0);
    void pop_timer(const std::string_view name);
    void set_timer_log_level(int level) { _timer.set_log_level(level); };
    TimerEntry::DurationType get_timer_duration(const std::string_view name) const;
    std::map<std::string, TimerEntry::DurationType> get_timer_durations() const;
};

constexpr Simulation::State operator|(Simulation::State lhs, Simulation::State rhs)
{
    return static_cast<Simulation::State>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr Simulation::State operator&(Simulation::State lhs, Simulation::State rhs)
{
    return static_cast<Simulation::State>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

/// Prints a state by name, a mask as its names joined by `|`.
template <>
struct fmt::formatter<Simulation::State> : fmt::formatter<std::string_view> {
    auto format(Simulation::State state, format_context& ctx) const
    {
        using enum Simulation::State;
        auto out = ctx.out();
        bool first = true;
        for(const auto& [flag, name] :
            {std::pair{Ready, "Ready"},
             std::pair{Iterating, "Iterating"},
             std::pair{Closed, "Closed"}}) {
            if((state & flag) == flag) {
                out = fmt::format_to(out, "{}{}", first ? "" : "|", name);
                first = false;
            }
        }
        return out;
    }
};
