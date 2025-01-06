// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"
#include <random> 
#include "AnticipationVelocityModelData.hpp"
#include "AnticipationVelocityModelUpdate.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "OperationalModel.hpp"
#include "SimulationError.hpp"
#include "Macros.hpp"
#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

OperationalModelType AnticipationVelocityModel::Type() const
{
  return OperationalModelType::ANTICIPATION_VELOCITY_MODEL;
}

OperationalModelUpdate AnticipationVelocityModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, _cutOffRadius);
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    // Remove any agent from the neighborhood that is obstructed by geometry and the current
    // agent
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

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        Point{},
        [&ped, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(ped, neighbor);
        });

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(ped, element);
        });

    const auto desired_direction = (ped.destination - ped.pos).Normalized();
    auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    if(direction == Point{}) {
        direction = ped.orientation;
    }
    // update direction towards the newly calculated direction
    direction = UpdateDirection(ped, direction, dT);
    const auto spacing = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&ped, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(ped, neighbor, direction));
        });

    const auto& model = std::get<AnticipationVelocityModelData>(ped.model);
    const auto optimal_speed = OptimalSpeed(ped, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    return AnticipationVelocityModelUpdate{ped.pos + velocity * dT, direction};
};

void AnticipationVelocityModel::ApplyUpdate(const OperationalModelUpdate& upd, GenericAgent& agent)
    const
{
    const auto& update = std::get<AnticipationVelocityModelUpdate>(upd);
    agent.pos = update.position;
    agent.orientation = update.orientation;
}

Point AnticipationVelocityModel::UpdateDirection(const GenericAgent& ped, const Point & calculatedDirection, double dt
) const{
  const auto& model = std::get<AnticipationVelocityModelData>(ped.model);
  const Point desiredDirection = (ped.destination - ped.pos).Normalized();
  Point actualDirection = ped.orientation;
  Point updatedDirection;

  if(desiredDirection.ScalarProduct(calculatedDirection)*desiredDirection.ScalarProduct(actualDirection) < 0) {
    updatedDirection = calculatedDirection;
  }
  else{
  // Compute the rate of change of direction (Eq. 7)
  const Point directionDerivative =
      (calculatedDirection.Normalized() - actualDirection) / model.reactionTime;
  updatedDirection = actualDirection + directionDerivative * dt;
}

  return updatedDirection.Normalized();
}


void AnticipationVelocityModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& model = std::get<AnticipationVelocityModelData>(agent.model);

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

    const auto anticipationTime = model.anticipationTime;
    constexpr double anticipationTimeMin = 0.0;
    constexpr double anticipationTimeMax = 5.0;
    validateConstraint(anticipationTime, anticipationTimeMin, anticipationTimeMax, "anticipationTime");

    const auto reactionTime = model.reactionTime;
    constexpr double reactionTimeMin = 0.05;
    constexpr double reactionTimeMax = 1.0;
    validateConstraint(reactionTime, reactionTimeMin, reactionTimeMax, "reactionTime");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
            continue;
        }
        const auto& neighbor_model = std::get<AnticipationVelocityModelData>(neighbor.model);
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

std::unique_ptr<OperationalModel> AnticipationVelocityModel::Clone() const
{
    return std::make_unique<AnticipationVelocityModel>(*this);
}

double AnticipationVelocityModel::OptimalSpeed(
    const GenericAgent& ped,
    double spacing,
    double time_gap) const
{
    const auto& model = std::get<AnticipationVelocityModelData>(ped.model);
    double min_spacing = -0.1;
    return std::min(std::max(spacing / time_gap, min_spacing
                           ), model.v0);
}

double AnticipationVelocityModel::GetSpacing(
    const GenericAgent& ped1,
    const GenericAgent& ped2,
    const Point& direction) const
{
    const auto& model1 = std::get<AnticipationVelocityModelData>(ped1.model);
    const auto& model2 = std::get<AnticipationVelocityModelData>(ped2.model);
    const auto distp12 = ped2.pos - ped1.pos;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto private_space = 0.5; 
    const auto l = model1.radius + model2.radius;
    
    float min_dist = l + private_space;// TODO
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - min_dist;
}

Point AnticipationVelocityModel::CalculateInfluenceDirection(const Point& desiredDirection, const Point& predictedDirection) const
{
  // Eq. (5)
  const double seed = 42;
  static std::random_device rd;
  static std::mt19937 gen(seed);
  static std::uniform_int_distribution<int> dist(0, 1); // Random integers: 0 or 1

  Point orthogonalDirection = Point(-desiredDirection.y, desiredDirection.x).Normalized();
  double alignment = orthogonalDirection.ScalarProduct(predictedDirection);
  Point influenceDirection = orthogonalDirection;
  if (fabs(alignment) < J_EPS) {
    // Choose a random direction (left or right)
    if (dist(gen) % 2 == 0) {
      influenceDirection = -orthogonalDirection;
    }
  } else if (alignment > 0) {
    influenceDirection = -orthogonalDirection;
  }
  return influenceDirection;
}

Point AnticipationVelocityModel::NeighborRepulsion(
    const GenericAgent& ped1,
    const GenericAgent& ped2) const
{
    const auto& model1 = std::get<AnticipationVelocityModelData>(ped1.model);
    const auto& model2 = std::get<AnticipationVelocityModelData>(ped2.model);

    const auto distp12 = ped2.pos - ped1.pos;
    const auto [distance, ep12] = distp12.NormAndNormalized();
    const double adjustedDist = distance - (model1.radius + model2.radius);
    
    // Pedestrian movement and desired directions
    const auto& e1 = ped1.orientation;
    const auto& d1 = (ped1.destination - ped1.pos).Normalized();
    const auto& e2 = ped2.orientation;
    
    // Check perception range (Eq. 1)
    const auto inPerceptionRange =  d1.ScalarProduct(ep12) >= 0 || e1.ScalarProduct(ep12) >= 0;
    if(!inPerceptionRange) return Point(0, 0);
    
    double S_Gap = (model1.velocity - model2.velocity).ScalarProduct(ep12) * model1.anticipationTime;
    double R_dist = adjustedDist - S_Gap;
    R_dist = std::max(R_dist, 0.0); // Clamp to zero if negative

    // Interaction strength (Eq. 3 & 4)
    constexpr double alignmentBase = 1.0;
    constexpr double alignmentWeight = 0.5;
    const double alignmentFactor = alignmentBase + alignmentWeight * (1.0 - d1.ScalarProduct(e2));
    const double interactionStrength = model1.strengthNeighborRepulsion * alignmentFactor * std::exp(-R_dist/model1.rangeNeighborRepulsion);
    auto newep12 = distp12 + model2.velocity*model2.anticipationTime ; //e_ij(t+ta)

    // Compute adjusted influence direction
    const auto influenceDirection = CalculateInfluenceDirection(d1, newep12);
    return influenceDirection * interactionStrength;

}

Point AnticipationVelocityModel::BoundaryRepulsion(
    const GenericAgent& ped,
    const LineSegment& boundarySegment) const
{
    const auto closestPoint = boundarySegment.ShortestPoint(ped.pos);
    const auto distanceVector = closestPoint - ped.pos;
    const auto [dist, directionToBoundary] = distanceVector.NormAndNormalized();
    const auto& model = std::get<AnticipationVelocityModelData>(ped.model);
    const auto& desiredDirection = (ped.destination - ped.pos).Normalized();
    double result_e0 = desiredDirection.ScalarProduct(directionToBoundary);
    double result_ei = ped.destination.ScalarProduct(directionToBoundary);

    // Check if the boundary is behind the pedestrian or the destination is behind the pedestrian
    if (result_e0 < 0 && result_ei < 0) return Point(0, 0);

    const auto l = model.radius;
    const auto boundaryRepulsionStrength =
      -model.strengthGeometryRepulsion * exp((l - dist) / model.rangeGeometryRepulsion);
    const auto adjustedDirection = CalculateInfluenceDirection(desiredDirection, directionToBoundary);
    return adjustedDirection * boundaryRepulsionStrength;

}
