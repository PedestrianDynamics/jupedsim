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
    const auto& currState = std::get<State>(current);
    const auto& boundaries = step.WallsNearby();
    const auto neighborhood =
        step.OtherAgentsInRange(_cutOffRadius, [&step, &boundaries](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, boundaries);
        });

    Point neighborRepulsion{};
    for(const auto& neighbor : neighborhood) {
        neighborRepulsion += NeighborRepulsion(currState, neighbor);
    }

    Point boundaryRepulsion{};
    for(const auto& wall : boundaries) {
        boundaryRepulsion += BoundaryRepulsion(currState, wall);
    }

    const auto desired_direction = step.ToNextTarget().Normalized();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    if(direction == Point{}) {
        direction = currState.orientation;
    }
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, GetSpacing(currState, neighbor, direction));
    }

    const auto optimal_speed = OptimalSpeed(currState, spacing, currState.timeGap);
    const auto velocity = direction * optimal_speed;
    std::get<State>(next).orientation = direction;
    return velocity * step.dt();
}

void CollisionFreeSpeedModel::CheckModelConstraint(const GenericAgent& agent, const AgentView& view)
    const
{
    const auto& currState = std::get<State>(agent.state);

    const auto r = currState.radius;
    constexpr double rMin = 0.;
    constexpr double rMax = 2.;
    validateConstraint(r, rMin, rMax, "radius", true);

    const auto v0 = currState.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = currState.timeGap;
    constexpr double timeGapMin = 0.1;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighState = std::get<State>(*neighbor.state);
        const auto contanctdDist = r + neighState.radius;
        const auto distance = neighbor.RelativePosition.Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.Position(),
                agent.Position() + neighbor.RelativePosition,
                distance);
        }
    }

    if(!view.WallsInRange(r).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            agent.Position(),
            r);
    }
}

double
CollisionFreeSpeedModel::OptimalSpeed(const State& currState, double spacing, double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), currState.v0);
}

double CollisionFreeSpeedModel::GetSpacing(
    const State& currState,
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
    const auto l = currState.radius + other.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}
Point CollisionFreeSpeedModel::NeighborRepulsion(
    const State& currState,
    const NeighborView& neighbor) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto [distance, direction] = neighbor.RelativePosition.NormAndNormalized();
    const auto l = currState.radius + other.radius;
    return direction *
           -(this->strengthNeighborRepulsion * exp((l - distance) / this->rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModel::BoundaryRepulsion(const State& currState, const WallView& boundary)
    const
{
    const auto l = currState.radius;
    const auto R_iw =
        -strengthGeometryRepulsion * exp((l - boundary.distance) / rangeGeometryRepulsion);
    return -boundary.normal * R_iw; // The repulsion points away from the agent
}
