// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModel.hpp"

#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "NeighborQuery.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <cmath>
#include <iterator>
#include <numeric>
#include <string>
#include <variant>

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
    const OperationalModelState& current,
    OperationalModelState& next,
    Point destination,
    const CollisionGeometry& geometry,
    const NeighborQuery& neighborQuery) const
{
    const auto& model = std::get<State>(current);
    auto forces = DrivingForce(model, destination);

    const auto neighborhood = neighborQuery(model.position, this->_cutOffRadius);
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        F_rep += AgentForce(model, std::get<State>(neighbor));
    }
    forces += F_rep / model.mass;
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(model.position);

    const auto obstacle_f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &model](const auto& acc, const auto& element) {
            return acc + ObstacleForce(model, element);
        });
    forces += obstacle_f / model.mass;

    const auto velocity = model.velocity + forces * dT;
    auto& nextModel = std::get<State>(next);
    nextModel.position = model.position + velocity * dT;
    nextModel.velocity = velocity;
}

void SocialForceModel::CheckModelConstraint(
    const OperationalModelState& state,
    const NeighborQuery& neighborQuery,
    const CollisionGeometry& geometry) const
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

    const auto& model = std::get<State>(state);

    const auto mass = model.mass;
    throwIfNegative(mass, "mass");

    const auto desiredSpeed = model.desiredSpeed;
    throwIfNegative(desiredSpeed, "desired speed");

    const auto reactionTime = model.reactionTime;
    throwIfNegative(reactionTime, "reaction time");

    const auto radius = model.radius;
    throwIfNegative(radius, "radius");

    const auto neighbors = neighborQuery(model.position, 2);
    for(const auto& neighbor : neighbors) {
        const auto& neighborPosition = std::get<State>(neighbor).position;
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
    const auto lineSegments = geometry.LineSegmentsInDistanceTo(maxRadius, model.position);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            model.position,
            model.radius);
    }
}

Point SocialForceModel::DrivingForce(const State& model, Point destination)
{
    const Point e0 = (destination - model.position).Normalized();
    return (e0 * model.desiredSpeed - model.velocity) / model.reactionTime;
};
double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const State& model1, const State& model2) const
{
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

Point SocialForceModel::ObstacleForce(const State& model, const LineSegment& segment) const
{
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
