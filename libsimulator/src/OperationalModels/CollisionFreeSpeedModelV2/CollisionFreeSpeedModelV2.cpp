// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV2.hpp"

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

OperationalModelType CollisionFreeSpeedModelV2::Type() const
{
    return OperationalModelType::COLLISION_FREE_SPEED_V2;
}

Point CollisionFreeSpeedModelV2::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& model = std::get<State>(current);
    const auto& boundaries = step.WallsNearby();
    auto neighborhood =
        step.OtherAgentsInRange(_cutOffRadius, [&step, &boundaries](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, boundaries);
        });

    Point neighborRepulsion{};
    for(const auto& neighbor : neighborhood) {
        neighborRepulsion += NeighborRepulsion(model, neighbor);
    }

    Point boundaryRepulsion{};
    for(const auto& wall : boundaries) {
        boundaryRepulsion += BoundaryRepulsion(model, wall);
    }

    const auto desired_direction = step.ToNextTarget().Normalized();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    if(direction == Point{}) {
        direction = model.orientation;
    }
    auto spacing = std::numeric_limits<double>::max();
    for(const auto& neighbor : neighborhood) {
        spacing = std::min(spacing, GetSpacing(model, neighbor, direction));
    }

    const auto optimal_speed = OptimalSpeed(model, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    std::get<State>(next).orientation = direction;
    return velocity * step.dt();
}

void CollisionFreeSpeedModelV2::CheckModelConstraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& model = std::get<State>(agent.model);

    const auto r = model.radius;
    constexpr double rMin = 0.;
    constexpr double rMax = 2.;
    validateConstraint(r, rMin, rMax, "radius", true);

    const auto v0 = model.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = model.timeGap;
    constexpr double timeGapMin = 0.1;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<State>(*neighbor.state);
        const auto contanctdDist = r + neighbor_model.radius;
        const auto distance = neighbor.RelativePosition.Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.position,
                agent.position + neighbor.RelativePosition,
                distance);
        }
    }

    if(!view.WallsInRange(r).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            agent.position,
            r);
    }
}

double
CollisionFreeSpeedModelV2::OptimalSpeed(const State& self, double spacing, double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), self.v0);
}

double CollisionFreeSpeedModelV2::GetSpacing(
    const State& self,
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
    const auto l = self.radius + other.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}
Point CollisionFreeSpeedModelV2::NeighborRepulsion(const State& self, const NeighborView& neighbor)
    const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto [distance, direction] = neighbor.RelativePosition.NormAndNormalized();
    const auto l = self.radius + other.radius;
    return direction *
           -(self.strengthNeighborRepulsion * exp((l - distance) / self.rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModelV2::BoundaryRepulsion(const State& self, const WallView& boundary)
    const
{
    const auto l = self.radius;
    const auto R_iw = -self.strengthGeometryRepulsion *
                      exp((l - boundary.distance) / self.rangeGeometryRepulsion);
    return -boundary.normal * R_iw; // The repulsion points away from the agent
}
