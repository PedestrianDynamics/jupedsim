// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModel.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <cmath>
#include <iterator>
#include <numeric>
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
    const auto& model = std::get<State>(current);
    auto forces = DrivingForce(model, step.ToNextTarget());

    const auto& walls = step.WallsNearby();
    auto neighborhood = step.OtherAgentsInRange(_cutOffRadius, [&step, &walls](const NeighborView& n) {
        return step.NoGeometryBetween(n.RelativePosition, walls);
    });
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        F_rep += AgentForce(model, neighbor);
    }
    forces += F_rep / model.mass;

    const auto obstacle_f = std::accumulate(
        std::begin(walls),
        std::end(walls),
        Point(0, 0),
        [this, &model](const auto& acc, const auto& element) {
            return acc + ObstacleForce(model, element);
        });
    forces += obstacle_f / model.mass;

    const auto velocity = model.velocity + forces * step.dt();
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

    const auto& model = std::get<State>(agent.model);

    const auto mass = model.mass;
    throwIfNegative(mass, "mass");

    const auto desiredSpeed = model.desiredSpeed;
    throwIfNegative(desiredSpeed, "desired speed");

    const auto reactionTime = model.reactionTime;
    throwIfNegative(reactionTime, "reaction time");

    const auto radius = model.radius;
    throwIfNegative(radius, "radius");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto distance = neighbor.RelativePosition.Norm();

        if(model.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "radius {}",
                agent.position,
                agent.position + neighbor.RelativePosition,
                distance,
                model.radius);
        }
    }
    const auto maxRadius = model.radius / 2;
    const auto lineSegments = view.WallsInRange(maxRadius);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            agent.position,
            model.radius);
    }
}

Point SocialForceModel::DrivingForce(const State& self, Point ToNextTarget)
{
    const Point e0 = ToNextTarget.Normalized();
    return (e0 * self.desiredSpeed - self.velocity) / self.reactionTime;
};
double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const State& self, const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);

    const double total_radius = self.radius + other.radius;

    return ForceFromSeparation(
        -neighbor.RelativePosition,
        self.agentScale,
        self.forceDistance,
        total_radius,
        other.velocity - self.velocity,
        this->bodyForce,
        this->friction);
};

Point SocialForceModel::ObstacleForce(const State& self, const LineSegment& segment) const
{
    const Point pt = segment.ShortestPoint(Point{});
    return ForceBetweenPoints(
        Point{},
        pt,
        self.obstacleScale,
        self.forceDistance,
        self.radius,
        self.velocity,
        this->bodyForce,
        this->friction);
}

Point SocialForceModel::ForceBetweenPoints(
    const Point pt1,
    const Point pt2,
    const double A,
    const double B,
    const double radius,
    const Point velocity,
    const double bodyForce,
    const double friction)
{
    return ForceFromSeparation(pt1 - pt2, A, B, radius, velocity, bodyForce, friction);
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
