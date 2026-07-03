// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModel.hpp"

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
    const GenericAgent& current,
    GenericAgent& next,
    const CollisionGeometry& geometry,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const
{
    auto neighborhood = neighborhoodSearch.GetNeighboringAgents(current.pos, _cutOffRadius);
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(current.pos);

    // Remove any agent from the neighborhood that is obstructed by geometry and the current
    // agent
    neighborhood.erase(
        std::remove_if(
            std::begin(neighborhood),
            std::end(neighborhood),
            [&current, &boundary](const auto& neighbor) {
                if(current.id == neighbor.id) {
                    return true;
                }
                const auto agent_to_neighbor = LineSegment(current.pos, neighbor.pos);
                if(std::find_if(
                       boundary.cbegin(),
                       boundary.cend(),
                       [&agent_to_neighbor](const auto& boundary_segment) {
                           return intersects(agent_to_neighbor, boundary_segment);
                       }) != boundary.end()) {
                    return true;
                }

                return false;
            }),
        std::end(neighborhood));

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        Point{},
        [&current, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(current, neighbor);
        });

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &current](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(current, element);
        });

    const auto desired_direction = (current.destination - current.pos).Normalized();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    const auto& model = std::get<State>(current.model);
    if(direction == Point{}) {
        direction = model.orientation;
    }
    const auto spacing = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&current, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(current, neighbor, direction));
        });

    const auto optimal_speed = OptimalSpeed(current, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    next.pos = current.pos + velocity * dT;
    std::get<State>(next.model).orientation = direction;
};

void CollisionFreeSpeedModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
    const CollisionGeometry& geometry) const
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

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
            continue;
        }
        const auto& neighbor_model = std::get<State>(neighbor.model);
        const auto contanctdDist = r + neighbor_model.radius;
        const auto distance = (agent.pos - neighbor.pos).Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.pos,
                neighbor.pos,
                distance);
        }
    }

    const auto lineSegments = geometry.LineSegmentsInDistanceTo(r, agent.pos);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            agent.pos,
            r);
    }
}

double CollisionFreeSpeedModel::OptimalSpeed(
    const GenericAgent& ped,
    double spacing,
    double time_gap) const
{
    const auto& model = std::get<State>(ped.model);
    return std::min(std::max(spacing / time_gap, 0.0), model.v0);
}

double CollisionFreeSpeedModel::GetSpacing(
    const GenericAgent& ped1,
    const GenericAgent& ped2,
    const Point& direction) const
{
    const auto& model1 = std::get<State>(ped1.model);
    const auto& model2 = std::get<State>(ped2.model);
    const auto distp12 = ped2.pos - ped1.pos;
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
Point CollisionFreeSpeedModel::NeighborRepulsion(const GenericAgent& ped1, const GenericAgent& ped2)
    const
{
    const auto distp12 = ped2.pos - ped1.pos;
    const auto [distance, direction] = distp12.NormAndNormalized();
    const auto& model1 = std::get<State>(ped1.model);
    const auto& model2 = std::get<State>(ped2.model);
    const auto l = model1.radius + model2.radius;
    return direction *
           -(this->strengthNeighborRepulsion * exp((l - distance) / this->rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModel::BoundaryRepulsion(
    const GenericAgent& ped,
    const LineSegment& boundary_segment) const
{
    const auto pt = boundary_segment.ShortestPoint(ped.pos);
    const auto dist_vec = pt - ped.pos;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto& model = std::get<State>(ped.model);
    const auto l = model.radius;
    const auto R_iw =
        -this->strengthGeometryRepulsion * exp((l - dist) / this->rangeGeometryRepulsion);
    return e_iw * R_iw;
}
