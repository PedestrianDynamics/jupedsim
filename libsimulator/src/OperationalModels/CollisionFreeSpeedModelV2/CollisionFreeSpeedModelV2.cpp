// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV2.hpp"

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <vector>

OperationalModelType CollisionFreeSpeedModelV2::Type() const
{
    return OperationalModelType::COLLISION_FREE_SPEED_V2;
}

void CollisionFreeSpeedModelV2::ComputeNextState(
    double dT,
    const GenericState& current,
    GenericState& next,
    const TacticalModelState& tactical,
    const CollisionGeometry& geometry,
    const StateContainer& neighborStates) const
{
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(Pos(current));
    const auto& state = std::get<State>(current);

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        Point{},
        [&state, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(state, std::get<State>(neighbor));
        });

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(state, element);
        });

    const auto desired_direction = (tactical.destination - Pos(current)).Normalized();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    if(direction == Point{}) {
        direction = state.orientation;
    }
    const auto spacing = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        std::numeric_limits<double>::max(),
        [&state, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(state, std::get<State>(neighbor), direction));
        });

    const auto optimal_speed = OptimalSpeed(state, spacing, state.timeGap);
    const auto velocity = direction * optimal_speed;
    auto& nextState = std::get<State>(next);
    nextState.position = Pos(current) + velocity * dT;
    nextState.orientation = direction;
}

void CollisionFreeSpeedModelV2::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& state = std::get<State>(agent.state);

    const auto r = state.radius;
    constexpr double rMin = 0.;
    constexpr double rMax = 2.;
    validateConstraint(r, rMin, rMax, "radius", true);

    const auto v0 = state.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = state.timeGap;
    constexpr double timeGapMin = 0.1;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(state.position, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
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
CollisionFreeSpeedModelV2::OptimalSpeed(const State& state, double spacing, double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), state.v0);
}

double CollisionFreeSpeedModelV2::GetSpacing(
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
    const auto l = state1.radius + state2.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}

Point CollisionFreeSpeedModelV2::NeighborRepulsion(const State& state1, const State& state2) const
{
    const auto distp12 = state2.position - state1.position;
    const auto [distance, direction] = distp12.NormAndNormalized();
    const auto l = state1.radius + state2.radius;
    return direction * -(state1.strengthNeighborRepulsion *
                         exp((l - distance) / state1.rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModelV2::BoundaryRepulsion(
    const State& state,
    const LineSegment& boundary_segment) const
{
    const auto pt = boundary_segment.ShortestPoint(state.position);
    const auto dist_vec = pt - state.position;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto l = state.radius;
    const auto R_iw =
        -state.strengthGeometryRepulsion * exp((l - dist) / state.rangeGeometryRepulsion);
    return e_iw * R_iw;
}
