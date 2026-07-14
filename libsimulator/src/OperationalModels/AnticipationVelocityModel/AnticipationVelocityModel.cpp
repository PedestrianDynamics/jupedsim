// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"

#include "AgentJourney.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Macros.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <vector>

AnticipationVelocityModel::AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed)
    : _pushoutStrength(pushoutStrength), gen(rng_seed)
{
}

OperationalModelType AnticipationVelocityModel::Type() const
{
    return OperationalModelType::ANTICIPATION_VELOCITY_MODEL;
}

void AnticipationVelocityModel::GetNeighbors(
    const GenericState& current,
    const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
    const CollisionGeometry& geometry,
    StateContainer& neighbor_states) const
{
    neighbor_states = neighborhoodsearch.GetNeighboringAgentStates(Pos(current), _cutOffRadius);
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(Pos(current));
    // Remove any agent from the neighborhood that is obstructed by geometry and the current
    // agent
    std::erase_if(neighbor_states, [&current, &boundary](const auto& neighbor) {
        if(Id(current) == Id(neighbor)) {
            return true;
        }
        const auto agent_to_neighbor =
            LineSegment(Pos(current), std::get<State>(neighbor).position);
        return std::any_of(
            boundary.cbegin(), boundary.cend(), [&agent_to_neighbor](const auto& segment) {
                return intersects(agent_to_neighbor, segment);
            });
    });
}

void AnticipationVelocityModel::ComputeNextState(
    double dT,
    const GenericState& current,
    GenericState& next,
    const AgentJourney& journey,
    const CollisionGeometry& geometry,
    const StateContainer& neighborStates) const
{
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(Pos(current));
    const auto& state = std::get<State>(current);

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        Point{},
        [&state, &journey, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(state, std::get<State>(neighbor), journey);
        });

    const auto desiredDirection = (journey.destination - Pos(current)).Normalized();
    auto direction = (desiredDirection + neighborRepulsion).Normalized();
    if(direction == Point{}) {
        direction = state.orientation;
    }

    const double wallBufferDistance = state.wallBufferDistance;
    // Wall sliding behavior

    direction = UpdateDirection(state, journey, direction, dT);
    const auto spacing = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        std::numeric_limits<double>::max(),
        [&state, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(state, std::get<State>(neighbor), direction));
        });

    const auto optimal_speed = OptimalSpeed(state, spacing, state.timeGap);
    direction = HandleWallAvoidance(
        direction, state.position, state.radius, boundary, wallBufferDistance, _pushoutStrength);

    const auto velocity = direction * optimal_speed;
    auto& nextState = std::get<State>(next);
    nextState.position = Pos(current) + velocity * dT;
    nextState.orientation = direction;
    nextState.velocity = velocity;
}

Point AnticipationVelocityModel::UpdateDirection(
    const State& state,
    const AgentJourney& journey,
    const Point& calculatedDirection,
    double dt) const
{
    const Point desiredDirection = (journey.destination - state.position).Normalized();
    const Point actualDirection = state.orientation;
    Point updatedDirection;

    if(desiredDirection.ScalarProduct(calculatedDirection) *
           desiredDirection.ScalarProduct(actualDirection) <
       0) {
        updatedDirection = calculatedDirection;
    } else {
        // Compute the rate of change of direction (Eq. 7)
        const Point directionDerivative =
            (calculatedDirection.Normalized() - actualDirection) / state.reactionTime;
        updatedDirection = actualDirection + directionDerivative * dt;
    }

    return updatedDirection.Normalized();
}

void AnticipationVelocityModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& state = std::get<State>(agent.state);
    const auto r = state.radius;
    constexpr double rMin = 0.;
    constexpr double rMax = 2.;
    validateConstraint(r, rMin, rMax, "radius", true);

    const auto strengthNeighborRepulsion = state.strengthNeighborRepulsion;
    constexpr double snMin = 0.;
    constexpr double snMax = 20.;
    validateConstraint(strengthNeighborRepulsion, snMin, snMax, "strengthNeighborRepulsion", false);

    const auto rangeNeighborRepulsion = state.rangeNeighborRepulsion;
    constexpr double rnMin = 0.;
    constexpr double rnMax = 5.;
    validateConstraint(rangeNeighborRepulsion, rnMin, rnMax, "rangeNeighborRepulsion", true);

    const auto buff = state.wallBufferDistance;
    constexpr double buffMin = 0.;
    constexpr double buffMax = 1.;
    validateConstraint(buff, buffMin, buffMax, "wallBufferDistance", false);

    const auto v0 = state.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = state.timeGap;
    constexpr double timeGapMin = 0.;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap", true);

    const auto anticipationTime = state.anticipationTime;
    constexpr double anticipationTimeMin = 0.0;
    constexpr double anticipationTimeMax = 5.0;
    validateConstraint(
        anticipationTime, anticipationTimeMin, anticipationTimeMax, "anticipationTime");

    const auto reactionTime = state.reactionTime;
    constexpr double reactionTimeMin = 0.0;
    constexpr double reactionTimeMax = 1.0;
    validateConstraint(reactionTime, reactionTimeMin, reactionTimeMax, "reactionTime", true);

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(state.position, 2);
    for(const auto& neighbor : neighbors) {
        if(Id(agent) == Id(neighbor)) {
            continue;
        }
        const auto& neighbor_state = std::get<State>(neighbor.state);
        const auto contanctdDist = r + neighbor_state.radius;
        const auto distance = (state.position - neighbor_state.position).Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                state.position,
                neighbor_state.position,
                distance);
        }
    }

    const auto lineSegments = geometry.LineSegmentsInDistanceTo(r, state.position);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            state.position,
            r);
    }
}

double
AnticipationVelocityModel::OptimalSpeed(const State& state, double spacing, double time_gap) const
{
    constexpr double creep_speed = 0.01;

    double speed = spacing / time_gap;

    if(std::abs(speed) < creep_speed) {
        // Random shuffle: forward, backward, or stop
        const auto r = gen() % 3;
        speed = (r == 0) ? creep_speed : (r == 1) ? -creep_speed : 0.0;
    }

    return std::min(std::max(speed, -creep_speed), state.v0);
}

double AnticipationVelocityModel::GetSpacing(
    const State& state1,
    const State& state2,
    const Point& direction) const
{
    const auto distp12 = state2.position - state1.position;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto buffer = 0.02;
    const auto l = state1.radius + state2.radius + buffer;
    const bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}

Point AnticipationVelocityModel::CalculateInfluenceDirection(
    const Point& desiredDirection,
    const Point& predictedDirection) const
{
    // Eq. (5)
    const Point orthogonalDirection = Point(-desiredDirection.y, desiredDirection.x).Normalized();
    const double alignment = orthogonalDirection.ScalarProduct(predictedDirection);
    Point influenceDirection = orthogonalDirection;
    if(fabs(alignment) < J_EPS) {
        // Choose a random direction (left or right)
        if(gen() % 2 == 0) {
            influenceDirection = -orthogonalDirection;
        }
    } else if(alignment > 0) {
        influenceDirection = -orthogonalDirection;
    }
    return influenceDirection;
}

Point AnticipationVelocityModel::NeighborRepulsion(
    const State& state1,
    const State& state2,
    const AgentJourney& journey) const
{
    const auto distp12 = state2.position - state1.position;
    const auto [distance, ep12] = distp12.NormAndNormalized();
    const double adjustedDist = distance - (state1.radius + state2.radius);

    // Pedestrian movement and desired directions
    const auto& e1 = state1.orientation;
    const auto& d1 = (journey.destination - state1.position).Normalized();
    const auto& e2 = state2.orientation;

    // Check perception range (Eq. 1)
    const auto inPerceptionRange = d1.ScalarProduct(ep12) >= 0 || e1.ScalarProduct(ep12) >= 0;
    if(!inPerceptionRange)
        return Point(0, 0);

    const double S_Gap =
        (state1.velocity - state2.velocity).ScalarProduct(ep12) * state1.anticipationTime;
    double R_dist = adjustedDist - S_Gap;
    R_dist = std::max(R_dist, 0.0); // Clamp to zero if negative

    // Interaction strength (Eq. 3 & 4)
    constexpr double alignmentBase = 1.0;
    constexpr double alignmentWeight = 0.5;
    const double alignmentFactor = alignmentBase + alignmentWeight * (1.0 - d1.ScalarProduct(e2));
    const double interactionStrength = state1.strengthNeighborRepulsion * alignmentFactor *
                                       std::exp(-R_dist / state1.rangeNeighborRepulsion);
    const auto newep12 = distp12 + state2.velocity * state2.anticipationTime; // e_ij(t+ta)

    // Compute adjusted influence direction
    const auto influenceDirection = CalculateInfluenceDirection(d1, newep12);
    return influenceDirection * interactionStrength;
}

Point AnticipationVelocityModel::HandleWallAvoidance(
    const Point& direction,
    const Point& agentPosition,
    double agentRadius,
    const std::vector<LineSegment>& boundary,
    double wallBufferDistance,
    double pushoutStrength) const
{
    const double criticalWallDistance = wallBufferDistance + agentRadius;

    Point modifiedDirection = direction;

    for(const auto& wall : boundary) {
        const auto closestPoint = wall.ShortestPoint(agentPosition);

        const auto distanceVector = agentPosition - closestPoint;
        const auto [distance, normalTowardAgent] = distanceVector.NormAndNormalized();

        if(distance > criticalWallDistance) {
            continue;
        }

        const auto dotProduct = modifiedDirection.ScalarProduct(normalTowardAgent);

        if(dotProduct < 0) {
            // Direction points into wall - need to project it out
            // Remove the component pointing into the wall
            const auto projectedDirection = modifiedDirection - normalTowardAgent * dotProduct;
            modifiedDirection = projectedDirection + normalTowardAgent * pushoutStrength;
        }
    }

    // Renormalize to maintain speed
    const auto finalDirection = modifiedDirection.Normalized();

    return finalDirection;
}
