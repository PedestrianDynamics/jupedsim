// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModel.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <cmath>
#include <string>

namespace
{
/// How far to ask for walls. The obstacle force decays with `forceDistance` (0.08 m by
/// default), so a wall this far off contributes "nothing".
constexpr double WallSearchRadius = 4.0;
} // namespace

SocialForceModel::SocialForceModel(double bodyForce, double friction)
    : bodyForce(bodyForce), friction(friction)
{
}

OperationalModelType SocialForceModel::Type() const
{
    return OperationalModelType::SOCIAL_FORCE;
}

Point SocialForceModel::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& currentState = std::get<State>(current);
    auto forces = DrivingForce(currentState, step.orientation_to_next_target());

    auto neighborhood = step.OtherAgentsInRange(
        _cutOffRadius, [&step](const NeighborView& n) { return step.NoGeometryBetween(n); });
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        F_rep += AgentForce(currentState, neighbor);
    }
    forces += F_rep / currentState.mass;
    Point obstacle_f{};
    for(const auto& wall : step.WallsInRange(WallSearchRadius)) {
        obstacle_f += ObstacleForce(currentState, wall);
    }
    forces += obstacle_f / currentState.mass;

    const auto velocity = currentState.velocity + forces * step.dt();
    std::get<State>(next).velocity = velocity;
    return velocity * step.dt();
}

void SocialForceModel::CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const
{
    // none of these constraint are given by the paper but are useful to create a simulation that
    // does not break immediately
    auto throwIfNegative = [](double value, std::string name) {
        if(value < 0) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be positive",
                name,
                value,
                name);
        }
    };

    const auto& currentState = std::get<State>(agent.state);

    const auto mass = currentState.mass;
    throwIfNegative(mass, "mass");

    const auto desiredSpeed = currentState.desiredSpeed;
    throwIfNegative(desiredSpeed, "desired speed");

    const auto reactionTime = currentState.reactionTime;
    throwIfNegative(reactionTime, "reaction time");

    const auto radius = currentState.radius;
    throwIfNegative(radius, "radius");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto distance = neighbor.RelativePosition.Norm();

        if(currentState.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: distance {}, "
                "radius {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.RelativePosition,
                distance,
                currentState.radius);
        }
    }
    const auto maxRadius = currentState.radius / 2;
    if(!view.WallsInRange(maxRadius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance < "
            "{}/2",
            agent.location.xy(),
            currentState.radius);
    }
}

Point SocialForceModel::DrivingForce(const State& currentState, Point e0)
{
    return (e0 * currentState.desiredSpeed - currentState.velocity) / currentState.reactionTime;
};
double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const State& currentState, const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);

    const double total_radius = currentState.radius + other.radius;

    return ForceFromSeparation(
        -neighbor.RelativePosition,
        currentState.agentScale,
        currentState.forceDistance,
        total_radius,
        other.velocity - currentState.velocity,
        this->bodyForce,
        this->friction);
};

Point SocialForceModel::ObstacleForce(const State& currentState, const WallView& wall) const
{
    return ForceFromSeparation(
        -wall.closest_point,
        currentState.obstacleScale,
        currentState.forceDistance,
        currentState.radius,
        currentState.velocity,
        this->bodyForce,
        this->friction);
}

Point SocialForceModel::ForceFromSeparation(
    const Point separation,
    const double A,
    const double B,
    const double radius,
    const Point velocity,
    const double bodyForce,
    const double friction)
{
    // todo reduce range of force to 180 degrees
    const double dist = separation.Norm();
    double pushing_force_length = PushingForceLength(A, B, radius, dist);
    double friction_force_length = 0;
    const Point n_ij = separation.Normalized();
    const Point tangent = n_ij.Rotate90Deg();
    if(dist < radius) {
        pushing_force_length += bodyForce * (radius - dist);
        friction_force_length = friction * (radius - dist) * (velocity.ScalarProduct(tangent));
    }
    return n_ij * pushing_force_length + tangent * friction_force_length;
}
