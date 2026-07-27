// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModel.hpp"

#include "EnvironmentQuery.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
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

void CollisionFreeSpeedModel::ComputeNextState(
    double dT,
    const OperationalModelState& current,
    OperationalModelState& next,
    const Point& destination,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(current);
    const auto& boundary = envQuery.LineSegmentsInRange(state.position);
    const auto neighborStates = envQuery.OtherAgentsInRange(
        current, _cutOffRadius, [&envQuery, from = state.position](const Point& to) {
            return envQuery.NoGeometryBetween(from, to);
        });

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborStates),
        std::end(neighborStates),
        Point{},
        [&state, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(state, std::get<State>(neighbor));
        });

    const auto boundaryRepulsion = std::accumulate(
        std::begin(boundary),
        std::end(boundary),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(state, element);
        });

    const auto desired_direction = (destination - state.position).Normalized();
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
    nextState.position = state.position + velocity * dT;
    nextState.orientation = direction;
}

void CollisionFreeSpeedModel::CheckModelConstraint(
    const OperationalModelState& generic_state,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(generic_state);

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

    const auto neighbors = envQuery.OtherAgentsInRange(generic_state, 2.0);
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
CollisionFreeSpeedModel::OptimalSpeed(const State& state, double spacing, double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), state.v0);
}

double
CollisionFreeSpeedModel::GetSpacing(const State& s1, const State& s2, const Point& direction) const
{
    const auto distp12 = s2.position - s1.position;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = s1.radius + s2.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}

Point CollisionFreeSpeedModel::NeighborRepulsion(const State& s1, const State& s2) const
{
    const auto distp12 = s2.position - s1.position;
    const auto [distance, direction] = distp12.NormAndNormalized();
    const auto l = s1.radius + s2.radius;
    return direction *
           -(this->strengthNeighborRepulsion * exp((l - distance) / this->rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModel::BoundaryRepulsion(
    const State& state,
    const LineSegment& boundary_segment) const
{
    const auto pt = boundary_segment.ShortestPoint(state.position);
    const auto dist_vec = pt - state.position;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto l = state.radius;
    const auto R_iw =
        -this->strengthGeometryRepulsion * exp((l - dist) / this->rangeGeometryRepulsion);
    return e_iw * R_iw;
}
