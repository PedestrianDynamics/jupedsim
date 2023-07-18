/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "VelocityModel.hpp"

#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"

#include <Logger.hpp>
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
    : OperationalModelBase(profiles), _aPed(aped), _DPed(Dped), _aWall(awall), _DWall(Dwall)
{
}

PedestrianUpdate VelocityModel::ComputeNewPosition(
    double dT,
    const Data& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const double radius = 4.0;
    auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, radius);
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    // Remove any agent from the neighborhood that is obstructed by geometry and the current
    // agent
    neighborhood.erase(
        std::remove_if(
            std::begin(neighborhood),
            std::end(neighborhood),
            [&ped, &walls](const auto& n) {
                if(ped.id == n->id) {
                    return true;
                }
                const auto lineSegment = LineSegment(ped.pos, n->pos);

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
    double min_spacing = 100.0;
    Point repPed = Point(0, 0);
    for(const auto neighbor : neighborhood) {
        repPed += ForceRepPed(ped, *neighbor);
    }
    // repulsive forces to walls and closed transitions that are not my target
    Point repWall = ForceRepRoom(ped, geometry);

    // calculate new direction ei according to (6)
    PedestrianUpdate update{};
    e0(ped, ped.destination, dT, update);
    const Point direction = update.e0 + repPed + repWall;
    for(const auto neighbor : neighborhood) {
        double spacing = GetSpacing(ped, *neighbor, direction).first;
        min_spacing = std::min(min_spacing, spacing);
    }

    update.velocity = direction.Normalized() * OptimalSpeed(ped, min_spacing, parameters.timeGap);
    update.position = ped.pos + *update.velocity * dT;
    return update;
};

void VelocityModel::ApplyUpdate(const PedestrianUpdate& update, Data& agent) const
{
    if(update.resetTurning) {
        agent.orientationDelay = 0;
    } else {
        ++agent.orientationDelay;
    }
    agent.e0 = update.e0;
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
    const Data& agent,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    const auto r = parameterProfile(agent.parameterProfileId).radius;
    for(const auto& neighbor : neighbors) {
        const auto contanctdDist = r + parameterProfile(neighbor->parameterProfileId).radius;
        const auto distance = (agent.pos - neighbor->pos).Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}",
                agent.id,
                neighbor->id);
        }
    }
}

std::unique_ptr<OperationalModel> VelocityModel::Clone() const
{
    return std::make_unique<VelocityModel>(*this);
}

void VelocityModel::e0(const Data& ped, Point target, double deltaT, PedestrianUpdate& update) const
{
    Point desired_direction;
    const auto pos = ped.pos;
    const auto dest = ped.destination;
    const auto dist = (dest - pos).Norm();
    if(dist > J_EPS_GOAL) {
        desired_direction = mollify_e0(target, pos, deltaT, ped.orientationDelay, ped.e0);
    } else {
        update.resetTurning = true;
        desired_direction = ped.e0;
    }
    update.e0 = desired_direction;
}

double VelocityModel::OptimalSpeed(const Data& ped, double spacing, double t) const
{
    const auto& profile = parameterProfile(ped.parameterProfileId);
    const double l = 2 * profile.radius;
    double speed = (spacing - l) / t;
    speed = (speed > 0) ? speed : 0;
    speed = (speed < profile.v0) ? speed : profile.v0;
    return speed;
}

// return spacing and id of the nearest pedestrian
my_pair VelocityModel::GetSpacing(const Data& ped1, const Data& ped2, Point ei) const
{
    Point distp12 = ped2.pos - ped1.pos; // inversed sign
    const double distance = distp12.Norm();
    const double l = 2 * parameterProfile(ped1.parameterProfileId).radius;
    Point ep12;
    ep12 = distp12.Normalized();

    double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
    double condition2 =
        ei.Rotate(0, 1).ScalarProduct(ep12); // theta = pi/2. condition2 should <= than l/Distance
    condition2 = (condition2 > 0) ? condition2 : -condition2; // abs

    if((condition1 >= 0) && (condition2 <= l / distance)) {
        // return a pair <dist, condition1>. Then take the smallest dist. In case of equality
        // the biggest condition1
        return my_pair(distp12.Norm(), ped2.id);
    }
    return my_pair(FLT_MAX, ped2.id);
}
Point VelocityModel::ForceRepPed(const Data& ped1, const Data& ped2) const
{
    Point F_rep(0.0, 0.0);
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = ped2.pos - ped1.pos;
    double Distance = distp12.Norm();
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double R_ij;
    double l = 2 * parameterProfile(ped1.parameterProfileId).radius;

    ep12 = distp12.Normalized();
    Point ei = ped1.orientation;
    double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
    condition1 = (condition1 > 0) ? condition1 : 0; // abs

    R_ij = -_aPed * exp((l - Distance) / _DPed);
    F_rep = ep12 * R_ij;

    return F_rep;
}

Point VelocityModel::ForceRepRoom(const Data& ped, const CollisionGeometry& geometry) const
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

Point VelocityModel::ForceRepWall(const Data& ped, const LineSegment& w) const
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
