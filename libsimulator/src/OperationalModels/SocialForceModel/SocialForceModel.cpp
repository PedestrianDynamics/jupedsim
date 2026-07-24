// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModel.hpp"

#include "EnvironmentQuery.hpp"
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

void SocialForceModel::ComputeNextState(
    double dT,
    const GenericAgent& current,
    GenericAgent& next,
    const EnvironmentQuery& envQuery) const
{
    const auto& model = std::get<State>(current.model);
    auto forces = DrivingForce(current);

    auto neighborhood = envQuery.OtherAgentsInRange(
        model.position, _cutOffRadius, [&envQuery, from = model.position](const Point& to) {
            return envQuery.NoGeometryBetween(from, to);
        });
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        F_rep += AgentForce(current, neighbor);
    }
    forces += F_rep / model.mass;
    const auto& walls = envQuery.LineSegmentsInRange(model.position);

    const auto obstacle_f = std::accumulate(
        std::begin(walls),
        std::end(walls),
        Point(0, 0),
        [this, &current](const auto& acc, const auto& element) {
            return acc + ObstacleForce(current, element);
        });
    forces += obstacle_f / model.mass;

    const auto velocity = model.velocity + forces * dT;
    auto& nextModel = std::get<State>(next.model);
    nextModel.position = model.position + velocity * dT;
    nextModel.velocity = velocity;
}

void SocialForceModel::CheckModelConstraint(
    const GenericAgent& agent,
    const EnvironmentQuery& envQuery) const
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

    const auto neighbors = envQuery.OtherAgentsInRange(agent.position(), 2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighborPosition = std::get<State>(neighbor.model).position;
        const auto distance = (model.position - neighborPosition).Norm();

        if(model.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "radius {}",
                model.position,
                neighborPosition,
                distance,
                model.radius);
        }
    }
    const auto maxRadius = model.radius / 2;
    const auto lineSegments = envQuery.LineSegmentsInRange(model.position, maxRadius);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            model.position,
            model.radius);
    }
}

Point SocialForceModel::DrivingForce(const GenericAgent& agent)
{
    const auto& model = std::get<State>(agent.model);
    const Point e0 = (agent.nextTarget - model.position).Normalized();
    return (e0 * model.desiredSpeed - model.velocity) / model.reactionTime;
};
double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const GenericAgent& ped1, const GenericAgent& ped2) const
{
    const auto& model1 = std::get<State>(ped1.model);
    const auto& model2 = std::get<State>(ped2.model);

    const double total_radius = model1.radius + model2.radius;

    return ForceBetweenPoints(
        model1.position,
        model2.position,
        model1.agentScale,
        model1.forceDistance,
        total_radius,
        model2.velocity - model1.velocity,
        this->bodyForce,
        this->friction);
};

Point SocialForceModel::ObstacleForce(const GenericAgent& agent, const LineSegment& segment) const
{
    const auto& model = std::get<State>(agent.model);
    const Point pt = segment.ShortestPoint(model.position);
    return ForceBetweenPoints(
        model.position,
        pt,
        model.obstacleScale,
        model.forceDistance,
        model.radius,
        model.velocity,
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
    // todo reduce range of force to 180 degrees
    const double dist = (pt1 - pt2).Norm();
    double pushing_force_length = PushingForceLength(A, B, radius, dist);
    double friction_force_length = 0;
    const Point n_ij = (pt1 - pt2).Normalized();
    const Point tangent = n_ij.Rotate90Deg();
    if(dist < radius) {
        pushing_force_length += bodyForce * (radius - dist);
        friction_force_length = friction * (radius - dist) * (velocity.ScalarProduct(tangent));
    }
    return n_ij * pushing_force_length + tangent * friction_force_length;
}
