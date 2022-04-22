/**
 * \file        GCFMModel.cpp
 * \date        Apr 15, 2014
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
 * Implementation of classes for force-based models.
 * Actually we've got two different models:
 * 1. Generalized Centrifugal Force Model
 *
 *
 **/
#include "GCFMModel.hpp"

#include "Simulation.hpp"
#include "direction/DirectionManager.hpp"
#include "direction/walking/DirectionStrategy.hpp"
#include "geometry/SubRoom.hpp"
#include "geometry/Wall.hpp"
#include "math/OperationalModel.hpp"
#include "neighborhood/NeighborhoodSearch.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <Logger.hpp>

GCFMModel::GCFMModel(
    DirectionManager* directionManager,
    double nuped,
    double nuwall,
    double dist_effPed,
    double dist_effWall,
    double intp_widthped,
    double intp_widthwall,
    double maxfped,
    double maxfwall)
    : OperationalModel(directionManager)
    , _nuPed(nuped)
    , _nuWall(nuwall)
    , _intp_widthPed(intp_widthped)
    , _intp_widthWall(intp_widthwall)
    , _maxfPed(maxfped)
    , _maxfWall(maxfwall)
    , _distEffMaxPed(dist_effPed)
    , _distEffMaxWall(dist_effWall)
{
}

PedestrianUpdate GCFMModel::ComputeNewPosition(
    double dT,
    const Pedestrian& ped,
    const Geometry& geometry,
    const NeighborhoodSearch& neighborhoodSearch) const
{
    const double delta = 1.5;
    const double normVi = ped.GetV().ScalarProduct(ped.GetV());
    const double tmp = (ped.GetV0Norm() + delta) * (ped.GetV0Norm() + delta);
    if(normVi > tmp && ped.GetV0Norm() > 0) {
        LOG_ERROR(
            "GCFMModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
            "is bigger than desired velocity (%f)\n",
            sqrt(normVi),
            ped.GetUID(),
            ped.GetV0Norm());
    }

    const auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.GetPos(), 4);
    const auto p1 = ped.GetPos();
    Point F_rep;
    for(const auto* other : neighborhood) {
        if(other->GetUID() == ped.GetUID()) {
            continue;
        }
        if(!geometry.IntersectsAny(Line(p1, other->GetPos()))) {
            F_rep += ForceRepPed(&ped, other);
        }
    }

    PedestrianUpdate update{};
    // repulsive forces to the walls and transitions that are not my target
    Point repwall = ForceRepRoom(&ped, geometry);
    Point fd = ForceDriv(&ped, _direction->GetTarget(&ped), update);
    Point acc = (fd + F_rep + repwall) / ped.GetMass();

    update.velocity = ped.GetV() + acc * dT;
    update.position = ped.GetPos() + *update.velocity * dT;
    return update;
}

void GCFMModel::ApplyUpdate(const PedestrianUpdate& update, Pedestrian& agent) const
{
    agent.SetV0(update.v0);
    agent.IncrementOrientationDelay();
    if(update.lastE0) {
        agent.SetLastE0(*update.lastE0);
    }
    if(update.position) {
        agent.SetPos(*update.position);
    }
    if(update.velocity) {
        agent.SetV(*update.velocity);
    }
    agent.SetPhiPed();
}

inline Point
GCFMModel::ForceDriv(const Pedestrian* ped, Point target, PedestrianUpdate& update) const
{
    if(ped->IsWaiting()) {
        update.waitingPos = target;
    }
    Point F_driv;
    const Point& pos = ped->GetPos();
    const double dist = ped->GetExitLine().DistTo(pos);
    const Point lastE0 = ped->GetLastE0();
    update.lastE0 = target - pos;

    // TODO(kkratz) fix this hack
    const auto* strategy = &_direction->GetDirectionStrategy();
    if(dynamic_cast<const DirectionLocalFloorfield*>(strategy)) {
        if(dist > 50 * J_EPS_GOAL) {
            const Point v0 = ped->GetV0(target);
            update.v0 = v0;
            F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
        } else {
            F_driv = ((lastE0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
            update.lastE0 = lastE0;
        }
    } else if(dist > J_EPS_GOAL) {
        const Point v0 = ped->GetV0(target);
        update.v0 = v0;
        F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
    } else {
        const Point v0 = ped->GetV0();
        F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
    }
    return F_driv;
}

Point GCFMModel::ForceRepPed(const Pedestrian* ped1, const Pedestrian* ped2) const
{
    Point F_rep;
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = ped2->GetPos() - ped1->GetPos();
    const Point& vp1 = ped1->GetV(); // v Ped1
    const Point& vp2 = ped2->GetV(); // v Ped2
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double tmp, tmp2;
    double v_ij;
    double K_ij;
    double nom; // nominator of Frep
    double px; // hermite Interpolation value
    const JEllipse& E1 = ped1->GetEllipse();
    const JEllipse& E2 = ped2->GetEllipse();
    double distsq;
    double dist_eff = E1.EffectiveDistanceToEllipse(E2, &distsq);

    //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //       ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    // If the pedestrian is outside the cutoff distance, the force is zero.
    if(dist_eff >= _distEffMaxPed) {
        F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    //%------- Free parameter --------------
    Point p1, p2; // "Normale" Koordinaten
    double mindist;

    p1 = Point(E1.GetXp(), 0)
             .TransformToCartesianCoordinates(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
    p2 = Point(E2.GetXp(), 0)
             .TransformToCartesianCoordinates(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
    distp12 = p2 - p1;
    mindist = 0.5; // for performance reasons, it is assumed that this distance is about 50 cm
    double dist_intpol_left = mindist + _intp_widthPed; // lower cut-off for Frep (modCFM)
    double dist_intpol_right = _distEffMaxPed - _intp_widthPed; // upper cut-off for Frep (modCFM)
    double smax = mindist - _intp_widthPed; // max overlapping
    double f = 0.0f, f1 = 0.0f; // function value and its derivative at the interpolation point'

    // todo: runtime normsquare?
    if(distp12.Norm() >= J_EPS) {
        ep12 = distp12.Normalized();

    } else {
        LOG_WARNING(
            "Distance between two pedestrians is small ({}<{}). Force can not be calculated.",
            distp12.Norm(),
            J_EPS);
        return F_rep; // Parameter values are not chosen wisely --> unrealistic overlaping ...
                      // ignore.
    }
    // calculate the parameter (whatever dist is)
    tmp = (vp1 - vp2).ScalarProduct(ep12); // < v_ij , e_ij >
    v_ij = 0.5 * (tmp + fabs(tmp));
    tmp2 = vp1.ScalarProduct(ep12); // < v_i , e_ij >

    // todo: runtime normsquare?
    if(vp1.Norm() < J_EPS) { // if(norm(v_i)==0)
        K_ij = 0;
    } else {
        double bla = tmp2 + fabs(tmp2);
        K_ij = 0.25 * bla * bla / vp1.ScalarProduct(vp1); // squared

        if(K_ij < J_EPS * J_EPS) {
            F_rep = Point(0.0, 0.0);
            return F_rep;
        }
    }
    nom = _nuPed * ped1->GetV0Norm() + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
    nom *= nom;

    K_ij = sqrt(K_ij);
    if(dist_eff <= smax) { // 5
        f = -ped1->GetMass() * K_ij * nom / dist_intpol_left;
        F_rep = ep12 * _maxfPed * f;
        return F_rep;
    }

    //          smax    dist_intpol_left           dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //           5   |     4       |            3             |      2       | 1

    if(dist_eff >= dist_intpol_right) { // 2
        f = -ped1->GetMass() * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
        f1 = -f / dist_intpol_right;
        px = hermite_interp(dist_eff, dist_intpol_right, _distEffMaxPed, f, 0, f1, 0);
        F_rep = ep12 * px;
    } else if(dist_eff >= dist_intpol_left) { // 3
        f = -ped1->GetMass() * K_ij * nom / fabs(dist_eff); // abs(NR-Dv(i)+Sa)
        F_rep = ep12 * f;
    } else { // 4
        f = -ped1->GetMass() * K_ij * nom / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(dist_eff, smax, dist_intpol_left, _maxfPed * f, f, 0, f1);
        F_rep = ep12 * px;
    }
    if(F_rep.x != F_rep.x || F_rep.y != F_rep.y) {
        LOG_ERROR(
            "NAN return p1{} p2 {} Frepx={:f} Frepy={:f} K_ij={:f}",
            ped1->GetUID(),
            ped2->GetUID(),
            F_rep.x,
            F_rep.y,
            K_ij);
    }
    return F_rep;
}

/* abstoßende Kraft zwischen ped und subroom
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - subroom: SubRoom für den alle abstoßende Kräfte von Wänden berechnet werden
 * Rückgabewerte:
 *   - Vektor(x,y) mit Summe aller abstoßenden Kräfte im SubRoom
 * */

inline Point GCFMModel::ForceRepRoom(const Pedestrian* ped, const Geometry& geometry) const
{
    auto walls = geometry.LineSegmentsInDistanceTo(5.0, ped->GetPos());

    auto f = std::accumulate(
        walls.begin(),
        walls.end(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + ForceRepWall(ped, element);
        });

    auto doors = geometry.DoorsInDistanceTo(5.0, ped->GetPos());

    f = std::accumulate(
        doors.begin(), doors.end(), f, [this, &ped](const auto& acc, const auto& door) {
            switch(door.state) {
                case DoorState::OPEN:
                    return acc + Point(0, 0);
                case DoorState::CLOSE:
                case DoorState::TEMP_CLOSE:
                    return acc + ForceRepWall(ped, door.linesegment);
            }
        });
    return f;
}

inline Point GCFMModel::ForceRepWall(const Pedestrian* ped, const Line& w) const
{
    Point F = Point(0.0, 0.0);
    Point pt = w.ShortestPoint(ped->GetPos());
    double wlen = w.LengthSquare();

    if(wlen < 0.01) { // ignore walls smaller than 10 cm
        return F;
    }
    // Kraft soll nur orthgonal wirken
    // ???
    if(fabs((w.GetPoint1() - w.GetPoint2()).ScalarProduct(ped->GetPos() - pt)) > J_EPS) {
        return F;
    }
    // double mind = ped->GetEllipse().MinimumDistanceToLine(w);
    double mind = 0.5; // for performance reasons this distance is assumed to be constant
    double vn = w.NormalComp(ped->GetV()); // normal component of the velocity on the wall
    F = ForceRepStatPoint(ped, pt, mind, vn);

    return F; // line --> l != 0
}

/* abstoßende Punktkraft zwischen ped und Punkt p
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - p: Punkt von dem die Kaft wirkt
 *   - l: Parameter zur Käfteinterpolation
 *   - vn: Parameter zur Käfteinterpolation
 * Rückgabewerte:
 *   - Vektor(x,y) mit abstoßender Kraft
 * */
// TODO: use effective DistanceToEllipse and simplify this function.
Point GCFMModel::ForceRepStatPoint(const Pedestrian* ped, const Point& p, double l, double vn) const
{
    Point F_rep = Point(0.0, 0.0);
    const Point& v = ped->GetV();
    Point dist = p - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
    double d = dist.Norm(); // distance between the centre of ped and point p
    Point e_ij; // x- and y-coordinate of the normalized vector between ped and p

    double tmp;
    double bla;
    Point r;
    Point pinE; // vorher x1, y1
    const JEllipse& E = ped->GetEllipse();

    if(d < J_EPS)
        return Point(0.0, 0.0);
    e_ij = dist / d;
    tmp = v.ScalarProduct(e_ij); // < v_i , e_ij >;
    bla = (tmp + fabs(tmp));
    if(!bla) // Fussgaenger nicht im Sichtfeld
        return Point(0.0, 0.0);
    if(fabs(v.x) < J_EPS && fabs(v.y) < J_EPS) // v==0)
        return Point(0.0, 0.0);
    double K_ij;
    K_ij = 0.5 * bla / v.Norm(); // K_ij
    // Punkt auf der Ellipse
    pinE = p.TransformToEllipseCoordinates(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
    // Punkt auf der Ellipse
    r = E.PointOnEllipse(pinE);
    // interpolierte Kraft
    F_rep = ForceInterpolation(ped->GetV0Norm(), K_ij, e_ij, vn, d, (r - E.GetCenter()).Norm(), l);
    return F_rep;
}

Point GCFMModel::ForceInterpolation(
    double v0,
    double K_ij,
    const Point& e,
    double vn,
    double d,
    double r,
    double l) const
{
    Point F_rep;
    double nominator = _nuWall * v0 + vn;
    nominator *= nominator * K_ij;
    double f = 0, f1 = 0; // function value and its derivative at the interpolation point
    double smax = l - _intp_widthWall; // max overlapping radius
    double dist_intpol_left = l + _intp_widthWall; // r_eps
    double dist_intpol_right = _distEffMaxWall - _intp_widthWall;

    double dist_eff = d - r;

    //         smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    double px = 0; // value of the interpolated function
    double tmp1 = _distEffMaxWall;
    double tmp2 = dist_intpol_right;
    double tmp3 = dist_intpol_left;
    double tmp5 = smax + r;

    if(dist_eff >= tmp1) { // 1
        // F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    if(dist_eff <= tmp5) { // 5
        F_rep = e * (-_maxfWall);
        return F_rep;
    }

    if(dist_eff > tmp2) { // 2
        f = -nominator / dist_intpol_right;
        f1 = -f / dist_intpol_right; // nominator / (dist_intpol_right^2) = derivativ of f
        px = hermite_interp(dist_eff, dist_intpol_right, _distEffMaxWall, f, 0, f1, 0);
        F_rep = e * px;
    } else if(dist_eff >= tmp3) { // 3
        f = -nominator / fabs(dist_eff); // devided by abs f the effective distance
        F_rep = e * f;
    } else { // 4 d > smax FIXME
        f = -nominator / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(dist_eff, smax, dist_intpol_left, _maxfWall * f, f, 0, f1);
        F_rep = e * px;
    }
    return F_rep;
}
