// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModel.hpp"

#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "NeighborQuery.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <variant>
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
    Point destination,
    const CollisionGeometry& geometry,
    const NeighborQuery& neighborQuery) const
{
    const auto& model = std::get<State>(current);
    const auto neighborhood = neighborQuery(model.position, _cutOffRadius);
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(model.position);

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        Point{},
        [&model, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(model, std::get<State>(neighbor));
        });

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &model](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(model, element);
        });

    const auto desired_direction = (destination - model.position).Normalized();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    if(direction == Point{}) {
        direction = model.orientation;
    }
    const auto spacing = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&model, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(model, std::get<State>(neighbor), direction));
        });

    const auto optimal_speed = OptimalSpeed(model, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    auto& nextModel = std::get<State>(next);
    nextModel.position = model.position + velocity * dT;
    nextModel.orientation = direction;
};

void CollisionFreeSpeedModel::CheckModelConstraint(
    const OperationalModelState& state,
    const NeighborQuery& neighborQuery,
    const CollisionGeometry& geometry) const
{
    const auto& model = std::get<State>(state);

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

    const auto neighbors = neighborQuery(model.position, 2);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<State>(neighbor);
        const auto contanctdDist = r + neighbor_model.radius;
        const auto distance = (model.position - neighbor_model.position).Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                model.position,
                neighbor_model.position,
                distance);
        }
    }

    const auto lineSegments = geometry.LineSegmentsInDistanceTo(r, model.position);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            model.position,
            r);
    }
}

double
CollisionFreeSpeedModel::OptimalSpeed(const State& model, double spacing, double time_gap) const
{
    return std::min(std::max(spacing / time_gap, 0.0), model.v0);
}

double CollisionFreeSpeedModel::GetSpacing(
    const State& model1,
    const State& model2,
    const Point& direction) const
{
    const auto distp12 = model2.position - model1.position;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = model1.radius + model2.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}
Point CollisionFreeSpeedModel::NeighborRepulsion(const State& model1, const State& model2) const
{
    const auto distp12 = model2.position - model1.position;
    const auto [distance, direction] = distp12.NormAndNormalized();
    const auto l = model1.radius + model2.radius;
    return direction *
           -(this->strengthNeighborRepulsion * exp((l - distance) / this->rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModel::BoundaryRepulsion(
    const State& model,
    const LineSegment& boundary_segment) const
{
    const auto pt = boundary_segment.ShortestPoint(model.position);
    const auto dist_vec = pt - model.position;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto l = model.radius;
    const auto R_iw =
        -this->strengthGeometryRepulsion * exp((l - dist) / this->rangeGeometryRepulsion);
    return e_iw * R_iw;
}
