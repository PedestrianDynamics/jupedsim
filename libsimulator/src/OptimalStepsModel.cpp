// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OptimalStepsModel.hpp"

#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Logger.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OptimalStepsModelData.hpp"
#include "RNG.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <numbers>
#include <numeric>
#include <vector>

OptimalStepsModel::OptimalStepsModel(
    double strengthNeighborRepulsion_,
    double rangeNeighborRepulsion_,
    double strengthGeometryRepulsion_,
    double rangeGeometryRepulsion_)
{
}

OperationalModelType OptimalStepsModel::Type() const
{
    return OperationalModelType::OPTIMAL_STEPS;
}

OperationalModelUpdate OptimalStepsModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const double radius = computeDesiredStepSize(ped);

    Point newPosition = ped.pos;
    double minPotential = computePotential(ped.pos, ped, geometry, neighborhoodSearch);

    for(size_t circlePosition = 0; circlePosition < positionsPerCircle; ++circlePosition) {
        auto candidatePosition = computeCandiateOnCircle(
            ped.pos,
            radius,
            circlePosition,
            positionsPerCircle,
            geometry.LineSegmentsInApproxDistanceTo(ped.pos));

        if(!candidatePosition) {
            continue;
        }

        double candidatePotential =
            computePotential(*candidatePosition, ped, geometry, neighborhoodSearch);

        if(candidatePotential < minPotential) {
            minPotential = candidatePotential;
            newPosition = *candidatePosition;
        }
    }

    // compute next time to act
    const double nextTimeToAct = computeNextTimeToAct(ped, radius);

    OptimalStepsModelUpdate update;
    update.position = newPosition;
    update.nextTimeToAct = nextTimeToAct;
    return update;
}

double OptimalStepsModel::computePotential(
    const Point& position,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    double targetPotential = computeDistancePotential(position, ped.target);
    double neighborPotential = computeNeighborsPotential(position, ped, neighborhoodSearch);
    double boundaryPotential = computeBoundaryPotential(position, ped, geometry);

    return targetPotential + neighborPotential + boundaryPotential;
}

void OptimalStepsModel::ApplyUpdate(const OperationalModelUpdate& upd, GenericAgent& agent) const
{
    const auto& update = std::get<OptimalStepsModelUpdate>(upd);
    agent.pos = update.position;
    std::get<OptimalStepsModelData>(agent.model).nextTimeToAct = update.nextTimeToAct;
}

void OptimalStepsModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& model = std::get<OptimalStepsModelData>(agent.model);

    const auto r = model.radius;

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<OptimalStepsModelData>(neighbor.model);
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

    // TODO radius < intimate
    // TODO intimate < personalWidth
}

std::unique_ptr<OperationalModel> OptimalStepsModel::Clone() const
{
    return std::make_unique<OptimalStepsModel>(*this);
}

double
OptimalStepsModel::computeDistancePotential(const Point& position, const Point& destination) const
{
    const auto path = routingEngine->ComputeAllWaypoints(position, destination);
    double distance = 0;
    for(size_t i = 1; i <= path.size(); ++i) {
        Point dist{path[i - 1] - path[i]};
        distance += dist.Norm();
    }
    return distance;
}

double OptimalStepsModel::computeNeighborsPotential(
    const Point& position,
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const double radius = std::get<OptimalStepsModelData>(agent.model).radius;

    // TODO (TS): replace 1.2 with model parameter
    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(position, 1.2 + radius);

    double potential = 0;
    for(const auto& neighbor : neighbors) {
        if(neighbor.id == agent.id) {
            continue;
        }
        potential += computeNeighborPotential(position, agent, neighbor);
    }
    return potential;
}

double OptimalStepsModel::computeNeighborPotential(
    const Point& position,
    const GenericAgent& agent,
    const GenericAgent& otherAgent) const
{
    const auto& agentModel = std::get<OptimalStepsModelData>(agent.model);
    const auto& otherModel = std::get<OptimalStepsModelData>(otherAgent.model);

    const double radii = agentModel.radius + otherModel.radius;
    const double distance = (position - otherAgent.pos).Norm();

    double potential = 0;
    if(distance < agentModel.personalSpaceWidth + radii) {
        potential += agentModel.repulsionIntensity *
                     std::exp(
                         4 / (std::pow(
                                  distance / (agentModel.personalSpaceWidth + radii),
                                  (2 * agentModel.personalSpacePower)) -
                              1));
    }

    if(distance < agentModel.intimateSpaceWidth + radii) {
        potential += (agentModel.repulsionIntensity / agentModel.intimateSpaceFactor) *
                     std::exp(
                         4 / std::pow(
                                 distance / (agentModel.personalSpaceWidth + radii),
                                 2 * agentModel.personalSpacePower) -
                         1);
    }

    if(distance < radii) {
        potential += 1000 * std::exp(1 / (std::pow(distance / radii, 4) - 1));
    }

    return 0;
}

double OptimalStepsModel::computeBoundaryPotential(
    const Point& position,
    const GenericAgent& agent,
    const CollisionGeometry& geometry) const
{
    const auto& lineSegments = geometry.LineSegmentsInApproxDistanceTo(position);
    const auto& agentModel = std::get<OptimalStepsModelData>(agent.model);

    const double radius = agentModel.radius;

    double minDistance = std::numeric_limits<double>::max();

    for(const auto& lineSegment : lineSegments) {
        minDistance = std::min(minDistance, lineSegment.DistTo(position));
    }

    if(minDistance < radius) {
        return 100000 * std::exp(1 / (std::pow(minDistance / radius, 2) - 1));
    } else if(minDistance < agentModel.geometryWidth) {
        return agentModel.geometryHeight *
               std::exp(2 / (std::pow(minDistance / (agentModel.geometryWidth), 2) - 1));
    }

    return 0;
}

std::optional<Point> OptimalStepsModel::computeCandiateOnCircle(
    const Point& center,
    double radius,
    int count,
    int numberOfPoints,
    const std::vector<LineSegment>& walls) const
{
    const double x = center.x + radius * cos(2. * std::numbers::pi * count / numberOfPoints);
    const double y = center.y + radius * sin(2. * std::numbers::pi * count / numberOfPoints);

    const Point candidate{x, y};
    const LineSegment movement{candidate, center};

    for(const auto& wall : walls) {
        if(intersects(movement, wall)) {
            return std::nullopt;
        }
    }

    return candidate;
}

double OptimalStepsModel::computeDesiredStepSize(const GenericAgent& agent) const
{
    static Random rng{1200};

    double step = stepLengthIntercept +
                  stepLengthSlopeSpeed * std::get<OptimalStepsModelData>(agent.model).v0 +
                  rng.normalDistributen(0., 1.) * stepLengthSD;
    return step;
}

double OptimalStepsModel::computeNextTimeToAct(const GenericAgent& agent, double stepSize) const
{
    const auto& agentModel = std::get<OptimalStepsModelData>(agent.model);

    return agentModel.nextTimeToAct + stepSize / agentModel.v0;
}