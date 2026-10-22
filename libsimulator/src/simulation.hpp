// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "agent_removal_system.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "journey.hpp"
#include "neighborhood_search.hpp"
#include "operational_decision_system.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"
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
#include <string_view>
#include <tuple>
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
    std::unique_ptr<Geometry> _geometry{};
    std::unique_ptr<RoutingEngine> _routing_engine{};
    AgentContainer<GenericAgent> _agents;
    std::vector<GenericAgent::ID> _removed_agents_in_last_iteration;
    std::unordered_map<Journey::ID, std::unique_ptr<Journey>> _journeys;
    Timer _timer{};
    State _state{State::Ready};
    enum LogLevel { General = 1, Detailed = 2, Debug = 3 };

    /// Throws SimulationError if the current state is in the mask @p forbidden.
    void throw_if_state(State forbidden, const char* operation) const;

public:
    /// Takes the geometry over: after this the caller no longer owns it. `geo()` hands out a
    /// borrowed reference for as long as the simulation lives.
    Simulation(
        std::unique_ptr<OperationalModel>&& operational_model,
        std::unique_ptr<Geometry>&& geometry,
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
    Journey::ID add_journey(const std::map<BaseStage::ID, TransitionDescription>& stages);
    BaseStage::ID add_stage(const StageDescription& stage_description);
    void mark_agent_for_removal(GenericAgent::ID id);
    const std::vector<GenericAgent::ID>& removed_agents() const;
    size_t agent_count() const;
    double elapsed_time() const;
    double dt() const;
    void
    switch_agent_journey(GenericAgent::ID agent_id, Journey::ID journey_id, BaseStage::ID stage_id);
    uint64_t iteration() const;
    std::vector<GenericAgent::ID> agents_in_range(Point p, double distance);
    /// Returns IDs of all agents inside the defined polygon
    /// @param polygon Required to be a simple convex polygon with CCW ordering.
    std::vector<GenericAgent::ID> agents_in_polygon(const std::vector<Point>& polygon);
    /// @param region_id Region @p position lies in, see `Geometry::get_location`.
    GenericAgent::ID add_agent(
        Journey::ID journey_id,
        BaseStage::ID stage_id,
        Point position,
        OperationalModelState model,
        std::size_t region_id);
    /// See `Geometry::get_location`.
    Location get_location(double x, double y, std::size_t region_id) const;
    /// Locates @p target on the surface closest to the agent's height.
    void set_agent_target(GenericAgent::ID id, Point target);
    void set_agent_target(GenericAgent::ID id, const Location& target);
    const GenericAgent& agent(GenericAgent::ID id) const;
    GenericAgent& agent(GenericAgent::ID id);
    AgentContainer<GenericAgent>& agents();
    OperationalModelType model_type() const;
    const BaseStage* stage(BaseStage::ID stage_id) const;
    /// Ids of all stages, in ascending order.
    std::vector<BaseStage::ID> stage_ids() const;
    /// Travel times to the area of stage @p stage_id over the grid of every region, by region id.
    /// Empty for a stage without an area, such as direct steering.
    std::map<std::size_t, TravelTimeGrid> travel_times(BaseStage::ID stage_id);
    /// The geometry this simulation runs on. Borrowed: it lives as long as the simulation.
    const Geometry& geo() const;
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
