// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModel.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>

CollisionFreeSpeedModel::CollisionFreeSpeedModel(
    double strengthNeighborRepulsion_,
    double rangeNeighborRepulsion_,
    double strengthGeometryRepulsion_,
    double rangeGeometryRepulsion_)
    : strengthNeighborRepulsion(strengthNeighborRepulsion_)
    , rangeNeighborRepulsion(rangeNeighborRepulsion_)
    , strengthGeometryRepulsion(strengthGeometryRepulsion_)
    , rangeGeometryRepulsion(rangeGeometryRepulsion_)
{
}

OperationalModelType CollisionFreeSpeedModel::Type() const
{
    return OperationalModelType::COLLISION_FREE_SPEED;
}

Point CollisionFreeSpeedModel::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& currentState = std::get<State>(current);
    auto _w = step.WallsNearby();
    const std::vector<WallView> boundaries(_w.begin(), _w.end());
    const auto neighborhood =
        step.OtherAgentsInRange(_cutOffRadius, [&step, &boundaries](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, boundaries);
        });

    Point neighborRepulsion{};
    for(const auto& neighbor : neighborhood) {
        neighborRepulsion += NeighborRepulsion(currentState, neighbor);
    }

    Point boundaryRepulsion{};
    for(const auto& wall : boundaries) {
        boundaryRepulsion += BoundaryRepulsion(currentState, wall);
    }

    const auto desired_direction = step.orientation_to_next_target();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    if(direction == Point{}) {
        direction = currentState.orientation;
    }
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, GetSpacing(currentState, neighbor, direction));
    }

    const auto optimal_speed = OptimalSpeed(currentState, spacing, currentState.timeGap);
    const auto velocity = direction * optimal_speed;
    std::get<State>(next).orientation = direction;
    return velocity * step.dt();
}

void CollisionFreeSpeedModel::CheckModelConstraint(const GenericAgent& agent, const AgentView& view)
    const
{
    const auto& currentState = std::get<State>(agent.state);

    const auto r = currentState.radius;
    constexpr double rMin = 0.;
    constexpr double rMax = 2.;
    validateConstraint(r, rMin, rMax, "radius", true);

    const auto v0 = currentState.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = currentState.timeGap;
    constexpr double timeGapMin = 0.1;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighborState = std::get<State>(*neighbor.state);
        const auto contanctdDist = r + neighborState.radius;
        const auto distance = neighbor.RelativePosition.Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: distance {}",
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

double CollisionFreeSpeedModel::OptimalSpeed(
    const State& currentState,
    double spacing,
    double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), currentState.v0);
}

double CollisionFreeSpeedModel::GetSpacing(
    const State& currentState,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.RelativePosition;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = currentState.radius + other.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}
Point CollisionFreeSpeedModel::NeighborRepulsion(
    const State& currentState,
    const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto [distance, direction] = neighbor.RelativePosition.NormAndNormalized();
    const auto l = currentState.radius + other.radius;
    return direction *
           -(this->strengthNeighborRepulsion * exp((l - distance) / this->rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModel::BoundaryRepulsion(
    const State& currentState,
    const WallView& boundary) const
{
    const auto l = currentState.radius;
    const auto R_iw =
        -strengthGeometryRepulsion * exp((l - boundary.distance) / rangeGeometryRepulsion);
    return -boundary.normal * R_iw; // The repulsion points away from the agent
}
