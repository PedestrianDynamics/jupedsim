// SPDX-License-Identifier: LGPL-3.0-or-later
#include "anticipation_velocity_model.hpp"

#include "agent_view.hpp"
#include "generic_agent.hpp"
#include "geometric_functions.hpp"
#include "macros.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"
#include "simulation_error.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <vector>

AnticipationVelocityModel::AnticipationVelocityModel(double pushout_strength, uint64_t rng_seed)
    : _pushout_strength(pushout_strength), _gen(rng_seed)
{
}

OperationalModelType AnticipationVelocityModel::type() const
{
    return OperationalModelType::AnticipationVelocityModel;
}

Point AnticipationVelocityModel::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& current_state = std::get<State>(current);
    // Exclude occluded and self agents
    auto neighborhood = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });

    const auto desired_direction = step.route_orientation();
    Point neighbor_repulsion{};
    for(const auto& neighbor : neighborhood) {
        neighbor_repulsion += this->neighbor_repulsion(current_state, desired_direction, neighbor);
    }

    auto direction = (desired_direction + neighbor_repulsion).normalized();
    if(direction == Point{}) {
        direction = current_state.orientation;
    }

    // update direction towards the newly calculated direction
    direction = update_direction(current_state, desired_direction, direction, step.dt());
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, get_spacing(current_state, neighbor, direction));
    }

    const auto optimal_speed = this->optimal_speed(current_state, spacing, current_state.time_gap);
    // Wall sliding behavior
    direction = handle_wall_avoidance(direction, current_state, step, _pushout_strength);

    const auto velocity = direction * optimal_speed;
    auto& next_model = std::get<State>(next);
    next_model.orientation = direction;
    next_model.velocity = velocity;
    return velocity * step.dt();
}

Point AnticipationVelocityModel::update_direction(
    const State& current_state,
    Point desired_direction,
    const Point& calculated_direction,
    double dt) const
{
    const Point actual_direction = current_state.orientation;
    Point updated_direction;

    if(desired_direction.scalar_product(calculated_direction) *
           desired_direction.scalar_product(actual_direction) <
       0) {
        updated_direction = calculated_direction;
    } else {
        // Compute the rate of change of direction (Eq. 7)
        const Point direction_derivative =
            (calculated_direction.normalized() - actual_direction) / current_state.reaction_time;
        updated_direction = actual_direction + direction_derivative * dt;
    }

    return updated_direction.normalized();
}

void AnticipationVelocityModel::check_model_constraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& current_state = std::get<State>(agent.state);
    const auto r = current_state.radius;
    constexpr double r_min = 0.;
    constexpr double r_max = 2.;
    validate_constraint(r, r_min, r_max, "radius", true);

    const auto strength_neighbor_repulsion = current_state.strength_neighbor_repulsion;
    constexpr double sn_min = 0.;
    constexpr double sn_max = 20.;
    validate_constraint(
        strength_neighbor_repulsion, sn_min, sn_max, "strengthNeighborRepulsion", false);

    const auto range_neighbor_repulsion = current_state.range_neighbor_repulsion;
    constexpr double rn_min = 0.;
    constexpr double rn_max = 5.;
    validate_constraint(range_neighbor_repulsion, rn_min, rn_max, "rangeNeighborRepulsion", true);

    const auto buff = current_state.wall_buffer_distance;
    constexpr double buff_min = 0.;
    constexpr double buff_max = 1.;
    validate_constraint(buff, buff_min, buff_max, "wallBufferDistance", false);

    const auto v0 = current_state.v0;
    constexpr double v0_min = 0.;
    constexpr double v0_max = 10.;
    validate_constraint(v0, v0_min, v0_max, "v0");

    const auto time_gap = current_state.time_gap;
    constexpr double time_gap_min = 0.;
    constexpr double time_gap_max = 10.;
    validate_constraint(time_gap, time_gap_min, time_gap_max, "timeGap", true);

    const auto anticipation_time = current_state.anticipation_time;
    constexpr double anticipation_time_min = 0.0;
    constexpr double anticipation_time_max = 5.0;
    validate_constraint(
        anticipation_time, anticipation_time_min, anticipation_time_max, "anticipationTime");

    const auto reaction_time = current_state.reaction_time;
    constexpr double reaction_time_min = 0.0;
    constexpr double reaction_time_max = 1.0;
    validate_constraint(reaction_time, reaction_time_min, reaction_time_max, "reactionTime", true);

    const auto neighbors = view.other_agents_in_range(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<State>(*neighbor.state);
        const auto contanctd_dist = r + neighbor_model.radius;
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

double AnticipationVelocityModel::optimal_speed(
    const State& current_state,
    double spacing,
    double time_gap) const
{
    constexpr double creep_speed = 0.01;

    double speed = spacing / time_gap;

    if(std::abs(speed) < creep_speed) {
        // Random shuffle: forward, backward, or stop
        const auto r = _gen() % 3;
        speed = (r == 0) ? creep_speed : (r == 1) ? -creep_speed : 0.0;
    }

    return std::min(std::max(speed, -creep_speed), current_state.v0);
}
double AnticipationVelocityModel::get_spacing(
    const State& current_state,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& neighbor_state = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.relative_position;
    const auto in_front = direction.scalar_product(distp12) >= 0;
    if(!in_front) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.rotate90_deg();
    const auto buffer = 0.02;
    const auto l = current_state.radius + neighbor_state.radius + buffer;
    const bool in_corridor = std::abs(left.scalar_product(distp12)) <= l;
    if(!in_corridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.norm() - l;
}

Point AnticipationVelocityModel::calculate_influence_direction(
    const Point& desired_direction,
    const Point& predicted_direction) const
{
    // Eq. (5)
    const Point orthogonal_direction =
        Point(-desired_direction.y, desired_direction.x).normalized();
    const double alignment = orthogonal_direction.scalar_product(predicted_direction);
    Point influence_direction = orthogonal_direction;
    if(fabs(alignment) < j_eps) {
        // Choose a random direction (left or right)
        if(_gen() % 2 == 0) {
            influence_direction = -orthogonal_direction;
        }
    } else if(alignment > 0) {
        influence_direction = -orthogonal_direction;
    }
    return influence_direction;
}

Point AnticipationVelocityModel::neighbor_repulsion(
    const State& current_state,
    Point desired_direction,
    const NeighborView& neighbor) const
{
    const auto& neighbor_state = std::get<State>(*neighbor.state);

    const auto distp12 = neighbor.relative_position;
    const auto [distance, ep12] = distp12.norm_and_normalized();
    const double adjusted_dist = distance - (current_state.radius + neighbor_state.radius);

    // Pedestrian movement and desired directions
    const auto& e1 = current_state.orientation;
    const auto& d1 = desired_direction;
    const auto& e2 = neighbor_state.orientation;

    // Check perception range (Eq. 1)
    const auto in_perception_range = d1.scalar_product(ep12) >= 0 || e1.scalar_product(ep12) >= 0;
    if(!in_perception_range)
        return Point(0, 0);

    const double s_gap = (current_state.velocity - neighbor_state.velocity).scalar_product(ep12) *
                         current_state.anticipation_time;
    double r_dist = adjusted_dist - s_gap;
    r_dist = std::max(r_dist, 0.0); // Clamp to zero if negative

    // Interaction strength (Eq. 3 & 4)
    constexpr double alignment_base = 1.0;
    constexpr double alignment_weight = 0.5;
    const double alignment_factor =
        alignment_base + alignment_weight * (1.0 - d1.scalar_product(e2));
    const double interaction_strength = current_state.strength_neighbor_repulsion *
                                        alignment_factor *
                                        std::exp(-r_dist / current_state.range_neighbor_repulsion);
    const auto newep12 =
        distp12 + neighbor_state.velocity * neighbor_state.anticipation_time; // e_ij(t+ta)

    // Compute adjusted influence direction
    const auto influence_direction = calculate_influence_direction(d1, newep12);
    return influence_direction * interaction_strength;
}

Point AnticipationVelocityModel::handle_wall_avoidance(
    const Point& direction,
    const State& current_state,
    const AgentStep& step,
    double pushout_strength) const
{
    const double critical_wall_distance = current_state.wall_buffer_distance + current_state.radius;

    Point modified_direction = direction;
    for(const auto& wall : step.walls_in_range(critical_wall_distance)) {
        const auto dot_product = modified_direction.scalar_product(wall.normal);

        if(dot_product < 0) {
            // Direction points into wall - need to project it out
            // Remove the component pointing into the wall
            const auto projected_direction = modified_direction - wall.normal * dot_product;
            modified_direction = projected_direction + wall.normal * pushout_strength;
        }
    }

    // Renormalize to maintain speed
    const auto final_direction = modified_direction.normalized();

    return final_direction;
}
