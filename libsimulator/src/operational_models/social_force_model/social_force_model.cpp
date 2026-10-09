// SPDX-License-Identifier: LGPL-3.0-or-later
#include "social_force_model.hpp"

#include "agent_view.hpp"
#include "generic_agent.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"
#include "simulation_error.hpp"

#include <cmath>
#include <string>

namespace
{
/// How far to ask for walls. The obstacle force decays with `force_distance` (0.08 m by
/// default), so a wall this far off contributes "nothing".
constexpr double wall_search_radius = 4.0;
} // namespace

SocialForceModel::SocialForceModel(double body_force, double friction)
    : _body_force(body_force), _friction(friction)
{
}

OperationalModelType SocialForceModel::type() const
{
    return OperationalModelType::SocialForce;
}

Point SocialForceModel::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& current_state = std::get<State>(current);
    auto forces = driving_force(current_state, step.route_orientation());

    auto neighborhood = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });
    Point f_rep;
    for(const auto& neighbor : neighborhood) {
        f_rep += agent_force(current_state, neighbor);
    }
    forces += f_rep / current_state.mass;
    Point obstacle_f{};
    for(const auto& wall : step.walls_in_range(wall_search_radius)) {
        obstacle_f += obstacle_force(current_state, wall);
    }
    forces += obstacle_f / current_state.mass;

    const auto velocity = current_state.velocity + forces * step.dt();
    std::get<State>(next).velocity = velocity;
    return velocity * step.dt();
}

void SocialForceModel::check_model_constraint(const GenericAgent& agent, const AgentView& view)
    const
{
    // none of these constraint are given by the paper but are useful to create a simulation that
    // does not break immediately
    auto throw_if_negative = [](double value, std::string name) {
        if(value < 0) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be positive",
                name,
                value,
                name);
        }
    };

    const auto& current_state = std::get<State>(agent.state);

    const auto mass = current_state.mass;
    throw_if_negative(mass, "mass");

    const auto desired_speed = current_state.desired_speed;
    throw_if_negative(desired_speed, "desired speed");

    const auto reaction_time = current_state.reaction_time;
    throw_if_negative(reaction_time, "reaction time");

    const auto radius = current_state.radius;
    throw_if_negative(radius, "radius");

    const auto neighbors = view.other_agents_in_range(2.0);
    for(const auto& neighbor : neighbors) {
        const auto distance = neighbor.relative_position.norm();

        if(current_state.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: distance {}, "
                "radius {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.relative_position,
                distance,
                current_state.radius);
        }
    }
    const auto max_radius = current_state.radius / 2;
    if(!view.walls_in_range(max_radius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance < "
            "{}/2",
            agent.location.xy(),
            current_state.radius);
    }
}

Point SocialForceModel::driving_force(const State& current_state, Point e0)
{
    return (e0 * current_state.desired_speed - current_state.velocity) /
           current_state.reaction_time;
};
double SocialForceModel::pushing_force_length(double a, double b, double r, double distance)
{
    return a * exp((r - distance) / b);
}

Point SocialForceModel::agent_force(const State& current_state, const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);

    const double total_radius = current_state.radius + other.radius;

    return force_from_separation(
        -neighbor.relative_position,
        current_state.agent_scale,
        current_state.force_distance,
        total_radius,
        other.velocity - current_state.velocity,
        this->_body_force,
        this->_friction);
};

Point SocialForceModel::obstacle_force(const State& current_state, const WallView& wall) const
{
    return force_from_separation(
        -wall.closest_point,
        current_state.obstacle_scale,
        current_state.force_distance,
        current_state.radius,
        -current_state.velocity,
        this->_body_force,
        this->_friction);
}

Point SocialForceModel::force_from_separation(
    const Point separation,
    const double a,
    const double b,
    const double radius,
    const Point velocity,
    const double body_force,
    const double friction)
{
    // todo reduce range of force to 180 degrees
    const double dist = separation.norm();
    double pushing_force_length = SocialForceModel::pushing_force_length(a, b, radius, dist);
    double friction_force_length = 0;
    const Point n_ij = separation.normalized();
    const Point tangent = n_ij.rotate90_deg();
    if(dist < radius) {
        pushing_force_length += body_force * (radius - dist);
        friction_force_length = friction * (radius - dist) * (velocity.scalar_product(tangent));
    }
    return n_ij * pushing_force_length + tangent * friction_force_length;
}
