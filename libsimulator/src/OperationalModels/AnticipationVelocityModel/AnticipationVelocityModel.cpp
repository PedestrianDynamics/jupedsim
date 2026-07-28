// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"

#include "EnvironmentQuery.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Macros.hpp"
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

void AnticipationVelocityModel::ComputeNextState(
    double dT,
    const OperationalModelState& current,
    OperationalModelState& next,
    const Point& destination,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(current);
    const auto& boundary = envQuery.LineSegmentsInRange(state.position);
    const auto neighborStates = envQuery.OtherAgentStatesInRange(
        current, _cutOffRadius, [&envQuery, from = state.position](const Point& to) {
            return envQuery.NoGeometryBetween(from, to);
        });

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        Point{},
        [&state, &destination, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(state, std::get<State>(neighbor), destination);
        });

    const auto desiredDirection = (destination - state.position).Normalized();
    auto direction = (desiredDirection + neighborRepulsion).Normalized();
    if(direction == Point{}) {
        direction = state.orientation;
    }

    direction = UpdateDirection(state, destination, direction, dT);
    const auto spacing = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        std::numeric_limits<double>::max(),
        [&state, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(state, std::get<State>(neighbor), direction));
        });

    const auto optimal_speed = OptimalSpeed(state, spacing, state.timeGap);
    direction = HandleWallAvoidance(
        direction,
        state.position,
        state.radius,
        boundary,
        state.wallBufferDistance,
        _pushoutStrength);

    const auto velocity = direction * optimal_speed;
    auto& nextState = std::get<State>(next);
    nextState.position = state.position + velocity * dT;
    nextState.orientation = direction;
    nextState.velocity = velocity;
}

Point AnticipationVelocityModel::UpdateDirection(
    const State& state,
    const Point& destination,
    const Point& calculatedDirection,
    double dt) const
{
    const Point desiredDirection = (destination - state.position).Normalized();
    const Point actualDirection = state.orientation;
    Point updatedDirection;

    if(desiredDirection.ScalarProduct(calculatedDirection) *
           desiredDirection.ScalarProduct(actualDirection) <
       0) {
        updatedDirection = calculatedDirection;
    } else {
        const Point directionDerivative =
            (calculatedDirection.Normalized() - actualDirection) / state.reactionTime;
        updatedDirection = actualDirection + directionDerivative * dt;
    }

    return updatedDirection.Normalized();
}

void AnticipationVelocityModel::CheckModelConstraint(
    const OperationalModelState& generic_state,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(generic_state);
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

    const auto neighbors = envQuery.OtherAgentStatesInRange(generic_state, 2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_state = std::get<State>(neighbor);
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

    const auto lineSegments = envQuery.LineSegmentsInRange(state.position, r);
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
        const auto r = gen() % 3;
        speed = (r == 0) ? creep_speed : (r == 1) ? -creep_speed : 0.0;
    }

    return std::min(std::max(speed, -creep_speed), state.v0);
}

double AnticipationVelocityModel::GetSpacing(
    const State& s1,
    const State& s2,
    const Point& direction) const
{
    const auto distp12 = s2.position - s1.position;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto buffer = 0.02;
    const auto l = s1.radius + s2.radius + buffer;
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
    const Point orthogonalDirection = Point(-desiredDirection.y, desiredDirection.x).Normalized();
    const double alignment = orthogonalDirection.ScalarProduct(predictedDirection);
    Point influenceDirection = orthogonalDirection;
    if(fabs(alignment) < J_EPS) {
        if(gen() % 2 == 0) {
            influenceDirection = -orthogonalDirection;
        }
    } else if(alignment > 0) {
        influenceDirection = -orthogonalDirection;
    }
    return influenceDirection;
}

Point AnticipationVelocityModel::NeighborRepulsion(
    const State& s1,
    const State& s2,
    const Point& destination) const
{
    const auto distp12 = s2.position - s1.position;
    const auto [distance, ep12] = distp12.NormAndNormalized();
    const double adjustedDist = distance - (s1.radius + s2.radius);

    const auto& e1 = s1.orientation;
    const auto& d1 = (destination - s1.position).Normalized();
    const auto& e2 = s2.orientation;

    const auto inPerceptionRange = d1.ScalarProduct(ep12) >= 0 || e1.ScalarProduct(ep12) >= 0;
    if(!inPerceptionRange)
        return Point(0, 0);

    const double S_Gap = (s1.velocity - s2.velocity).ScalarProduct(ep12) * s1.anticipationTime;
    double R_dist = adjustedDist - S_Gap;
    R_dist = std::max(R_dist, 0.0);

    constexpr double alignmentBase = 1.0;
    constexpr double alignmentWeight = 0.5;
    const double alignmentFactor = alignmentBase + alignmentWeight * (1.0 - d1.ScalarProduct(e2));
    const double interactionStrength = s1.strengthNeighborRepulsion * alignmentFactor *
                                       std::exp(-R_dist / s1.rangeNeighborRepulsion);
    const auto newep12 = distp12 + s2.velocity * s2.anticipationTime;

    const auto influenceDirection = CalculateInfluenceDirection(d1, newep12);
    return influenceDirection * interactionStrength;
}

Point AnticipationVelocityModel::HandleWallAvoidance(
    const Point& direction,
    const Point& agentPosition,
    double agentRadius,
    const auto& boundary,
    double wallBufferDistance,
    double pushoutStrength) const
{
    const double criticalWallDistance = wallBufferDistance + agentRadius;

    Point modifiedDirection = direction;
    std::for_each(
        std::begin(boundary),
        std::end(boundary),
        [&agentPosition, &criticalWallDistance, &modifiedDirection, pushoutStrength](
            const LineSegment& wall) {
            const auto closestPoint = wall.ShortestPoint(agentPosition);

            const auto distanceVector = agentPosition - closestPoint;
            const auto [distance, normalTowardAgent] = distanceVector.NormAndNormalized();

            if(distance > criticalWallDistance) {
                return;
            }

            const auto dotProduct = modifiedDirection.ScalarProduct(normalTowardAgent);

            if(dotProduct < 0) {
                const auto projectedDirection = modifiedDirection - normalTowardAgent * dotProduct;
                modifiedDirection = projectedDirection + normalTowardAgent * pushoutStrength;
            }
        });

    return modifiedDirection.Normalized();
}
