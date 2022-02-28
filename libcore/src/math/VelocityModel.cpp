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

#include "Simulation.hpp"
#include "direction/DirectionManager.hpp"
#include "direction/walking/DirectionStrategy.hpp"
#include "geometry/SubRoom.hpp"
#include "geometry/Wall.hpp"
#include "math/OperationalModel.hpp"
#include "neighborhood/NeighborhoodSearch.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <Logger.hpp>
#include <memory>

double xRight = 26.0;
double xLeft  = 0.0;
double cutoff = 2.0;

VelocityModel::VelocityModel(
    DirectionManager * directionManager,
    double aped,
    double Dped,
    double awall,
    double Dwall,
    unsigned int seed) :
    OperationalModel(directionManager),
    _aPed(aped),
    _DPed(Dped),
    _aWall(awall),
    _DWall(Dwall),
    _seed(seed)
{
}

PedestrianUpdate
VelocityModel::ComputeNewPosition(double dT, const Pedestrian & ped, Building * building) const
{
    auto [room, subroom]                 = building->GetRoomAndSubRoom(ped.GetPos());
    Point repPed                         = Point(0, 0);
    std::vector<Pedestrian *> neighbours = building->GetNeighborhoodSearch().GetNeighbourhood(&ped);

    int size = static_cast<int>(neighbours.size());
    for(int i = 0; i < size; i++) {
        Pedestrian * ped1 = neighbours[i];
        //if they are in the same subroom
        Point p1 = ped.GetPos();

        Point p2 = ped1->GetPos();

        auto [room1, subroom1] = building->GetRoomAndSubRoom(ped1->GetPos());
        //subrooms to consider when looking for neighbour for the 3d visibility
        std::vector<SubRoom *> emptyVector;
        emptyVector.push_back(subroom);
        emptyVector.push_back(subroom1);
        bool isVisible = building->IsVisible(p1, p2, emptyVector, false);
        if(!isVisible) {
            continue;
        }
        if(room == room1 && subroom == subroom1) {
            repPed += ForceRepPed(&ped, ped1);
        } else {
            // or in neighbour subrooms
            if(subroom->IsDirectlyConnectedWith(subroom1)) {
                repPed += ForceRepPed(&ped, ped1);
            }
        }
    } // for i
    //repulsive forces to walls and closed transitions that are not my target
    Point repWall = ForceRepRoom(&ped, subroom);

    double min_spacing = 100.0;
    // calculate new direction ei according to (6)
    PedestrianUpdate update{};
    e0(&ped, room, update);
    const Point direction = update.v0 + repPed + repWall;
    for(int i = 0; i < size; i++) {
        Pedestrian * ped1      = neighbours[i];
        auto [room1, subroom1] = building->GetRoomAndSubRoom(ped1->GetPos());
        // calculate spacing
        // my_pair spacing_winkel = GetSpacing(ped, ped1);
        if(room == room1 && subroom == subroom1) {
            double spaceing = GetSpacing(&ped, ped1, direction).first;
            min_spacing     = std::min(min_spacing, spaceing);
        } else {
            // or in neighbour subrooms
            if(subroom->IsDirectlyConnectedWith(subroom1)) {
                double spaceing = GetSpacing(&ped, ped1, direction).first;
                min_spacing     = std::min(min_spacing, spaceing);
            }
        }
    }

    update.velocity = direction.Normalized() * OptimalSpeed(&ped, min_spacing);
    update.position = ped.GetPos() + *update.velocity * dT;
    if(update.velocity->Norm() >= J_EPS_V) {
        update.resetPhi = true;
    }
    return update;
};

void VelocityModel::ApplyUpdate(const PedestrianUpdate & update, Pedestrian & agent) const
{
    if(update.resetTurning) {
        agent.SetSmoothTurning();
    } else {
        agent.IncrementOrientationDelay();
    }
    if(update.lastE0) {
        agent.SetLastE0(*update.lastE0);
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

void VelocityModel::e0(const Pedestrian * ped, const Room * room, PedestrianUpdate & update) const
{
    const Point target = _direction->GetTarget(room, ped);
    if(ped->IsWaiting()) {
        update.waitingPos = target;
    }

    Point desired_direction;
    const Point pos = ped->GetPos();
    const auto dist = ped->GetExitLine().DistTo(pos);

    // TODO(kkratz): Fix this hack
    const auto * strategy = &_direction->GetDirectionStrategy();
    if(dynamic_cast<const DirectionLocalFloorfield *>(strategy)) {
        Point lastE0      = ped->GetLastE0();
        update.lastE0     = target - pos;
        desired_direction = target - pos;
        if(desired_direction.NormSquare() < 0.25 && !ped->IsWaiting()) {
            desired_direction = lastE0;
            update.lastE0     = lastE0;
        }
    } else if(dist > J_EPS_GOAL) {
        desired_direction = ped->GetV0(target);
    } else {
        update.resetTurning = true;
        desired_direction   = ped->GetV0();
    }
    update.v0 = desired_direction;
}

double VelocityModel::OptimalSpeed(const Pedestrian * ped, double spacing) const
{
    double v0    = ped->GetV0Norm();
    double T     = ped->GetT();
    double l     = 2 * ped->GetEllipse().GetBmax(); //assume peds are circles with const radius
    double speed = (spacing - l) / T;
    speed        = (speed > 0) ? speed : 0;
    speed        = (speed < v0) ? speed : v0;
    //      (1-winkel)*speed;
    //todo use winkel
    return speed;
}

// return spacing and id of the nearest pedestrian
my_pair VelocityModel::GetSpacing(const Pedestrian * ped1, const Pedestrian * ped2, Point ei) const
{
    Point distp12   = ped2->GetPos() - ped1->GetPos(); // inversed sign
    double Distance = distp12.Norm();
    double l        = 2 * ped1->GetEllipse().GetBmax();
    Point ep12;
    if(Distance >= J_EPS) {
        ep12 = distp12.Normalized();
    } else {
        LOG_WARNING(
            "VelocityModel::GetSPacing() ep12 can not be calculated! Pedestrians are to close "
            "to "
            "each other ({:f})",
            Distance);
        exit(EXIT_FAILURE); //TODO
    }

    double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
    double condition2 =
        ei.Rotate(0, 1).ScalarProduct(ep12); // theta = pi/2. condition2 should <= than l/Distance
    condition2 = (condition2 > 0) ? condition2 : -condition2; // abs

    if((condition1 >= 0) && (condition2 <= l / Distance)) {
        // return a pair <dist, condition1>. Then take the smallest dist. In case of equality the biggest condition1
        return my_pair(distp12.Norm(), ped2->GetUID());
    }
    return my_pair(FLT_MAX, ped2->GetUID());
}
Point VelocityModel::ForceRepPed(const Pedestrian * ped1, const Pedestrian * ped2) const
{
    Point F_rep(0.0, 0.0);
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12   = ped2->GetPos() - ped1->GetPos();
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
        exit(EXIT_FAILURE); //TODO: quick and dirty fix for issue #158
                            // (sometimes sources create peds on the same location)
    }
    Point ei = ped1->GetV().Normalized();
    if(ped1->GetV().NormSquare() < 0.01) {
        ei = ped1->GetV0().Normalized();
    }
    double condition1 = ei.ScalarProduct(ep12);            // < e_i , e_ij > should be positive
    condition1        = (condition1 > 0) ? condition1 : 0; // abs

    R_ij  = -_aPed * exp((l - Distance) / _DPed);
    F_rep = ep12 * R_ij;

    return F_rep;
}

Point VelocityModel::ForceRepRoom(const Pedestrian * ped, const SubRoom * subroom) const
{
    Point f(0., 0.);
    const Point & centroid = subroom->GetCentroid();
    bool inside            = subroom->IsInSubRoom(centroid);
    //first the walls
    for(const auto & wall : subroom->GetAllWalls()) {
        f += ForceRepWall(ped, wall, centroid, inside);
    }

    //then the obstacles
    for(const auto & obst : subroom->GetAllObstacles()) {
        if(obst->Contains(ped->GetPos())) {
            LOG_ERROR(
                "Agent {} is trapped in obstacle in room/subroom {:d}/{:d}",
                ped->GetUID(),
                subroom->GetRoomID(),
                subroom->GetSubRoomID());
            exit(EXIT_FAILURE);
        } else
            for(const auto & wall : obst->GetAllWalls()) {
                f += ForceRepWall(ped, wall, centroid, inside);
            }
    }

    // and finally the closed doors
    for(const auto & trans : subroom->GetAllTransitions()) {
        if(!trans->IsOpen()) {
            f += ForceRepWall(ped, *(static_cast<Line *>(trans)), centroid, inside);
        }
    }

    return f;
}

Point VelocityModel::ForceRepWall(
    const Pedestrian * ped,
    const Line & w,
    const Point & centroid,
    bool inside) const
{
    Point F_wrep = Point(0.0, 0.0);
    Point pt     = w.ShortestPoint(ped->GetPos());

    Point dist       = pt - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
    const double EPS = 0.000;              // molified see Koester2013
    double Distance  = dist.Norm() + EPS;  // distance between the centre of ped and point p
    Point e_iw; // x- and y-coordinate of the normalized vector between ped and pt
    double l = ped->GetEllipse().GetBmax();
    double R_iw;
    double min_distance_to_wall = 0.001; // 10 cm

    if(Distance > min_distance_to_wall) {
        e_iw = dist / Distance;
    } else {
        LOG_WARNING(
            "Velocity: forceRepWall() ped {} [{:f}, {:f}] is too near to the wall [{:f}, "
            "{:f}]-[{:f}, {:f}] (dist={:f})",
            ped->GetUID(),
            ped->GetPos().y,
            ped->GetPos().y,
            w.GetPoint1().x,
            w.GetPoint1().y,
            w.GetPoint2().x,
            w.GetPoint2().y,
            Distance);
        Point new_dist = centroid - ped->GetPos();
        new_dist       = new_dist / new_dist.Norm();
        e_iw           = (inside ? new_dist : new_dist * -1);
    }
    //-------------------------

    const Point & pos   = ped->GetPos();
    const auto distGoal = ped->GetExitLine().DistToSquare(pos);

    if(distGoal < J_EPS_GOAL * J_EPS_GOAL)
        return F_wrep;
    //-------------------------
    R_iw   = -_aWall * exp((l - Distance) / _DWall);
    F_wrep = e_iw * R_iw;

    return F_wrep;
}

std::string VelocityModel::GetDescription() const
{
    std::string rueck;
    char tmp[1024];

    sprintf(tmp, "\t\ta: \t\tPed: %f \tWall: %f\n", _aPed, _aWall);
    rueck.append(tmp);
    sprintf(tmp, "\t\tD: \t\tPed: %f \tWall: %f\n", _DPed, _DWall);
    rueck.append(tmp);
    return rueck;
}
