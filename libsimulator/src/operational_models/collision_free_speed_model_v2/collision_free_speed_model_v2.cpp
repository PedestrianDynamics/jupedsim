// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model_v2.hpp"

#include "agent_view.hpp"
#include "generic_agent.hpp"
#include "geometric_functions.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"
#include "simulation_error.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>

namespace
{
/// How far to ask for walls. The repulsion decays with `range_geometry_repulsion` (0.02 m by
/// default), so a wall this far off contributes "nothing".
constexpr double wall_search_radius = 4.0;
} // namespace

OperationalModelType CollisionFreeSpeedModelV2::type() const
{
    return OperationalModelType::CollisionFreeSpeedV2;
}

Point CollisionFreeSpeedModelV2::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& current_state = std::get<State>(current);
    auto neighborhood = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });

    Point neighbor_repulsion{};
    for(const auto& neighbor : neighborhood) {
        neighbor_repulsion += this->neighbor_repulsion(current_state, neighbor);
    }

    Point boundary_repulsion{};
    for(const auto& wall : step.walls_in_range(wall_search_radius)) {
        boundary_repulsion += this->boundary_repulsion(current_state, wall);
    }

    const auto desired_direction = step.route_orientation();
    auto direction = (desired_direction + neighbor_repulsion + boundary_repulsion).normalized();
    if(direction == Point{}) {
        direction = current_state.orientation;
    }
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, get_spacing(current_state, neighbor, direction));
    }

    const auto optimal_speed = this->optimal_speed(current_state, spacing, current_state.time_gap);
    const auto velocity = direction * optimal_speed;
    std::get<State>(next).orientation = direction;
    return velocity * step.dt();
}

void CollisionFreeSpeedModelV2::check_model_constraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& current_state = std::get<State>(agent.state);

    const auto r = current_state.radius;
    constexpr double r_min = 0.;
    constexpr double r_max = 2.;
    validate_constraint(r, r_min, r_max, "radius", true);

    const auto v0 = current_state.v0;
    constexpr double v0_min = 0.;
    constexpr double v0_max = 10.;
    validate_constraint(v0, v0_min, v0_max, "v0");

    const auto time_gap = current_state.time_gap;
    constexpr double time_gap_min = 0.1;
    constexpr double time_gap_max = 10.;
    validate_constraint(time_gap, time_gap_min, time_gap_max, "timeGap");

    const auto neighbors = view.other_agents_in_range(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_state = std::get<State>(*neighbor.state);
        const auto contanctd_dist = r + neighbor_state.radius;
        const auto distance = neighbor.relative_position.norm();
        if(contanctd_dist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.relative_position,
                distance);
        }
    }

    if(!view.walls_in_range(r).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance "
            "< {}",
            agent.location.xy(),
            r);
    }
}

double CollisionFreeSpeedModelV2::optimal_speed(
    const State& current_state,
    double spacing,
    double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), current_state.v0);
}

double CollisionFreeSpeedModelV2::get_spacing(
    const State& current_state,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.relative_position;
    const auto in_front = direction.scalar_product(distp12) >= 0;
    if(!in_front) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.rotate90_deg();
    const auto l = current_state.radius + other.radius;
    bool in_corridor = std::abs(left.scalar_product(distp12)) <= l;
    if(!in_corridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.norm() - l;
}
Point CollisionFreeSpeedModelV2::neighbor_repulsion(
    const State& current_state,
    const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto [distance, direction] = neighbor.relative_position.norm_and_normalized();
    const auto l = current_state.radius + other.radius;
    return direction * -(current_state.strength_neighbor_repulsion *
                         exp((l - distance) / current_state.range_neighbor_repulsion));
}

Point CollisionFreeSpeedModelV2::boundary_repulsion(
    const State& current_state,
    const WallView& boundary) const
{
    const auto l = current_state.radius;
    const auto r_iw = -current_state.strength_geometry_repulsion *
                      exp((l - boundary.distance) / current_state.range_geometry_repulsion);
    return -boundary.normal * r_iw; // The repulsion points away from the agent
}
