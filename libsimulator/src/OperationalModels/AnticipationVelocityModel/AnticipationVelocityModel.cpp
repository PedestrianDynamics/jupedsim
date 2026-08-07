// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Macros.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <vector>

AnticipationVelocityModel::AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed)
    : _pushoutStrength(pushoutStrength), gen(rng_seed)
{
}

OperationalModelType AnticipationVelocityModel::Type() const
{
    return OperationalModelType::ANTICIPATION_VELOCITY_MODEL;
}

Point AnticipationVelocityModel::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& currentState = std::get<State>(current);
    auto _w = step.WallsNearby();
    const std::vector<WallView> boundaries(_w.begin(), _w.end());
    // Exclude occluded and self agents
    auto neighborhood =
        step.OtherAgentsInRange(_cutOffRadius, [&step, &boundaries](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, boundaries);
        });

    const auto toNextTarget = step.ToNextTarget();
    Point neighborRepulsion{};
    for(const auto& neighbor : neighborhood) {
        neighborRepulsion += NeighborRepulsion(currentState, toNextTarget, neighbor);
    }

    const auto desiredDirection = toNextTarget.Normalized();
    auto direction = (desiredDirection + neighborRepulsion).Normalized();
    if(direction == Point{}) {
        direction = currentState.orientation;
    }

    // update direction towards the newly calculated direction
    direction = UpdateDirection(currentState, toNextTarget, direction, step.dt());
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, GetSpacing(currentState, neighbor, direction));
    }

    const auto optimal_speed = OptimalSpeed(currentState, spacing, currentState.timeGap);
    // Wall sliding behavior
    direction = HandleWallAvoidance(
        direction,
        currentState.radius,
        boundaries,
        currentState.wallBufferDistance,
        _pushoutStrength);

    const auto velocity = direction * optimal_speed;
    auto& nextModel = std::get<State>(next);
    nextModel.orientation = direction;
    nextModel.velocity = velocity;
    return velocity * step.dt();
}

Point AnticipationVelocityModel::UpdateDirection(
    const State& currentState,
    Point toNextTarget,
    const Point& calculatedDirection,
    double dt) const
{
    const Point desiredDirection = toNextTarget.Normalized();
    const Point actualDirection = currentState.orientation;
    Point updatedDirection;

    if(desiredDirection.ScalarProduct(calculatedDirection) *
           desiredDirection.ScalarProduct(actualDirection) <
       0) {
        updatedDirection = calculatedDirection;
    } else {
        // Compute the rate of change of direction (Eq. 7)
        const Point directionDerivative =
            (calculatedDirection.Normalized() - actualDirection) / currentState.reactionTime;
        updatedDirection = actualDirection + directionDerivative * dt;
    }

    return updatedDirection.Normalized();
}

void AnticipationVelocityModel::CheckModelConstraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& currentState = std::get<State>(agent.state);
    const auto r = currentState.radius;
    constexpr double rMin = 0.;
    constexpr double rMax = 2.;
    validateConstraint(r, rMin, rMax, "radius", true);

    const auto strengthNeighborRepulsion = currentState.strengthNeighborRepulsion;
    constexpr double snMin = 0.;
    constexpr double snMax = 20.;
    validateConstraint(strengthNeighborRepulsion, snMin, snMax, "strengthNeighborRepulsion", false);

    const auto rangeNeighborRepulsion = currentState.rangeNeighborRepulsion;
    constexpr double rnMin = 0.;
    constexpr double rnMax = 5.;
    validateConstraint(rangeNeighborRepulsion, rnMin, rnMax, "rangeNeighborRepulsion", true);

    const auto buff = currentState.wallBufferDistance;
    constexpr double buffMin = 0.;
    constexpr double buffMax = 1.;
    validateConstraint(buff, buffMin, buffMax, "wallBufferDistance", false);

    const auto v0 = currentState.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = currentState.timeGap;
    constexpr double timeGapMin = 0.;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap", true);

    const auto anticipationTime = currentState.anticipationTime;
    constexpr double anticipationTimeMin = 0.0;
    constexpr double anticipationTimeMax = 5.0;
    validateConstraint(
        anticipationTime, anticipationTimeMin, anticipationTimeMax, "anticipationTime");

    const auto reactionTime = currentState.reactionTime;
    constexpr double reactionTimeMin = 0.0;
    constexpr double reactionTimeMax = 1.0;
    validateConstraint(reactionTime, reactionTimeMin, reactionTimeMax, "reactionTime", true);

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<State>(*neighbor.state);
        const auto contanctdDist = r + neighbor_model.radius;
        const auto distance = neighbor.RelativePosition.Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: "
                "distance {}",
                agent.Position(),
                agent.Position() + neighbor.RelativePosition,
                distance);
        }
    }

    if(!view.WallsInRange(r).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance "
            "<= {}",
            agent.Position(),
            r);
    }
}

double AnticipationVelocityModel::OptimalSpeed(
    const State& currentState,
    double spacing,
    double time_gap) const
{
    constexpr double creep_speed = 0.01;

    double speed = spacing / time_gap;

    if(std::abs(speed) < creep_speed) {
        // Random shuffle: forward, backward, or stop
        const auto r = gen() % 3;
        speed = (r == 0) ? creep_speed : (r == 1) ? -creep_speed : 0.0;
    }

    return std::min(std::max(speed, -creep_speed), currentState.v0);
}
double AnticipationVelocityModel::GetSpacing(
    const State& currentState,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& neighborState = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.RelativePosition;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto buffer = 0.02;
    const auto l = currentState.radius + neighborState.radius + buffer;
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
    const State& currentState,
    Point toNextTarget,
    const NeighborView& neighbor) const
{
    const auto& neighborState = std::get<State>(*neighbor.state);

    const auto distp12 = neighbor.RelativePosition;
    const auto [distance, ep12] = distp12.NormAndNormalized();
    const double adjustedDist = distance - (currentState.radius + neighborState.radius);

    // Pedestrian movement and desired directions
    const auto& e1 = currentState.orientation;
    const auto& d1 = toNextTarget.Normalized();
    const auto& e2 = neighborState.orientation;

    // Check perception range (Eq. 1)
    const auto inPerceptionRange = d1.ScalarProduct(ep12) >= 0 || e1.ScalarProduct(ep12) >= 0;
    if(!inPerceptionRange)
        return Point(0, 0);

    const double S_Gap = (currentState.velocity - neighborState.velocity).ScalarProduct(ep12) *
                         currentState.anticipationTime;
    double R_dist = adjustedDist - S_Gap;
    R_dist = std::max(R_dist, 0.0); // Clamp to zero if negative

    // Interaction strength (Eq. 3 & 4)
    constexpr double alignmentBase = 1.0;
    constexpr double alignmentWeight = 0.5;
    const double alignmentFactor = alignmentBase + alignmentWeight * (1.0 - d1.ScalarProduct(e2));
    const double interactionStrength = currentState.strengthNeighborRepulsion * alignmentFactor *
                                       std::exp(-R_dist / currentState.rangeNeighborRepulsion);
    const auto newep12 =
        distp12 + neighborState.velocity * neighborState.anticipationTime; // e_ij(t+ta)

    // Compute adjusted influence direction
    const auto influenceDirection = CalculateInfluenceDirection(d1, newep12);
    return influenceDirection * interactionStrength;
}

Point AnticipationVelocityModel::HandleWallAvoidance(
    const Point& direction,
    double agentRadius,
    const auto& boundaries,
    double wallBufferDistance,
    double pushoutStrength) const
{
    const double criticalWallDistance = wallBufferDistance + agentRadius;

    Point modifiedDirection = direction;
    for(const auto& wall : boundaries) {
        if(wall.distance > criticalWallDistance) {
            continue;
        }

        const auto dotProduct = modifiedDirection.ScalarProduct(wall.normal);

        if(dotProduct < 0) {
            // Direction points into wall - need to project it out
            // Remove the component pointing into the wall
            const auto projectedDirection = modifiedDirection - wall.normal * dotProduct;
            modifiedDirection = projectedDirection + wall.normal * pushoutStrength;
        }
    }

    // Renormalize to maintain speed
    const auto finalDirection = modifiedDirection.Normalized();

    return finalDirection;
}
