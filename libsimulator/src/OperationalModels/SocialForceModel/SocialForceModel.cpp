// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModel.hpp"

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
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

void SocialForceModel::GetNeighbors(
    const GenericState& current,
    const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
    const CollisionGeometry& /*geometry*/,
    StateContainer& neighbor_states) const
{
    neighbor_states = neighborhoodsearch.GetNeighboringAgentStates(Pos(current), _cutOffRadius);
    neighbor_states.erase(
        std::remove_if(
            std::begin(neighbor_states),
            std::end(neighbor_states),
            [&current](const auto& neighbor) { return Id(current) == Id(neighbor); }),
        std::end(neighbor_states));
}

void SocialForceModel::ComputeNextState(
    double dT,
    const GenericState& current,
    GenericState& next,
    const AgentJourney& journey,
    const CollisionGeometry& geometry,
    const StateContainer& neighborStates) const
{
    const auto& state = std::get<State>(current);
    auto forces = DrivingForce(state, journey);

    Point F_rep;
    for(const auto& neighbor : neighborStates) {
        const auto& neighbor_state = std::get<State>(neighbor);
        F_rep += AgentForce(state, neighbor_state);
    }
    forces += F_rep / state.mass;
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(state.position);

    const auto obstacle_f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + ObstacleForce(state, element);
        });
    forces += obstacle_f / state.mass;

    const auto velocity = state.velocity + forces * dT;
    auto& nextState = std::get<State>(next);
    nextState.position = Pos(current) + velocity * dT;
    nextState.velocity = velocity;
}

void SocialForceModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
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

    const auto& state = std::get<State>(agent.state);

    const auto mass = state.mass;
    throwIfNegative(mass, "mass");

    const auto desiredSpeed = state.desiredSpeed;
    throwIfNegative(desiredSpeed, "desired speed");

    const auto reactionTime = state.reactionTime;
    throwIfNegative(reactionTime, "reaction time");

    const auto radius = state.radius;
    throwIfNegative(radius, "radius");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(state.position, 2);
    for(const auto& neighbor : neighbors) {
        const auto& neighborPosition = std::get<State>(neighbor.state).position;
        const auto distance = (state.position - neighborPosition).Norm();

        if(state.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "radius {}",
                state.position,
                neighborPosition,
                distance,
                state.radius);
        }
    }
    const auto maxRadius = state.radius / 2;
    const auto lineSegments = geometry.LineSegmentsInDistanceTo(maxRadius, state.position);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            state.position,
            state.radius);
    }
}

Point SocialForceModel::DrivingForce(const State& state, const AgentJourney& journey)
{
    const Point e0 = (journey.destination - state.position).Normalized();
    return (e0 * state.desiredSpeed - state.velocity) / state.reactionTime;
}
double SocialForceModel::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const State& state1, const State& state2) const
{

    const double total_radius = state1.radius + state2.radius;

    return ForceBetweenPoints(
        state1.position,
        state2.position,
        state1.agentScale,
        state1.forceDistance,
        total_radius,
        state2.velocity - state1.velocity,
        this->bodyForce,
        this->friction);
};

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
