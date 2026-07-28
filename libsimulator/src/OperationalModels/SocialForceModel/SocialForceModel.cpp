// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModel.hpp"

#include "EnvironmentQuery.hpp"
#include "LineSegment.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <cmath>
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
    const OperationalModelState& current,
    OperationalModelState& next,
    const Point& destination,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(current);
    auto forces = DrivingForce(state, destination);

    const auto neighborStates = envQuery.OtherAgentStatesInRange(
        current, _cutOffRadius, [&envQuery, from = state.position](const Point& to) {
            return envQuery.NoGeometryBetween(from, to);
        });

    Point F_rep;
    for(const auto& neighbor : neighborStates) {
        F_rep += AgentForce(state, std::get<State>(neighbor));
    }
    forces += F_rep / state.mass;

    const auto& walls = envQuery.LineSegmentsInRange(state.position);
    const auto obstacle_f = std::accumulate(
        std::begin(walls),
        std::end(walls),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + ObstacleForce(state, element);
        });
    forces += obstacle_f / state.mass;

    const auto velocity = state.velocity + forces * dT;
    auto& nextState = std::get<State>(next);
    nextState.position = state.position + velocity * dT;
    nextState.velocity = velocity;
}

void SocialForceModel::CheckModelConstraint(
    const OperationalModelState& generic_state,
    const EnvironmentQuery& envQuery) const
{
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

    const auto& state = std::get<State>(generic_state);

    throwIfNegative(state.mass, "mass");
    throwIfNegative(state.desiredSpeed, "desired speed");
    throwIfNegative(state.reactionTime, "reaction time");
    throwIfNegative(state.radius, "radius");

    const auto neighbors = envQuery.OtherAgentStatesInRange(generic_state, 2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighborState = std::get<State>(neighbor);
        const auto distance = (state.position - neighborState.position).Norm();

        if(state.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "radius {}",
                state.position,
                neighborState.position,
                distance,
                state.radius);
        }
    }
    const auto maxRadius = state.radius / 2;
    const auto lineSegments = envQuery.LineSegmentsInRange(state.position, maxRadius);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            state.position,
            state.radius);
    }
}

Point SocialForceModel::DrivingForce(const State& state, const Point& destination)
{
    const Point e0 = (destination - state.position).Normalized();
    return (e0 * state.desiredSpeed - state.velocity) / state.reactionTime;
}

double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const State& s1, const State& s2) const
{
    const double total_radius = s1.radius + s2.radius;

    return ForceBetweenPoints(
        s1.position,
        s2.position,
        s1.agentScale,
        s1.forceDistance,
        total_radius,
        s2.velocity - s1.velocity,
        this->bodyForce,
        this->friction);
}

Point SocialForceModel::ObstacleForce(const State& state, const LineSegment& segment) const
{
    const Point pt = segment.ShortestPoint(state.position);
    return ForceBetweenPoints(
        state.position,
        pt,
        state.obstacleScale,
        state.forceDistance,
        state.radius,
        state.velocity,
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
