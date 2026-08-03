// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
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
    const CollisionFreeSpeedModelV3::State& model)
{
    const auto range_x = std::max(Eps, model.rangeNeighborRepulsion * model.rangeXScale);
    const auto range_y = std::max(Eps, model.rangeNeighborRepulsion * model.rangeYScale);
    const auto theta_max =
        std::clamp(model.strengthNeighborRepulsion, 0.0, model.thetaMaxUpperBound);

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
    const auto& model = std::get<State>(current);
    const auto& boundaries = step.WallsNearby();
    auto neighborhood = step.OtherAgentsInRange(
        _cutOffRadius, [&step, &boundaries](const NeighborView& n) {
            return step.NoGeometryBetween(n.RelativePosition, boundaries);
        });

    const auto boundaryRepulsion = std::accumulate(
        std::begin(boundaries),
        std::end(boundaries),
        Point(0, 0),
        [this, &model](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(model, element);
        });

    const auto desired_direction = step.ToNextTarget().Normalized();
    auto reference_direction = (desired_direction + boundaryRepulsion).Normalized();
    if(reference_direction == Point{}) {
        reference_direction = model.orientation;
    }

    const auto heading_target = NeighborInfluence(neighborhood, reference_direction, model);
    const auto alpha = std::clamp(step.dt() / TauTheta, 0.0, 1.0);
    const auto heading_angle = model.headingAngle + alpha * (heading_target - model.headingAngle);
    auto direction =
        reference_direction.Rotate(std::cos(heading_angle), std::sin(heading_angle)).Normalized();
    if(direction == Point{}) {
        direction = reference_direction;
    }

    const auto spacing_move = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&model, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(model, neighbor, direction));
        });

    const auto goal_direction =
        (desired_direction == Point{}) ? reference_direction : desired_direction;
    const auto spacing_goal = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&model, &goal_direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(model, neighbor, goal_direction));
        });

    const auto spacing =
        spacing_move * (1.0 - SpacingBlendWeight) + spacing_goal * SpacingBlendWeight;

    const auto optimal_speed = OptimalSpeed(model, spacing, model.timeGap);
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
    const auto& model = std::get<State>(agent.model);

    validateConstraint(model.radius, 0.0, 2.0, "radius", true);
    validateConstraint(model.v0, 0.0, 10.0, "v0");
    validateConstraint(model.timeGap, 0.1, 10.0, "timeGap");

    validateConstraint(
        model.strengthNeighborRepulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthNeighborRepulsion");
    validateConstraint(
        model.rangeNeighborRepulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeNeighborRepulsion");
    validateConstraint(
        model.strengthGeometryRepulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthGeometryRepulsion");
    validateConstraint(
        model.rangeGeometryRepulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeGeometryRepulsion");

    validateConstraint(model.rangeXScale, 0.01, std::numeric_limits<double>::max(), "rangeXScale");
    validateConstraint(model.rangeYScale, 0.01, std::numeric_limits<double>::max(), "rangeYScale");
    validateConstraint(model.thetaMaxUpperBound, 0.0, std::acos(-1.0), "thetaMaxUpperBound");
    validateConstraint(model.agentBuffer, 0.0, 100.0, "agentBuffer");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<State>(*neighbor.state);
        const auto contactDist = model.radius + neighbor_model.radius;
        const auto distance = neighbor.RelativePosition.Norm();
        if(contactDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.position,
                agent.position + neighbor.RelativePosition,
                distance);
        }
    }

    const auto lineSegments = view.WallsInRange(model.radius);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            agent.position,
            model.radius);
    }
}

double
CollisionFreeSpeedModelV3::OptimalSpeed(const State& self, double spacing, double time_gap) const
{
    const auto effective_spacing = spacing - self.agentBuffer;
    return std::min(std::max(effective_spacing / time_gap, MinReverseSpeed), self.v0);
}

double CollisionFreeSpeedModelV3::GetSpacing(
    const State& self,
    const NeighborView& neighbor,
    const Point& direction) const
{
    const auto& other = std::get<State>(*neighbor.state);
    const auto distp12 = neighbor.RelativePosition;
    if(direction.ScalarProduct(distp12) < 0.0) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = self.radius + other.radius;
    const auto inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }

    return distp12.Norm() - l;
}

Point CollisionFreeSpeedModelV3::BoundaryRepulsion(
    const State& self,
    const LineSegment& boundary_segment) const
{
    const auto dist_vec = boundary_segment.ShortestPoint(Point{});
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto l = self.radius;
    const auto R_iw =
        -self.strengthGeometryRepulsion * std::exp((l - dist) / self.rangeGeometryRepulsion);
    return e_iw * R_iw;
}
