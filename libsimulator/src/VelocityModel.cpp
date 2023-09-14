// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "VelocityModel.hpp"

#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "VelocityModelData.hpp"

#include <Logger.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

double xRight = 26.0;
double xLeft = 0.0;
double cutoff = 2.0;

VelocityModel::VelocityModel(
    double aped,
    double Dped,
    double awall,
    double Dwall,
    const std::vector<VelocityModelAgentParameters>& profiles)
    : OperationalModelBase(profiles)
    , _aPed(aped)
    , _DPed(Dped)
    , _aWall(awall)
    , _DWall(Dwall)
    , _cutOffRadius(
          2 * std::max_element(
                  std::begin(profiles),
                  std::end(profiles),
                  [](const auto& p1, const auto& p2) { return p1.radius < p2.radius; })
                  ->radius -
          Dped * log(_minForce / aped))
{
}

OperationalModelType VelocityModel::Type() const
{
    return OperationalModelType::VELOCITY;
}

PedestrianUpdate VelocityModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, _cutOffRadius);
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    // Remove any agent from the neighborhood that is obstructed by geometry and the current
    // agent
    neighborhood.erase(
        std::remove_if(
            std::begin(neighborhood),
            std::end(neighborhood),
            [&ped, &walls](const auto& n) {
                if(ped.id == n.id) {
                    return true;
                }
                const auto lineSegment = LineSegment(ped.pos, n.pos);

                if(std::find_if(
                       walls.cbegin(), walls.cend(), [&lineSegment](const auto& candidate) {
                           return intersects(lineSegment, candidate);
                       }) != walls.end()) {
                    return true;
                }

                return false;
            }),
        std::end(neighborhood));

    const auto& parameters = parameterProfile(ped.parameterProfileId);
    double min_spacing = std::numeric_limits<double>::max();
    Point repPed = Point(0, 0);
    for(const auto& neighbor : neighborhood) {
        repPed += ForceRepPed(ped, neighbor);
    }
    // repulsive forces to walls and closed transitions that are not my target
    Point repWall = ForceRepRoom(ped, geometry);

    // calculate new direction ei according to (6)
    PedestrianUpdate update{};
    e0(ped, ped.destination, dT, update);
    const Point direction = (update.e0 + repPed + repWall).Normalized();
    for(const auto& neighbor : neighborhood) {
        const double spacing = GetSpacing(ped, neighbor, direction);
        min_spacing = std::min(min_spacing, spacing);
    }

    update.velocity = direction * OptimalSpeed(ped, min_spacing, parameters.timeGap);
    update.position = ped.pos + *update.velocity * dT;
    return update;
};

void VelocityModel::ApplyUpdate(const PedestrianUpdate& update, GenericAgent& agent) const
{
    auto& model = std::get<VelocityModelData>(agent.model);
    if(update.resetTurning) {
        model.orientationDelay = 0;
    } else {
        ++model.orientationDelay;
    }
    model.e0 = update.e0;
    if(update.position) {
        agent.pos = *update.position;
    }
    if(update.velocity) {
        const auto dir = (*update.velocity).Normalized();
        if(dir != Point{0, 0}) {
            agent.orientation = dir;
        }
    }
}
void VelocityModel::CheckDistanceConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    const auto r = parameterProfile(agent.parameterProfileId).radius;
    for(const auto& neighbor : neighbors) {
        const auto contanctdDist = r + parameterProfile(neighbor.parameterProfileId).radius;
        const auto distance = (agent.pos - neighbor.pos).Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.pos,
                neighbor.pos,
                distance);
        }
    }
}

std::unique_ptr<OperationalModel> VelocityModel::Clone() const
{
    return std::make_unique<VelocityModel>(*this);
}

void VelocityModel::e0(
    const GenericAgent& ped,
    Point target,
    double deltaT,
    PedestrianUpdate& update) const
{
    Point desired_direction;
    const auto pos = ped.pos;
    const auto dest = ped.destination;
    const auto dist = (dest - pos).Norm();
    const auto& model = std::get<VelocityModelData>(ped.model);
    if(dist > J_EPS_GOAL) {
        desired_direction = mollify_e0(target, pos, deltaT, model.orientationDelay, model.e0);
    } else {
        update.resetTurning = true;
        desired_direction = model.e0;
    }
    update.e0 = desired_direction;
}

double VelocityModel::OptimalSpeed(const GenericAgent& ped, double spacing, double t) const
{
    const auto& profile = parameterProfile(ped.parameterProfileId);
    const double l = 2 * profile.radius;
    double speed = (spacing - l) / t;
    speed = (speed > 0) ? speed : 0;
    speed = (speed < profile.v0) ? speed : profile.v0;
    return speed;
}

// return spacing and id of the nearest pedestrian
double VelocityModel::GetSpacing(
    const GenericAgent& ped1,
    const GenericAgent& ped2,
    const Point direction) const
{
    const Point distp12 = ped2.pos - ped1.pos;
    const bool inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const Point left = direction.Rotate90Deg();
    const double l = parameterProfile(ped1.parameterProfileId).radius +
                     parameterProfile(ped2.parameterProfileId).radius;
    if(std::abs(left.ScalarProduct(distp12)) > l) {
        return std::numeric_limits<double>::max();
    }

    return distp12.Norm();
}
Point VelocityModel::ForceRepPed(const GenericAgent& ped1, const GenericAgent& ped2) const
{
    const Point distp12 = ped2.pos - ped1.pos;
    const double dist = distp12.Norm();
    const double l = parameterProfile(ped1.parameterProfileId).radius +
                     parameterProfile(ped2.parameterProfileId).radius;
    const auto ep12 = distp12 / dist;
    const double R_ij = -_aPed * exp((l - dist) / _DPed);
    return ep12 * R_ij;
}

Point VelocityModel::ForceRepRoom(const GenericAgent& ped, const CollisionGeometry& geometry) const
{
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    auto f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + ForceRepWall(ped, element);
        });
    return f;
}

Point VelocityModel::ForceRepWall(const GenericAgent& ped, const LineSegment& w) const
{
    if(const auto distGoal = (ped.destination - ped.pos).Norm(); distGoal < J_EPS_GOAL) {
        return Point{0, 0};
    }

    const Point pt = w.ShortestPoint(ped.pos);
    const Point dist_vec = pt - ped.pos;
    const double dist = dist_vec.Norm();
    const Point e_iw = dist_vec / dist;

    const double l = parameterProfile(ped.parameterProfileId).radius;
    const double R_iw = -_aWall * exp((l - dist) / _DWall);
    return e_iw * R_iw;
}
