// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
constexpr double eps = 1e-6; // Numeric lower bound to avoid division by zero in range terms.
constexpr double side_eps =
    0.05; // Smooths left/right sign near centerline to reduce heading flips.
constexpr double spacing_blend_weight =
    0.15; // Blends move-direction spacing with goal-direction spacing.
constexpr double tau_theta = 0.3; // Heading relaxation timescale [s] for temporal smoothing.
constexpr double min_reverse_speed =
    -0.01; // Deterministic tiny reverse floor [m/s] to release local blockages.
/// How far to ask for walls. The repulsion decays with `range_geometry_repulsion` (0.02 m by
/// default), so a wall this far off contributes "nothing".
constexpr double wall_search_radius = 4.0;

double neighbor_influence(
    const std::vector<NeighborView>& neighborhood,
    const Point& reference_direction,
    const CollisionFreeSpeedModelV3::State& current_state)
{
    const auto range_x =
        std::max(eps, current_state.range_neighbor_repulsion * current_state.range_x_scale);
    const auto range_y =
        std::max(eps, current_state.range_neighbor_repulsion * current_state.range_y_scale);
    const auto theta_max = std::clamp(
        current_state.strength_neighbor_repulsion, 0.0, current_state.theta_max_upper_bound);

    double best_influence = 0.0;
    double best_weight = 0.0;
    for(const auto& neighbor : neighborhood) {
        const auto relative = neighbor.relative_position;
        const auto x = reference_direction.scalar_product(relative);
        if(x <= 0.0) {
            continue;
        }

        const auto signed_lateral = reference_direction.cross_product(relative);
        const auto y = std::abs(signed_lateral);
        const auto longitudinal_weight = std::exp(-x / range_x);
        const auto lateral_weight = std::exp(-y / range_y);
        const auto weight = longitudinal_weight * lateral_weight;
        if(weight > best_weight) {
            best_weight = weight;
            best_influence = -weight * (signed_lateral / (std::abs(signed_lateral) + side_eps));
        }
    }

    return theta_max * std::tanh(best_influence);
}
} // namespace

OperationalModelType CollisionFreeSpeedModelV3::type() const
{
    return OperationalModelType::CollisionFreeSpeedV3;
}

Point CollisionFreeSpeedModelV3::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& current_state = std::get<State>(current);
    auto neighborhood = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });

    Point boundary_repulsion{};
    for(const auto& wall : step.walls_in_range(wall_search_radius)) {
        boundary_repulsion += this->boundary_repulsion(current_state, wall);
    }

    const auto desired_direction = step.route_orientation();
    auto reference_direction = (desired_direction + boundary_repulsion).normalized();
    if(reference_direction == Point{}) {
        reference_direction = current_state.orientation;
    }

    const auto heading_target =
        neighbor_influence(neighborhood, reference_direction, current_state);
    const auto alpha = std::clamp(step.dt() / tau_theta, 0.0, 1.0);
    const auto heading_angle =
        current_state.heading_angle + alpha * (heading_target - current_state.heading_angle);
    auto direction =
        reference_direction.rotate(std::cos(heading_angle), std::sin(heading_angle)).normalized();
    if(direction == Point{}) {
        direction = reference_direction;
    }

    const auto closest_spacing_towards = [&](Point towards) {
        auto spacing = std::numeric_limits<double>::max();
        for(const auto& neighbor : neighborhood) {
            spacing = std::min(spacing, get_spacing(current_state, neighbor, towards));
        }
        return spacing;
    };

    const auto spacing_move = closest_spacing_towards(direction);
    const auto goal_direction =
        (desired_direction == Point{}) ? reference_direction : desired_direction;
    const auto spacing_goal = closest_spacing_towards(goal_direction);

    const auto spacing =
        spacing_move * (1.0 - spacing_blend_weight) + spacing_goal * spacing_blend_weight;

    const auto optimal_speed = this->optimal_speed(current_state, spacing, current_state.time_gap);
    const auto velocity = direction * optimal_speed;
    auto& next_model = std::get<State>(next);
    next_model.orientation = direction;
    next_model.heading_angle = heading_angle;
    return velocity * step.dt();
}

void CollisionFreeSpeedModelV3::check_model_constraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& current_state = std::get<State>(agent.state);

    validate_constraint(current_state.radius, 0.0, 2.0, "radius", true);
    validate_constraint(current_state.v0, 0.0, 10.0, "v0");
    validate_constraint(current_state.time_gap, 0.1, 10.0, "timeGap");

    validate_constraint(
        current_state.strength_neighbor_repulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthNeighborRepulsion");
    validate_constraint(
        current_state.range_neighbor_repulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeNeighborRepulsion");
    validate_constraint(
        current_state.strength_geometry_repulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthGeometryRepulsion");
    validate_constraint(
        current_state.range_geometry_repulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeGeometryRepulsion");

    validate_constraint(
        current_state.range_x_scale, 0.01, std::numeric_limits<double>::max(), "rangeXScale");
    validate_constraint(
        current_state.range_y_scale, 0.01, std::numeric_limits<double>::max(), "rangeYScale");
    validate_constraint(
        current_state.theta_max_upper_bound, 0.0, std::acos(-1.0), "thetaMaxUpperBound");
    validate_constraint(current_state.agent_buffer, 0.0, 100.0, "agentBuffer");

    const auto neighbors = view.other_agents_in_range(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_state = std::get<State>(*neighbor.state);
        const auto contact_dist = current_state.radius + neighbor_state.radius;
        const auto distance = neighbor.relative_position.norm();
        if(contact_dist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.relative_position,
                distance);
        }
    }

    if(!view.walls_in_range(current_state.radius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance "
            "< {}",
            agent.location.xy(),
            current_state.radius);
    }
}

double CollisionFreeSpeedModelV3::optimal_speed(
    const State& current_state,
    double spacing,
    double time_gap) const
{
    const auto effective_spacing = spacing - current_state.agent_buffer;
    return std::min(std::max(effective_spacing / time_gap, min_reverse_speed), current_state.v0);
}

double CollisionFreeSpeedModelV3::get_spacing(
    const State& current_state,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.relative_position;
    if(direction.scalar_product(distp12) < 0.0) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.rotate90_deg();
    const auto l = current_state.radius + other.radius;
    const auto in_corridor = std::abs(left.scalar_product(distp12)) <= l;
    if(!in_corridor) {
        return std::numeric_limits<double>::max();
    }

    return distp12.norm() - l;
}

Point CollisionFreeSpeedModelV3::boundary_repulsion(
    const State& current_state,
    const WallView& boundary) const
{
    const auto l = current_state.radius;
    const auto r_iw = -current_state.strength_geometry_repulsion *
                      std::exp((l - boundary.distance) / current_state.range_geometry_repulsion);
    return -boundary.normal * r_iw; // The repulsion points away from the agent
}
