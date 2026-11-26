// SPDX-License-Identifier: LGPL-3.0-or-later
#include "collision_free_speed_model_tordeux2016.hpp"

#include "agent_view.hpp"
#include "generic_agent.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"
#include "simulation_error.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace
{
/// How far ahead to look for a wall. The repulsion decays with `range_geometry_repulsion` (0.02 m
/// by default), so a wall this far off contributes "nothing".
constexpr double wall_search_radius = 4.0;
/// How far beyond the body the side lines run, in units of `range_geometry_repulsion`. A wall
/// just outside them pushes with at most exp(-5) of the strength at contact.
constexpr double wall_margin_in_ranges = 5.0;
} // namespace

CollisionFreeSpeedModelTordeux2016::CollisionFreeSpeedModelTordeux2016(
    double strength_neighbor_repulsion,
    double range_neighbor_repulsion,
    double strength_geometry_repulsion,
    double range_geometry_repulsion)
    : _strength_neighbor_repulsion(strength_neighbor_repulsion)
    , _range_neighbor_repulsion(range_neighbor_repulsion)
    , _strength_geometry_repulsion(strength_geometry_repulsion)
    , _range_geometry_repulsion(range_geometry_repulsion)
{
}

OperationalModelType CollisionFreeSpeedModelTordeux2016::type() const
{
    return OperationalModelType::CollisionFreeSpeedTordeux2016;
}

Vector CollisionFreeSpeedModelTordeux2016::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& current_state = std::get<State>(current);
    const auto neighborhood = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });

    Vector neighbor_repulsion{};
    for(const auto& neighbor : neighborhood) {
        neighbor_repulsion += this->neighbor_repulsion(current_state, neighbor);
    }

    const auto desired_direction = step.route_orientation();
    // The orientation along 'v'; where 'v' has no direction, the current one.
    const auto orientation_along = [&current_state](Vector v) {
        const Vector heading = v.normalized();
        return heading == Vector{} ? current_state.orientation : heading;
    };
    Vector boundary_repulsion{};
    for(const auto& wall : step.walls_ahead(
            orientation_along(desired_direction + neighbor_repulsion) * wall_search_radius,
            current_state.radius + wall_margin_in_ranges * _range_geometry_repulsion)) {
        boundary_repulsion += this->boundary_repulsion(current_state, wall);
    }

    const Vector direction =
        orientation_along(desired_direction + neighbor_repulsion + boundary_repulsion);
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, get_spacing(current_state, neighbor, direction));
    }

    const auto optimal_speed = this->optimal_speed(current_state, spacing, current_state.time_gap);
    const auto velocity = direction * optimal_speed;
    std::get<State>(next).orientation = direction;
    return velocity * step.dt();
}

void CollisionFreeSpeedModelTordeux2016::check_model_constraint(
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
            throw InvalidValueError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.relative_position,
                distance);
        }
    }

    if(!view.walls_in_range(r).empty()) {
        throw InvalidValueError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance "
            "< {}",
            agent.location.xy(),
            r);
    }
}

double CollisionFreeSpeedModelTordeux2016::optimal_speed(
    const State& current_state,
    double spacing,
    double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), current_state.v0);
}

double CollisionFreeSpeedModelTordeux2016::get_spacing(
    const State& current_state,
    const NeighborView& neighbor,
    const Vector& direction) const
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
Vector CollisionFreeSpeedModelTordeux2016::neighbor_repulsion(
    const State& current_state,
    const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto [distance, direction] = neighbor.relative_position.norm_and_normalized();
    const auto l = current_state.radius + other.radius;
    return direction * -(this->_strength_neighbor_repulsion *
                         exp((l - distance) / this->_range_neighbor_repulsion));
}

Vector CollisionFreeSpeedModelTordeux2016::boundary_repulsion(
    const State& current_state,
    const WallView& boundary) const
{
    const auto l = current_state.radius;
    const auto r_iw =
        -_strength_geometry_repulsion * exp((l - boundary.distance) / _range_geometry_repulsion);
    return -boundary.normal * r_iw; // The repulsion points away from the agent
}
