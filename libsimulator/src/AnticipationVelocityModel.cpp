// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModel.hpp"

#include "AnticipationVelocityModelData.hpp"
#include "AnticipationVelocityModelUpdate.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Macros.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

AnticipationVelocityModel::AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed)
    : pushoutStrength(pushoutStrength), gen(rng_seed)
{
}

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

    const auto desiredDirection = (ped.destination - ped.pos).Normalized();
    auto direction = (desiredDirection + neighborRepulsion).Normalized();
    if(direction == Point{}) {
        direction = ped.orientation;
    }

    const auto& model = std::get<AnticipationVelocityModelData>(ped.model);
    const double wallBufferDistance = model.wallBufferDistance;
    // Wall sliding behavior

    // update direction towards the newly calculated direction
    direction = UpdateDirection(ped, direction, dT);
    direction = HandleWallAvoidance(direction, ped.pos, model.radius, boundary, wallBufferDistance);
    const auto spacing = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&ped, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(ped, neighbor, direction));
        });

    const auto optimal_speed = OptimalSpeed(ped, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    return AnticipationVelocityModelUpdate{
        .position = ped.pos + velocity * dT, .velocity = velocity, .orientation = direction};
};

void AnticipationVelocityModel::ApplyUpdate(const OperationalModelUpdate& upd, GenericAgent& agent)
    const
{
    const auto& update = std::get<AnticipationVelocityModelUpdate>(upd);
    auto& model = std::get<AnticipationVelocityModelData>(agent.model);
    agent.pos = update.position;
    agent.orientation = update.orientation;
    model.velocity = update.velocity;
}

Point AnticipationVelocityModel::UpdateDirection(
    const GenericAgent& ped,
    const Point& calculatedDirection,
    double dt) const
{
    const auto& model = std::get<AnticipationVelocityModelData>(ped.model);
    const Point desiredDirection = (ped.destination - ped.pos).Normalized();
    const Point actualDirection = ped.orientation;
    Point updatedDirection;

    if(desiredDirection.ScalarProduct(calculatedDirection) *
           desiredDirection.ScalarProduct(actualDirection) <
       0) {
        updatedDirection = calculatedDirection;
    } else {
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

    const auto strengthNeighborRepulsion = model.strengthNeighborRepulsion;
    constexpr double snMin = 0.;
    constexpr double snMax = 20.;
    validateConstraint(strengthNeighborRepulsion, snMin, snMax, "strengthNeighborRepulsion", false);

    const auto rangeNeighborRepulsion = model.rangeNeighborRepulsion;
    constexpr double rnMin = 0.;
    constexpr double rnMax = 5.;
    validateConstraint(rangeNeighborRepulsion, rnMin, rnMax, "rangeNeighborRepulsion", true);

    const auto buff = model.wallBufferDistance;
    constexpr double buffMin = 0.;
    constexpr double buffMax = 1.;
    validateConstraint(buff, buffMin, buffMax, "wallBufferDistance", false);

    const auto v0 = model.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto timeGap = model.timeGap;
    constexpr double timeGapMin = 0.;
    constexpr double timeGapMax = 10.;
    validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap", true);

    const auto anticipationTime = model.anticipationTime;
    constexpr double anticipationTimeMin = 0.0;
    constexpr double anticipationTimeMax = 5.0;
    validateConstraint(
        anticipationTime, anticipationTimeMin, anticipationTimeMax, "anticipationTime");

    const auto reactionTime = model.reactionTime;
    constexpr double reactionTimeMin = 0.0;
    constexpr double reactionTimeMax = 1.0;
    validateConstraint(reactionTime, reactionTimeMin, reactionTimeMax, "reactionTime", true);

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
    const double min_spacing = 0.0;
    return std::min(std::max(spacing / time_gap, min_spacing), model.v0);
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
    const auto l = model1.radius + model2.radius;
    const bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }
    return distp12.Norm() - l;
}

Point AnticipationVelocityModel::CalculateInfluenceDirection(
    const Point& desiredDirection,
    const Point& predictedDirection) const
{
    // Eq. (5)
    const Point orthogonalDirection = Point(-desiredDirection.y, desiredDirection.x).Normalized();
    const double alignment = orthogonalDirection.ScalarProduct(predictedDirection);
    Point influenceDirection = orthogonalDirection;
    if(fabs(alignment) < J_EPS) {
        // Choose a random direction (left or right)
        if(gen() % 2 == 0) {
            influenceDirection = -orthogonalDirection;
        }
    } else if(alignment > 0) {
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
    const auto inPerceptionRange = d1.ScalarProduct(ep12) >= 0 || e1.ScalarProduct(ep12) >= 0;
    if(!inPerceptionRange)
        return Point(0, 0);

    const double S_Gap =
        (model1.velocity - model2.velocity).ScalarProduct(ep12) * model1.anticipationTime;
    double R_dist = adjustedDist - S_Gap;
    R_dist = std::max(R_dist, 0.0); // Clamp to zero if negative

    // Interaction strength (Eq. 3 & 4)
    constexpr double alignmentBase = 1.0;
    constexpr double alignmentWeight = 0.5;
    const double alignmentFactor = alignmentBase + alignmentWeight * (1.0 - d1.ScalarProduct(e2));
    const double interactionStrength = model1.strengthNeighborRepulsion * alignmentFactor *
                                       std::exp(-R_dist / model1.rangeNeighborRepulsion);
    const auto newep12 = distp12 + model2.velocity * model2.anticipationTime; // e_ij(t+ta)

    // Compute adjusted influence direction
    const auto influenceDirection = CalculateInfluenceDirection(d1, newep12);
    return influenceDirection * interactionStrength;
}

Point AnticipationVelocityModel::HandleWallAvoidance(
    const Point& direction,
    const Point& agentPosition,
    double agentRadius,
    const std::vector<LineSegment>& boundary,
    double wallBufferDistance) const
{
    const double criticalWallDistance = wallBufferDistance + agentRadius;
    const double influenceDistance =
        1.0 * criticalWallDistance; // 1.0 for now. Might be different. Need more tests.

    Point modifiedDirection = direction;

    for(const auto& wall : boundary) {
        const auto closestPoint = wall.ShortestPoint(agentPosition);
        const auto distanceVector = agentPosition - closestPoint;
        const auto [distance, normalTowardAgent] = distanceVector.NormAndNormalized();

        if(distance > influenceDistance) {
            continue;
        }

        const auto dotProduct = modifiedDirection.ScalarProduct(normalTowardAgent);

        if(dotProduct < 0) {
            // Direction points into wall - need to project it out
            // Remove the component pointing into the wall
            const auto projectedDirection = modifiedDirection - normalTowardAgent * dotProduct;

            if(distance <= criticalWallDistance) {
                modifiedDirection = projectedDirection + normalTowardAgent * pushoutStrength;
            } else {
                // Soft influence: blend between original and projected based on distance
                const double blendFactor =
                    (influenceDistance - distance) / (influenceDistance - criticalWallDistance);
                modifiedDirection =
                    modifiedDirection * (1.0 - blendFactor) + projectedDirection * blendFactor;
            }
        }
    }

    // Renormalize to maintain speed
    const auto finalDirection = modifiedDirection.Normalized();

    return finalDirection;
}
