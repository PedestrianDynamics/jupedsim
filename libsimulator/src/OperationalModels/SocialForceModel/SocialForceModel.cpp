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
    const auto& currState = std::get<State>(current);
    auto forces = DrivingForce(currState, step.ToNextTarget());

    const auto& walls = step.WallsNearby();
    auto neighborhood =
        step.OtherAgentsInRange(_cutOffRadius, [&step, &walls](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, walls);
        });
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        F_rep += AgentForce(currState, neighbor);
    }
    forces += F_rep / currState.mass;
    Point obstacle_f{};
    for(const auto& wall : walls) {
        obstacle_f += ObstacleForce(currState, wall);
    }
    forces += obstacle_f / currState.mass;

    const auto velocity = currState.velocity + forces * step.dt();
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

    const auto& currState = std::get<State>(agent.state);

    const auto mass = currState.mass;
    throwIfNegative(mass, "mass");

    const auto desiredSpeed = currState.desiredSpeed;
    throwIfNegative(desiredSpeed, "desired speed");

    const auto reactionTime = currState.reactionTime;
    throwIfNegative(reactionTime, "reaction time");

    const auto radius = currState.radius;
    throwIfNegative(radius, "radius");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto distance = neighbor.RelativePosition.Norm();

        if(currState.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "radius {}",
                agent.Position(),
                agent.Position() + neighbor.RelativePosition,
                distance,
                currState.radius);
        }
    }
    const auto maxRadius = currState.radius / 2;
    if(!view.WallsInRange(maxRadius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            agent.Position(),
            currState.radius);
    }
}

Point SocialForceModel::DrivingForce(const State& currState, Point ToNextTarget)
{
    const Point e0 = ToNextTarget.Normalized();
    return (e0 * currState.desiredSpeed - currState.velocity) / currState.reactionTime;
};
double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const State& currState, const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);

    const double total_radius = currState.radius + other.radius;

    return ForceFromSeparation(
        -neighbor.RelativePosition,
        currState.agentScale,
        currState.forceDistance,
        total_radius,
        other.velocity - currState.velocity,
        this->bodyForce,
        this->friction);
};

Point SocialForceModel::ObstacleForce(const State& currState, const WallView& wall) const
{
    return ForceFromSeparation(
        -wall.closest_point,
        currState.obstacleScale,
        currState.forceDistance,
        currState.radius,
        currState.velocity,
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
