// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3.hpp"

#include "CollisionFreeSpeedModelV3Data.hpp"
#include "CollisionFreeSpeedModelV3Update.hpp"
#include "CollisionGeometry.hpp"
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
#include <memory>
#include <numeric>
#include <vector>

namespace
{
constexpr double Eps = 1e-6; // Numeric lower bound to avoid division by zero in range terms.
constexpr double SideEps = 0.05; // Smooths left/right sign near centerline to reduce heading flips.
constexpr double SpacingBlendWeight = 0.15; // Blends move-direction spacing with goal-direction spacing.
constexpr double TauTheta = 0.4; // Heading relaxation timescale [s] for temporal smoothing.
constexpr double MinReverseSpeed = -0.01; // Deterministic tiny reverse floor [m/s] to release local blockages.
}

OperationalModelType CollisionFreeSpeedModelV3::Type() const
{
    return OperationalModelType::COLLISION_FREE_SPEED_V3;
}

OperationalModelUpdate CollisionFreeSpeedModelV3::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, _cutOffRadius);
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    neighborhood.erase(
        std::remove_if(
            std::begin(neighborhood),
            std::end(neighborhood),
            [&ped, &boundary](const auto& neighbor) {
                if(ped.id == neighbor.id) {
                    return true;
                }
                const auto agent_to_neighbor = LineSegment(ped.pos, neighbor.pos);
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

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(ped, element);
        });

    const auto& model = std::get<CollisionFreeSpeedModelV3Data>(ped.model);
    const auto desired_direction = (ped.destination - ped.pos).Normalized();
    auto reference_direction = (desired_direction + boundaryRepulsion).Normalized();
    if(reference_direction == Point{}) {
        reference_direction = ped.orientation;
    }

    const auto range_x = std::max(Eps, model.rangeNeighborRepulsion * model.rangeXScale);
    const auto range_y = std::max(Eps, model.rangeNeighborRepulsion * model.rangeYScale);
    const auto theta_max =
        std::clamp(model.strengthNeighborRepulsion, 0.0, model.thetaMaxUpperBound);

    double best_influence = 0.0;
    double best_weight = 0.0;
    for(const auto& neighbor : neighborhood) {
        const auto relative = neighbor.pos - ped.pos;
        const auto x = reference_direction.ScalarProduct(relative);
        if(x <= 0.0) {
            continue;
        }

        const auto signed_lateral = reference_direction.CrossProduct(relative);
        const auto y = std::abs(signed_lateral);
        const auto longitudinal_weight = std::exp(-x / range_x);
        const auto lateral_weight = std::exp(-std::pow(y / range_y, 2.0));
        const auto weight = longitudinal_weight * lateral_weight;
        if(weight > best_weight) {
            best_weight = weight;
            best_influence = -weight * (signed_lateral / (std::abs(signed_lateral) + SideEps));
        }
    }

    const auto heading_target = theta_max * std::tanh(best_influence);
    const auto alpha = std::clamp(dT / TauTheta, 0.0, 1.0);
    const auto heading_angle =
        model.headingAngle + alpha * (heading_target - model.headingAngle);
    auto direction = reference_direction.Rotate(std::cos(heading_angle), std::sin(heading_angle))
                         .Normalized();
    if(direction == Point{}) {
        direction = reference_direction;
    }

    const auto spacing_move = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&ped, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(ped, neighbor, direction));
        });

    const auto goal_direction = (desired_direction == Point{}) ? reference_direction : desired_direction;
    const auto spacing_goal = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&ped, &goal_direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(ped, neighbor, goal_direction));
        });

    const auto spacing =
        spacing_move * (1.0 - SpacingBlendWeight) + spacing_goal * SpacingBlendWeight;

    const auto optimal_speed = OptimalSpeed(ped, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    return CollisionFreeSpeedModelV3Update{ped.pos + velocity * dT, direction, heading_angle};
};

void CollisionFreeSpeedModelV3::ApplyUpdate(const OperationalModelUpdate& upd, GenericAgent& agent)
    const
{
    const auto& update = std::get<CollisionFreeSpeedModelV3Update>(upd);
    agent.pos = update.position;
    agent.orientation = update.orientation;
    auto& model = std::get<CollisionFreeSpeedModelV3Data>(agent.model);
    model.headingAngle = update.headingAngle;
}

void CollisionFreeSpeedModelV3::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& model = std::get<CollisionFreeSpeedModelV3Data>(agent.model);

    validateConstraint(model.radius, 0.0, 2.0, "radius", true);
    validateConstraint(model.v0, 0.0, 10.0, "v0");
    validateConstraint(model.timeGap, 0.1, 10.0, "timeGap");

    validateConstraint(model.strengthNeighborRepulsion, 0.0, std::numeric_limits<double>::max(), "strengthNeighborRepulsion");
    validateConstraint(model.rangeNeighborRepulsion, 0.01, std::numeric_limits<double>::max(), "rangeNeighborRepulsion");
    validateConstraint(model.strengthGeometryRepulsion, 0.0, std::numeric_limits<double>::max(), "strengthGeometryRepulsion");
    validateConstraint(model.rangeGeometryRepulsion, 0.01, std::numeric_limits<double>::max(), "rangeGeometryRepulsion");

    validateConstraint(model.rangeXScale, 0.01, std::numeric_limits<double>::max(), "rangeXScale");
    validateConstraint(model.rangeYScale, 0.01, std::numeric_limits<double>::max(), "rangeYScale");
    validateConstraint(model.thetaMaxUpperBound, 0.0, std::acos(-1.0), "thetaMaxUpperBound");
    validateConstraint(model.agentBuffer, 0.0, 100.0, "agentBuffer");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
            continue;
        }
        const auto& neighbor_model = std::get<CollisionFreeSpeedModelV3Data>(neighbor.model);
        const auto contactDist = model.radius + neighbor_model.radius;
        const auto distance = (agent.pos - neighbor.pos).Norm();
        if(contactDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.pos,
                neighbor.pos,
                distance);
        }
    }

    const auto lineSegments = geometry.LineSegmentsInDistanceTo(model.radius, agent.pos);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance "
            "<= {}",
            agent.pos,
            model.radius);
    }
}

std::unique_ptr<OperationalModel> CollisionFreeSpeedModelV3::Clone() const
{
    return std::make_unique<CollisionFreeSpeedModelV3>(*this);
}

double CollisionFreeSpeedModelV3::OptimalSpeed(
    const GenericAgent& ped,
    double spacing,
    double time_gap) const
{
    const auto& model = std::get<CollisionFreeSpeedModelV3Data>(ped.model);
    const auto effective_spacing = spacing - model.agentBuffer;
    return std::min(std::max(effective_spacing / time_gap, MinReverseSpeed), model.v0);
}

double CollisionFreeSpeedModelV3::GetSpacing(
    const GenericAgent& ped1,
    const GenericAgent& ped2,
    const Point& direction) const
{
    const auto& model1 = std::get<CollisionFreeSpeedModelV3Data>(ped1.model);
    const auto& model2 = std::get<CollisionFreeSpeedModelV3Data>(ped2.model);
    const auto distp12 = ped2.pos - ped1.pos;
    if(direction.ScalarProduct(distp12) < 0.0) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = model1.radius + model2.radius;
    const auto inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }

    return distp12.Norm() - l;
}

Point CollisionFreeSpeedModelV3::NeighborRepulsion(
    const GenericAgent& ped1,
    const GenericAgent& ped2) const
{
    const auto distp12 = ped2.pos - ped1.pos;
    const auto [distance, direction] = distp12.NormAndNormalized();
    const auto& model1 = std::get<CollisionFreeSpeedModelV3Data>(ped1.model);
    const auto& model2 = std::get<CollisionFreeSpeedModelV3Data>(ped2.model);

    const auto l = model1.radius + model2.radius;
    return direction * -(model1.strengthNeighborRepulsion *
                         std::exp((l - distance) / model1.rangeNeighborRepulsion));
}

Point CollisionFreeSpeedModelV3::BoundaryRepulsion(
    const GenericAgent& ped,
    const LineSegment& boundary_segment) const
{
    const auto pt = boundary_segment.ShortestPoint(ped.pos);
    const auto dist_vec = pt - ped.pos;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto& model = std::get<CollisionFreeSpeedModelV3Data>(ped.model);
    const auto l = model.radius;
    const auto R_iw =
        -model.strengthGeometryRepulsion * std::exp((l - dist) / model.rangeGeometryRepulsion);
    return e_iw * R_iw;
}
