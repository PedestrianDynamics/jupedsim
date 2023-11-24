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
#include "SimulationError.hpp"
#include "Stage.hpp"

#include <algorithm>
#include <limits>
#include <memory>
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
    Point newPosition{0, 0};
    double minPotential = std::numeric_limits<double>::max();

    for(size_t circle = 0; circle < numberCircles; ++circle) {
        for(size_t circlePosition = 0; circlePosition < positionsPerCircle; ++circlePosition) {
            Point candidatePosition;
            // compute candidate position

            double targetPotential = computeDistancePotential(candidatePosition, ped.target);
            double neighborPotential =
                computeNeighborsPotential(candidatePosition, ped, neighborhoodSearch);
            double boundaryPotential = computeBoundaryPotential(candidatePosition, ped, geometry);

            double candidatePotential = targetPotential + neighborPotential + boundaryPotential;

            if(candidatePotential < minPotential) {
                newPosition = candidatePosition;
            }
        }
    }

    // compute next time to act
    double nextTimeToAct = 0;

    OptimalStepsModelUpdate update;
    update.position = newPosition;
    update.nextTimeToAct = nextTimeToAct;
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
    // constexpr double rMin = 0.;
    // constexpr double rMax = 2.;
    // validateConstraint(r, rMin, rMax, "radius", true);

    // const auto v0 = model.v0;
    // constexpr double v0Min = 0.;
    // constexpr double v0Max = 10.;
    // validateConstraint(v0, v0Min, v0Max, "v0");

    // const auto timeGap = model.timeGap;
    // constexpr double timeGapMin = 0.1;
    // constexpr double timeGapMax = 10.;
    // validateConstraint(timeGap, timeGapMin, timeGapMax, "timeGap");

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
    const double radii = std::get<OptimalStepsModelData>(agent.model).radius +
                         std::get<OptimalStepsModelData>(otherAgent.model).radius;

    const double distanceSq = (position - otherAgent.pos).NormSquare();
    const double maxDistanceSq = (std::max(personalWidth, intimateWidth) + radii) *
                                 (std::max(personalWidth, intimateWidth) + radii);

    if(distanceSq < maxDistanceSq) {
        const double distance = (position - otherAgent.pos).Norm();

        const int intimateSpacePower = 1; // b_p
        const int personalSpacePower = 1;
        const double intimateSpaceFactor = 2.; // a_p

        if(distance < radii) {
            // vadere
            return 1000 * std::exp(1 / (std::pow(distance / radii, 4) - 1));
            // sivers 2016
            // return 1000 * std::exp(1 / (std::pow(distance / radii, 2) - 1));
        }
        if(distance < intimateWidth + radii) {
            return repulsionIntensity / intimateSpaceFactor *
                   std::exp(
                       4 /
                       (std::pow(distance / (intimateWidth + radii), 2 * intimateSpacePower) - 1));
        }

        if(distance < personalWidth + radii) {
            return repulsionIntensity *
                   std::exp(
                       4 /
                       (std::pow(distance / (personalWidth + radii), 2 * personalSpacePower) - 1));
        }
    }

    return 0;
}

double OptimalStepsModel::computeBoundaryPotential(
    const Point& position,
    const GenericAgent& agent,
    const CollisionGeometry& geometry) const
{
    double boundaryPotential = 0;
    const auto& lineSegments = geometry.LineSegmentsInApproxDistanceTo(position);
    const double radius = std::get<OptimalStepsModelData>(agent.model).radius;

    for(const auto& lineSegment : lineSegments) {
        // TODO (TS): Check if line segment "directly visibile"?
        auto distance = lineSegment.DistTo(position);

        if(distance < radius) {
            boundaryPotential += 100000 * std::exp(1 / (std::pow(distance / radius, 2) - 1));
        } else if(distance < geometryWidth) {
            boundaryPotential +=
                geometryHeight * std::exp(2 / (std::pow(distance / (geometryWidth), 2) - 1));
        }
    }
    return boundaryPotential;
}
