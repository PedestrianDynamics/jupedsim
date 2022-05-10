/**
 * \file        VelocityModel.cpp
 * \date        Aug. 07, 2015
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * Implementation of first-order model
 * 3. Velocity Model: Tordeux2015
 *
 *
 **/
#include "VelocityModel.hpp"

#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "Pedestrian.hpp"
#include "Simulation.hpp"

#include <Logger.hpp>
#include <memory>
#include <numeric>

double xRight = 26.0;
double xLeft = 0.0;
double cutoff = 2.0;

VelocityModel::VelocityModel(double aped, double Dped, double awall, double Dwall)
    : _aPed(aped), _DPed(Dped), _aWall(awall), _DWall(Dwall)
{
}

PedestrianUpdate VelocityModel::ComputeNewPosition(
    double dT,
    const Pedestrian& ped,
    const Geometry& geometry,
    const NeighborhoodSearch& neighborhoodSearch) const
{
    const auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.GetPos(), 4);
    double min_spacing = 100.0;
    Point repPed = Point(0, 0);
    const Point p1 = ped.GetPos();
    for(const auto* other : neighborhood) {
        if(other->GetUID() == ped.GetUID()) {
            continue;
        }
        if(!geometry.IntersectsAny(Line(p1, other->GetPos()))) {
            repPed += ForceRepPed(&ped, other);
        }
    }
    // repulsive forces to walls and closed transitions that are not my target
    Point repWall = ForceRepRoom(&ped, geometry);

    // calculate new direction ei according to (6)
    PedestrianUpdate update{};
    e0(&ped, ped.destination, update);
    const Point direction = update.v0 + repPed + repWall;
    for(const auto* other : neighborhood) {
        if(other->GetUID() == ped.GetUID()) {
            continue;
        }
        if(!geometry.IntersectsAny(Line(p1, other->GetPos()))) {
            double spaceing = GetSpacing(&ped, other, direction).first;
            min_spacing = std::min(min_spacing, spaceing);
        }
    }

    update.velocity = direction.Normalized() * OptimalSpeed(&ped, min_spacing);
    update.position = ped.GetPos() + *update.velocity * dT;
    if(update.velocity->Norm() >= J_EPS_V) {
        update.resetPhi = true;
    }
    return update;
};

void VelocityModel::ApplyUpdate(const PedestrianUpdate& update, Pedestrian& agent) const
{
    if(update.resetTurning) {
        agent.SetSmoothTurning();
    } else {
        agent.IncrementOrientationDelay();
    }
    agent.SetV0(update.v0);
    if(update.resetPhi) {
        agent.SetPhiPed();
    }
    if(update.position) {
        agent.SetPos(*update.position);
    }
    if(update.velocity) {
        agent.SetV(*update.velocity);
    }
}

void VelocityModel::e0(const Pedestrian* ped, Point target, PedestrianUpdate& update) const
{
    Point desired_direction;
    const auto pos = ped->GetPos();
    const auto dest = ped->destination;
    const auto dist = (dest - pos).Norm();
    if(dist > J_EPS_GOAL) {
        desired_direction = ped->GetV0(target);
    } else {
        update.resetTurning = true;
        desired_direction = ped->GetV0();
    }
    update.v0 = desired_direction;
}

double VelocityModel::OptimalSpeed(const Pedestrian* ped, double spacing) const
{
    double v0 = ped->GetV0Norm();
    double T = ped->GetT();
    double l = 2 * ped->GetEllipse().GetBmax(); // assume peds are circles with const radius
    double speed = (spacing - l) / T;
    speed = (speed > 0) ? speed : 0;
    speed = (speed < v0) ? speed : v0;
    //      (1-winkel)*speed;
    // todo use winkel
    return speed;
}

// return spacing and id of the nearest pedestrian
my_pair VelocityModel::GetSpacing(const Pedestrian* ped1, const Pedestrian* ped2, Point ei) const
{
    Point distp12 = ped2->GetPos() - ped1->GetPos(); // inversed sign
    double Distance = distp12.Norm();
    double l = 2 * ped1->GetEllipse().GetBmax();
    Point ep12;
    if(Distance >= J_EPS) {
        ep12 = distp12.Normalized();
    } else {
        LOG_WARNING(
            "VelocityModel::GetSPacing() ep12 can not be calculated! Pedestrians are to close "
            "to "
            "each other ({:f})",
            Distance);
        exit(EXIT_FAILURE); // TODO
    }

    double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
    double condition2 =
        ei.Rotate(0, 1).ScalarProduct(ep12); // theta = pi/2. condition2 should <= than l/Distance
    condition2 = (condition2 > 0) ? condition2 : -condition2; // abs

    if((condition1 >= 0) && (condition2 <= l / Distance)) {
        // return a pair <dist, condition1>. Then take the smallest dist. In case of equality the
        // biggest condition1
        return my_pair(distp12.Norm(), ped2->GetUID());
    }
    return my_pair(FLT_MAX, ped2->GetUID());
}
Point VelocityModel::ForceRepPed(const Pedestrian* ped1, const Pedestrian* ped2) const
{
    Point F_rep(0.0, 0.0);
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = ped2->GetPos() - ped1->GetPos();
    double Distance = distp12.Norm();
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double R_ij;
    double l = 2 * ped1->GetEllipse().GetBmax();

    if(Distance >= J_EPS) {
        ep12 = distp12.Normalized();
    } else {
        LOG_ERROR(
            "VelocityModel::forcePedPed() ep12 can not be calculated! Pedestrians are too near "
            "to "
            "each other (dist={:f}). Adjust <a> value in force_ped to counter this. Affected "
            "pedestrians ped1 {} at ({:f},{:f}) and ped2 {} at ({:f}, {:f})",
            Distance,
            ped1->GetUID(),
            ped1->GetPos().x,
            ped1->GetPos().y,
            ped2->GetUID(),
            ped2->GetPos().x,
            ped2->GetPos().y);
        exit(EXIT_FAILURE); // TODO: quick and dirty fix for issue #158
                            //  (sometimes sources create peds on the same location)
    }
    Point ei = ped1->GetV().Normalized();
    if(ped1->GetV().NormSquare() < 0.01) {
        ei = ped1->GetV0().Normalized();
    }
    double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
    condition1 = (condition1 > 0) ? condition1 : 0; // abs

    R_ij = -_aPed * exp((l - Distance) / _DPed);
    F_rep = ep12 * R_ij;

    return F_rep;
}

Point VelocityModel::ForceRepRoom(const Pedestrian* ped, const Geometry& geometry) const
{
    auto walls = geometry.LineSegmentsInDistanceTo(5.0, ped->GetPos());

    auto f = std::accumulate(
        walls.begin(),
        walls.end(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + ForceRepWall(ped, element);
        });
    return f;
}

Point VelocityModel::ForceRepWall(const Pedestrian* ped, const Line& w) const
{
    if(const auto distGoal = (ped->destination - ped->GetPos()).Norm(); distGoal < J_EPS_GOAL) {
        return Point{0, 0};
    }

    const Point pt = w.ShortestPoint(ped->GetPos());
    const Point dist_vec = pt - ped->GetPos();
    const double dist = dist_vec.Norm();
    const Point e_iw = dist_vec / dist;

    const double l = ped->GetEllipse().GetBmax();
    const double R_iw = -_aWall * exp((l - dist) / _DWall);
    return e_iw * R_iw;
}
