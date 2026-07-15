// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3.hpp"

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "TacticalModelState.hpp"

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
    const CollisionFreeSpeedModelV3::StateContainer& neighborhood,
    const Point& pos,
    const Point& reference_direction,
    const CollisionFreeSpeedModelV3::State& state)
{
    const auto range_x = std::max(Eps, state.rangeNeighborRepulsion * state.rangeXScale);
    const auto range_y = std::max(Eps, state.rangeNeighborRepulsion * state.rangeYScale);
    const auto theta_max =
        std::clamp(state.strengthNeighborRepulsion, 0.0, state.thetaMaxUpperBound);

    double best_influence = 0.0;
    double best_weight = 0.0;
    for(const auto& neighbor : neighborhood) {
        const auto relative = std::get<CollisionFreeSpeedModelV3::State>(neighbor).position - pos;
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

void CollisionFreeSpeedModelV3::ComputeNextState(
    double dT,
    const GenericState& current,
    GenericState& next,
    const TacticalModelState& tactical,
    const CollisionGeometry& geometry,
    const StateContainer& neighborStates) const
{
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(Pos(current));
    const auto& state = std::get<State>(current);

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(state, element);
        });

    const auto desired_direction = (tactical.destination - Pos(current)).Normalized();
    auto reference_direction = (desired_direction + boundaryRepulsion).Normalized();
    if(reference_direction == Point{}) {
        reference_direction = state.orientation;
    }

    const auto heading_target =
        NeighborInfluence(neighborStates, Pos(current), reference_direction, state);
    const auto alpha = std::clamp(dT / TauTheta, 0.0, 1.0);
    const auto heading_angle = state.headingAngle + alpha * (heading_target - state.headingAngle);
    auto direction =
        reference_direction.Rotate(std::cos(heading_angle), std::sin(heading_angle)).Normalized();
    if(direction == Point{}) {
        direction = reference_direction;
    }

    const auto spacing_move = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        std::numeric_limits<double>::max(),
        [&state, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(state, std::get<State>(neighbor), direction));
        });

    const auto goal_direction =
        (desired_direction == Point{}) ? reference_direction : desired_direction;
    const auto spacing_goal = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        std::numeric_limits<double>::max(),
        [&state, &goal_direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(state, std::get<State>(neighbor), goal_direction));
        });

    const auto spacing =
        spacing_move * (1.0 - SpacingBlendWeight) + spacing_goal * SpacingBlendWeight;

    const auto optimal_speed = OptimalSpeed(state, spacing, state.timeGap);
    const auto velocity = direction * optimal_speed;
    auto& nextState = std::get<State>(next);
    nextState.position = Pos(current) + velocity * dT;
    nextState.orientation = direction;
    nextState.headingAngle = heading_angle;
}

void CollisionFreeSpeedModelV3::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& state = std::get<State>(agent.state);

    validateConstraint(state.radius, 0.0, 2.0, "radius", true);
    validateConstraint(state.v0, 0.0, 10.0, "v0");
    validateConstraint(state.timeGap, 0.1, 10.0, "timeGap");

    validateConstraint(
        state.strengthNeighborRepulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthNeighborRepulsion");
    validateConstraint(
        state.rangeNeighborRepulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeNeighborRepulsion");
    validateConstraint(
        state.strengthGeometryRepulsion,
        0.0,
        std::numeric_limits<double>::max(),
        "strengthGeometryRepulsion");
    validateConstraint(
        state.rangeGeometryRepulsion,
        0.01,
        std::numeric_limits<double>::max(),
        "rangeGeometryRepulsion");

    validateConstraint(state.rangeXScale, 0.01, std::numeric_limits<double>::max(), "rangeXScale");
    validateConstraint(state.rangeYScale, 0.01, std::numeric_limits<double>::max(), "rangeYScale");
    validateConstraint(state.thetaMaxUpperBound, 0.0, std::acos(-1.0), "thetaMaxUpperBound");
    validateConstraint(state.agentBuffer, 0.0, 100.0, "agentBuffer");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(state.position, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
            continue;
        }
        const auto& neighbor_state = std::get<State>(neighbor.state);
        const auto contactDist = state.radius + neighbor_state.radius;
        const auto distance = (state.position - neighbor_state.position).Norm();
        if(contactDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                state.position,
                neighbor_state.position,
                distance);
        }
    }

    const auto lineSegments = geometry.LineSegmentsInDistanceTo(state.radius, state.position);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            state.position,
            state.radius);
    }
}

double
CollisionFreeSpeedModelV3::OptimalSpeed(const State& state, double spacing, double time_gap) const
{
    const auto effective_spacing = spacing - state.agentBuffer;
    return std::min(std::max(effective_spacing / time_gap, MinReverseSpeed), state.v0);
}

double CollisionFreeSpeedModelV3::GetSpacing(
    const State& state1,
    const State& state2,
    const Point& direction) const
{
    const auto distp12 = state2.position - state1.position;
    if(direction.ScalarProduct(distp12) < 0.0) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = state1.radius + state2.radius;
    const auto inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }

    return distp12.Norm() - l;
}

Point CollisionFreeSpeedModelV3::BoundaryRepulsion(
    const State& state,
    const LineSegment& boundary_segment) const
{
    const auto pt = boundary_segment.ShortestPoint(state.position);
    const auto dist_vec = pt - state.position;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto l = state.radius;
    const auto R_iw =
        -state.strengthGeometryRepulsion * std::exp((l - dist) / state.rangeGeometryRepulsion);
    return e_iw * R_iw;
}
