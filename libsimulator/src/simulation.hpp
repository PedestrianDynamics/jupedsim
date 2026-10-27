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

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

class Simulation
{
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
    /// Set for the duration of iterate(); mutating entry points must not run while the
    /// iteration pipeline works on the agent containers.
    bool _iterating{false};
    enum LogLevel { General = 1, Detailed = 2, Debug = 3 };

    void throw_if_iterating(const char* operation) const;
    /// The destination of stage @p stage_id; throws for a stage without an area.
    const Destination& destination_of(BaseStage::ID stage_id) const;

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
    /// Speed fields every stage without an override is routed with, by region id.
    std::map<std::size_t, SpeedGrid> default_speed_fields() const;
    /// Replaces the default speed field of @p region_id, see
    /// FloorfieldRoutingEngine::set_default_speed for what @p values must hold.
    void set_default_speed_field(std::size_t region_id, const std::vector<double>& values);
    /// Speed fields agents are routed to the area of stage @p stage_id with, by region id. Empty
    /// for a stage without an area, such as direct steering.
    std::map<std::size_t, SpeedGrid> speed_fields(BaseStage::ID stage_id) const;
    /// Whether stage @p stage_id has its own speed field in @p region_id.
    bool has_speed_field_override(BaseStage::ID stage_id, std::size_t region_id) const;
    /// Routes agents to the area of stage @p stage_id with @p values in @p region_id instead of
    /// the default speed field. Other stages are not affected. Throws for a stage without an
    /// area.
    void set_speed_field_override(
        BaseStage::ID stage_id,
        std::size_t region_id,
        const std::vector<double>& values);
    /// Routes agents to the area of stage @p stage_id with the default speed field of
    /// @p region_id again. Throws for a stage without an area.
    void clear_speed_field_override(BaseStage::ID stage_id, std::size_t region_id);
    /// The geometry this simulation runs on. Borrowed: it lives as long as the simulation.
    const Geometry& geo() const;
    void push_timer(const std::string_view name, size_t probe_log_level = 0);
    void pop_timer(const std::string_view name);
    void set_timer_log_level(int level) { _timer.set_log_level(level); };
    TimerEntry::DurationType get_timer_duration(const std::string_view name) const;
    std::map<std::string, TimerEntry::DurationType> get_timer_durations() const;
};
