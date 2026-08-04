// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
constexpr double Eps = 1e-6; // Numeric lower bound to avoid division by zero in range terms.
constexpr double SideEps = 0.05; // Smooths left/right sign near centerline to reduce heading flips.
constexpr double SpacingBlendWeight =
    0.15; // Blends move-direction spacing with goal-direction spacing.
constexpr double TauTheta = 0.3; // Heading relaxation timescale [s] for temporal smoothing.
constexpr double MinReverseSpeed =
    -0.01; // Deterministic tiny reverse floor [m/s] to release local blockages.

double NeighborInfluence(
    const std::vector<NeighborView>& neighborhood,
    const Point& reference_direction,
    const CollisionFreeSpeedModelV3::State& currState)
{
    const auto range_x = std::max(Eps, currState.rangeNeighborRepulsion * currState.rangeXScale);
    const auto range_y = std::max(Eps, currState.rangeNeighborRepulsion * currState.rangeYScale);
    const auto theta_max =
        std::clamp(currState.strengthNeighborRepulsion, 0.0, currState.thetaMaxUpperBound);

    double best_influence = 0.0;
    double best_weight = 0.0;
    for(const auto& neighbor : neighborhood) {
        const auto relative = neighbor.RelativePosition;
        const auto x = reference_direction.ScalarProduct(relative);
        if(x <= 0.0) {
            continue;
        }

        const auto signed_lateral = reference_direction.CrossProduct(relative);
        const auto y = std::abs(signed_lateral);
        const auto longitudinal_weight = std::exp(-x / range_x);
        const auto lateral_weight = std::exp(-y / range_y);
        const auto weight = longitudinal_weight * lateral_weight;
        if(weight > best_weight) {
            best_weight = weight;
            best_influence = -weight * (signed_lateral / (std::abs(signed_lateral) + SideEps));
        }
    }

    return theta_max * std::tanh(best_influence);
}
} // namespace

OperationalModelType CollisionFreeSpeedModelV3::Type() const
{
    return OperationalModelType::COLLISION_FREE_SPEED_V3;
}

Point CollisionFreeSpeedModelV3::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& currState = std::get<State>(current);
    const auto& boundaries = step.WallsNearby();
    auto neighborhood =
        step.OtherAgentsInRange(_cutOffRadius, [&step, &boundaries](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, boundaries);
        });

    Point boundaryRepulsion{};
    for(const auto& wall : boundaries) {
        boundaryRepulsion += BoundaryRepulsion(currState, wall);
    }

    const auto desired_direction = step.ToNextTarget().Normalized();
    auto reference_direction = (desired_direction + boundaryRepulsion).Normalized();
    if(reference_direction == Point{}) {
        reference_direction = currState.orientation;
    }

    const auto heading_target = NeighborInfluence(neighborhood, reference_direction, currState);
    const auto alpha = std::clamp(step.dt() / TauTheta, 0.0, 1.0);
    const auto heading_angle =
        currState.headingAngle + alpha * (heading_target - currState.headingAngle);
    auto direction =
        reference_direction.Rotate(std::cos(heading_angle), std::sin(heading_angle)).Normalized();
    if(direction == Point{}) {
        direction = reference_direction;
    }

    const auto closest_spacing_towards = [&](Point towards) {
        auto spacing = std::numeric_limits<double>::max();
        for(const auto& neighbor : neighborhood) {
            spacing = std::min(spacing, GetSpacing(currState, neighbor, towards));
        }
        return spacing;
    };

    const auto spacing_move = closest_spacing_towards(direction);
    const auto goal_direction =
        (desired_direction == Point{}) ? reference_direction : desired_direction;
    const auto spacing_goal = closest_spacing_towards(goal_direction);

    const auto spacing =
        spacing_move * (1.0 - SpacingBlendWeight) + spacing_goal * SpacingBlendWeight;

    const auto optimal_speed = OptimalSpeed(currState, spacing, currState.timeGap);
    const auto velocity = direction * optimal_speed;
    auto& nextModel = std::get<State>(next);
    nextModel.orientation = direction;
    nextModel.headingAngle = heading_angle;
    return velocity * step.dt();
}

void CollisionFreeSpeedModelV3::CheckModelConstraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& currState = std::get<State>(agent.state);

    validateConstraint(currState.radius, 0.0, 2.0, "radius", true);
    validateConstraint(currState.v0, 0.0, 10.0, "v0");
    validateConstraint(currState.timeGap, 0.1, 10.0, "timeGap");

    validateConstraint(
        currState.strengthNeighborRepulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthNeighborRepulsion");
    validateConstraint(
        currState.rangeNeighborRepulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeNeighborRepulsion");
    validateConstraint(
        currState.strengthGeometryRepulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthGeometryRepulsion");
    validateConstraint(
        currState.rangeGeometryRepulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeGeometryRepulsion");

    validateConstraint(
        currState.rangeXScale, 0.01, std::numeric_limits<double>::max(), "rangeXScale");
    validateConstraint(
        currState.rangeYScale, 0.01, std::numeric_limits<double>::max(), "rangeYScale");
    validateConstraint(currState.thetaMaxUpperBound, 0.0, std::acos(-1.0), "thetaMaxUpperBound");
    validateConstraint(currState.agentBuffer, 0.0, 100.0, "agentBuffer");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighState = std::get<State>(*neighbor.state);
        const auto contactDist = currState.radius + neighState.radius;
        const auto distance = neighbor.RelativePosition.Norm();
        if(contactDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.Position(),
                agent.Position() + neighbor.RelativePosition,
                distance);
        }
    }

    if(!view.WallsInRange(currState.radius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            agent.Position(),
            currState.radius);
    }
}

double CollisionFreeSpeedModelV3::OptimalSpeed(
    const State& currState,
    double spacing,
    double time_gap) const
{
    const auto effective_spacing = spacing - currState.agentBuffer;
    return std::min(std::max(effective_spacing / time_gap, MinReverseSpeed), currState.v0);
}

double CollisionFreeSpeedModelV3::GetSpacing(
    const State& currState,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.RelativePosition;
    if(direction.ScalarProduct(distp12) < 0.0) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = currState.radius + other.radius;
    const auto inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }

    return distp12.Norm() - l;
}

Point CollisionFreeSpeedModelV3::BoundaryRepulsion(const State& currState, const WallView& boundary)
    const
{
    const auto l = currState.radius;
    const auto R_iw = -currState.strengthGeometryRepulsion *
                      std::exp((l - boundary.distance) / currState.rangeGeometryRepulsion);
    return -boundary.normal * R_iw; // The repulsion points away from the agent
}
