/**
 * \file        Line.cpp
 * \date        Sep 30, 2010
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
 *
 *
 **/
#include "Line.h"

#include "Wall.h"
#include "general/Logger.h"
#include "general/Macros.h"
#include "math/Mathematics.h"

#include <vector>

int Line::_static_UID = 0;


#define DEBUG 0

/************************************************************
  Konstruktoren
 ************************************************************/
Line::Line()
{
    SetPoint1(Point()); //Default-Constructor  (0.0,0.0)
    SetPoint2(Point());
    _centre = (_point1 + _point2) * 0.5;
    _length = (_point1 - _point2).Norm();
    _uid    = _static_UID++;
}

Line::Line(const Point & p1, const Point & p2, int count) :
    _point1(p1), _point2(p2), _centre((p1 + p2) * 0.5), _length((p1 - p2).Norm())
{
    if(count)
        _uid = _static_UID++;
}

Line::Line(const Point & p1, const Point & p2) :
    _point1(p1), _point2(p2), _centre((p1 + p2) * 0.5), _length((p1 - p2).Norm())
{
    _uid = _static_UID++;
}

int Line::GetUniqueID() const
{
    return _uid;
}


/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Line::SetPoint1(const Point & p)
{
    //if we call this setter and change line, it should get a new UID (ar.graf) @todo:
    _point1 = p;
    _centre = (_point1 + _point2) * 0.5;
    _length = (_point1 - _point2).Norm();
}

void Line::SetPoint2(const Point & p)
{
    //if we call this setter and change line, it should get a new UID (ar.graf) @todo:
    _point2 = p;
    _centre = (_point1 + _point2) * 0.5;
    _length = (_point1 - _point2).Norm();
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/
const Point & Line::GetPoint1() const
{
    return _point1;
}

const Point & Line::GetPoint2() const
{
    return _point2;
}

const Point & Line::GetCentre() const
{
    return _centre;
}

/*************************************************************
 Ausgabe
 ************************************************************/
std::string Line::Write() const
{
    std::string geometry;
    char wall[500] = "";
    geometry.append("\t\t<wall color=\"100\">\n");
    sprintf(
        wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n", (GetPoint1()._x), (GetPoint1()._y));
    geometry.append(wall);
    sprintf(
        wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n", (GetPoint2()._x), (GetPoint2()._y));
    geometry.append(wall);
    geometry.append("\t\t</wall>\n");
    return geometry;
}


/*************************************************************
 Sonstige Funktionen
 ************************************************************/
// Normalen vector zur Linie

Point Line::NormalVec() const
{
    double nx, ny;
    Point r = GetPoint2() - GetPoint1();

    if(r._x == 0.0) {
        nx = 1;
        ny = 0;
    } else {
        double norm;
        nx = -r._y / r._x;
        ny = 1;
        /* Normieren */
        norm = sqrt(nx * nx + ny * ny);
        if(fabs(norm) < J_EPS) {
            LOG_ERROR("Line::NormalVec() norm==0");
            exit(0);
        }
        nx /= norm;
        ny /= norm;
    }
    return Point(nx, ny);
}

// Normale Komponente von v auf l

double Line::NormalComp(const Point & v) const
{
    // Normierte Vectoren
    Point l         = (_point2 - _point1).Normalized();
    const Point & n = NormalVec();

    double alpha;

    if(fabs(l._x) < J_EPS) {
        alpha = v._x / n._x;
    } else if(fabs(l._y) < J_EPS) {
        alpha = v._y / n._y;
    } else {
        alpha = l.CrossProduct(v) / n.CrossProduct(l);
    }

    return fabs(alpha);
}
// Lotfußpunkt zur Geraden Line
// Muss nicht im Segment liegen

Point Line::LotPoint(const Point & p) const
{
    const Point & t = _point1 - _point2;
    Point tmp;
    double lambda;

    tmp     = p - _point2;
    lambda  = tmp.ScalarProduct(t) / t.ScalarProduct(t);
    Point f = _point2 + t * lambda;
    return f;
}

// return true if point the orthogonal projection of p on Line segment is on the
// line segment.
bool Line::isBetween(const Point & p) const
{
    const Point & t = _point1 - _point2;
    double lambda   = (p - _point2).ScalarProduct(t) / t.ScalarProduct(t);
    return (lambda > 0) && (lambda < 1);
}


/* Punkt auf der Linie mit kürzestem Abstand zu p
 * In der Regel Lotfußpunkt, Ist der Lotfußpunkt nicht im Segment
 * wird der entsprechende Eckpunkt der Line genommen
 * */
Point Line::ShortestPoint(const Point & p) const
{
    if(_point1 == _point2)
        return _point1;

    const Point & t = _point1 - _point2;
    double lambda   = (p - _point2).ScalarProduct(t) / t.ScalarProduct(t);
    if(lambda < 0)
        return _point2;
    else if(lambda > 1)
        return _point1;
    else
        return _point2 + t * lambda;
}

/*
 *  Prüft, ob Punkt p im Liniensegment enthalten ist
 * algorithm from:
 * http://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
 *
 * */
bool Line::IsInLineSegment(const Point & p) const
{
    Point differenceTwoAndOne = _point2 - _point1;
    Point differencePAndOne   = p - _point1;

    // cross product to check if point i colinear
    auto crossProduct = differenceTwoAndOne.CrossProduct(differencePAndOne);

    if(std::abs(crossProduct) > 5. * std::numeric_limits<double>::epsilon()) {
        return false;
    }

    // dotproduct and distSquared to check if point is in segment and not just in line
    double dotp = differencePAndOne.ScalarProduct(differenceTwoAndOne);

    if(dotp < 0.) {
        return false;
    }

    return dotp <= (differenceTwoAndOne).NormSquare();
}

bool Line::NearlyInLineSegment(const Point & p) const
{
    return fabs((_point1 - p).Norm() + (_point2 - p).Norm() - (_point2 - _point1).Norm()) <
           J_EPS_DIST; // old version
}

/* Berechnet direkt den Abstand von p zum Segment l
 * dazu wird die Funktion Line::ShortestPoint()
 * benuzt
 * */
double Line::DistTo(const Point & p) const
{
    return (p - ShortestPoint(p)).Norm();
}

double Line::DistToSquare(const Point & p) const
{
    return DistTo(p) * DistTo(p);
}

/* Zwei Linien sind gleich, wenn ihre beiden Punkte
 * gleich sind
 * */
bool Line::operator==(const Line & l) const
{
    return (
        (_point1 == l.GetPoint1() && _point2 == l.GetPoint2()) ||
        (_point2 == l.GetPoint1() && _point1 == l.GetPoint2()));
}

/* Zwei Linien sind ungleich, wenn ihre beiden Punkte
 * ungleich sind.
 * */
bool Line::operator!=(const Line & l) const
{
    return (!(*this == l));
}
// this function is necessary to use std::set and is basically the same as !=
bool Line::operator<(const Line & l) const
{
    return (!(*this == l));
}


double Line::GetLength() const
{
    return _length;
}
double Line::Length() const
{
    return (_point1 - _point2).Norm();
}

double Line::LengthSquare() const
{
    return (_point1 - _point2).NormSquare();
}

//TODO unit  test
bool Line::Overlapp(const Line & l) const
{
    //first check if they are colinear
    Point vecAB = l.GetPoint2() - l.GetPoint1();
    Point vecDC = _point1 - _point2;
    if(fabs(vecAB.Determinant(vecDC)) < J_EPS) {
        if(IsInLineSegment(l.GetPoint1()) && !HasEndPoint(l.GetPoint1())) {
            return true;
        }

        if(IsInLineSegment(l.GetPoint2()) && !HasEndPoint(l.GetPoint2())) {
            return true;
        }
    }
    return false;
}

//from http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
int Line::IntersectionWith(const Point & p1, const Point & p2, Point & p3) const
{
    p3._x   = J_NAN;
    p3._y   = J_NAN;
    Point r = p2 - p1;
    Point s = _point2 - _point1;

    double denom = r.CrossProduct(s);
    double numer = (_point1 - p1).CrossProduct(r);

    if(denom == 0.) {
        if(numer == 0.) {
            if(this->ShareCommonPointWith(Line(p1, p2), p3)) {
                return LineIntersectType::INTERSECTION;
            }
            double t0 = (_point1 - p1).ScalarProduct(r) / r.ScalarProduct(r);
            double t1 = t0 + s.ScalarProduct(r) / r.ScalarProduct(r);
            bool intersec;
            if(s.ScalarProduct(r) < 0) {
                intersec = IntervalsIntersect(t1, t0, 1, 0);
            } else {
                intersec = IntervalsIntersect(t0, t1, 1, 0);
            }
            if(intersec) {
                return LineIntersectType::OVERLAP;
            } else {
                return LineIntersectType::NO_INTERSECTION; //colinear and disjoint
            }
        } else { // the lines are just parallel and do not share a common poin
            return LineIntersectType::NO_INTERSECTION;
        }
    }
    double t = (_point1 - p1).CrossProduct(s) / (r.CrossProduct(s));
    double u = (_point1 - p1).CrossProduct(r) / (r.CrossProduct(s));


    if(-0.05 > t || t > 1) {
        return LineIntersectType::NO_INTERSECTION;
    }

    if(0 > u || u > 1) {
        return LineIntersectType::NO_INTERSECTION;
    }

    p3 = p1 + (r * t);

    return LineIntersectType::INTERSECTION;
}

int Line::IntersectionWith(const Line & L, Point & p3) const
{
    return IntersectionWith(L._point1, L._point2, p3);
}

int Line::IntersectionWith(const Point & p1, const Point & p2) const
{
    Point dummy;
    return IntersectionWith(p1, p2, dummy);
}

int Line::IntersectionWith(const Line & l) const
{
    Point dummy;
    return IntersectionWith(l._point1, l._point2, dummy);
}

Line Line::Enlarge(double d) const
{
    Point diff = (_point1 - _point2).Normalized() * d;
    return Line(_point1 + diff, _point2 - diff);
}

bool Line::IsHorizontal()
{
    return fabs(_point1._y - _point2._y) <= J_EPS;
}

bool Line::IsVertical()
{
    return fabs(_point1._x - _point2._x) <= J_EPS;
}

int Line::WichSide(const Point & pt)
{
    if(IsLeft(pt))
        return 0;
    return 1;
}

bool Line::ShareCommonPointWith(const Line & line, Point & P) const
{
    if(line.GetPoint1() == _point1 || line.GetPoint2() == _point1) {
        P = _point1;
        return true;
    } else if(line.GetPoint1() == _point2 || line.GetPoint2() == _point2) {
        P = _point2;
        return true;
    } else
        return false;
}

bool Line::ShareCommonPointWith(const Line & line) const
{
    Point dummy;
    return ShareCommonPointWith(line, dummy);
}

bool Line::HasEndPoint(const Point & point) const
{
    if(_point1 == point)
        return true;
    return _point2 == point;
}

bool Line::NearlyHasEndPoint(const Point & point) const
{
    if((_point1 - point).Norm() <= J_EPS_DIST)
        return true;
    return ((_point2 - point).Norm() <= J_EPS_DIST);
}


bool Line::IntersectionWithCircle(const Point & centre, double radius /*cm for pedestrians*/)
{
    //this formula assumes that the circle is centered the origin.
    // so we translate the complete stuff such that the circle ends up at the origin
    Point p1 = _point1 - centre;
    Point p2 = _point2 - centre;
    //xc=xc-xc;yc=yc-yc; to make it perfect

    // we first check the intersection of the circle and the  infinite line defined by the segment
    double dr2 = (p2 - p1).ScalarProduct(p2 - p1);
    double D2  = p1.CrossProduct(p2) * p1.CrossProduct(p2);
    double r2  = radius * radius;

    double delta = r2 * dr2 - D2;
    if(delta <= 0.0)
        return false;

    double a = dr2;

    double b = 2 * 2 * (p2 - p1).ScalarProduct(p1);
    ;
    double c = p1.ScalarProduct(p1) - radius * radius;

    delta = b * b - 4 * a * c;

    if(p1 == p2) {
        return false;
    }
    if(delta < 0.0) {
        LOG_ERROR("Line::IntersectionWithCircle does not support delta < 0. delta = {}", delta);
        return false;
    }

    double t1 = (-b + sqrt(delta)) / (2 * a);
    double t2 = (-b - sqrt(delta)) / (2 * a);
    if((t1 < 0.0) || (t1 > 1.0))
        return false;
    return !((t2 < 0.0) || (t2 > 1.0));
}

//TODO: Consider numerical stability and special case pt is on line
// Returns true if pt is on the left side ( from point1 toward point2)
bool Line::IsLeft(const Point & pt)
{
    double test = (_point2._x - _point1._x) * (pt._y - _point1._y) -
                  (_point2._y - _point1._y) * (pt._x - _point1._x);
    return test > 0.0;
}

const Point & Line::GetLeft(const Point & pt)
{
    if(IsLeft(pt)) {
        return _point2;
    } else {
        return _point1;
    }
}

const Point & Line::GetRight(const Point & pt)
{
    if(!IsLeft(pt)) {
        return _point2;
    } else {
        return _point1;
    }
}

std::string Line::toString() const
{
    std::stringstream tmp;
    tmp << _point1.toString() << "--" << _point2.toString();
    return tmp.str();
}

// get distance between first point of line with the intersection point.
//if no intersection return infinity
// this function is exactly the same as GetIntersection(), but returns the distance squared
//insteed of a boolian
double Line::GetDistanceToIntersectionPoint(const Line & l) const
{
    Point PointF;
    int intersection = this->IntersectionWith(l, PointF);
    if(intersection == LineIntersectType::NO_INTERSECTION ||
       intersection == LineIntersectType::OVERLAP) {
        return std::numeric_limits<double>::infinity();
    }
    if(!IsInLineSegment(PointF)) //is point on the line?
        return std::numeric_limits<double>::infinity();
    double dist = (_point1 - PointF).NormSquare();

    LOG_DEBUG(
        "GetDistanceToIntersectionPoint: {} intersects with {} in point {} with distance {}",
        l.toString(),
        toString(),
        PointF.toString(),
        sqrt(dist));

    return dist;
}

// calculates the angles QPF and QPL
// return the angle of the point (F or L) which is nearer to the Goal
//the calling line: P->Q, Q is the crossing point
//
//                 o P
//                /
//               /
//   F          /              L
//   o --------x---------------o
//            / Q
//           /
//          o Goal

double Line::GetDeviationAngle(const Line & l) const
{
    Point P    = _point1;
    Point Goal = _point2;

    Point L = l._point1;
    Point R = l._point2;

    double dist_Goal_L = (Goal - L).NormSquare();
    double dist_Goal_R = (Goal - R).NormSquare();

    double angle, angleL, angleR;
    // we don't need to calculate both angles, but for debugging purposes we do it.
    angleL = atan((Goal - P).CrossProduct(L - P) / (Goal - P).ScalarProduct(L - P));
    angleR = atan((Goal - P).CrossProduct(R - P) / (Goal - P).ScalarProduct(R - P));

    angle = (dist_Goal_L < dist_Goal_R) ? angleL : angleR;
    return angle;
}

// return the biggest angle between two lanes
double Line::GetAngle(const Line & l) const
{
    Point P    = _point1;
    Point Goal = _point2;

    Point L = l._point1;
    Point R = l._point2;

    double angleL, angleR;
    // we don't need to calculate both angles, but for debugging purposes we do it.
    angleL = atan((Goal - P).CrossProduct(L - P) / (Goal - P).ScalarProduct(L - P));
    angleR = atan((Goal - P).CrossProduct(R - P) / (Goal - P).ScalarProduct(R - P));

    return (fabs(angleL) > fabs(angleR)) ? angleL : angleR;
}

// get the smallest angle that ensures a safe deviation from an
// obstacle. Safe means that the rotated line ped--->goal do not
// intersect with any line of the obstacle.
// ALgorithm:
// =========
// calculate angle to the left and angle to the right
// choose the angle which is visible.
// If both are visible then choose the one which:
// 1. leads to larger distance to room walls (we don't
//    want to avoid the obstacle to crash in a wall, do
//    we?)
// 2. in case of equality choose the smallest angle. If
//    they are nearly equal that opt for the right.
double Line::GetObstacleDeviationAngle(
    const std::vector<Wall> & owalls,
    const std::vector<Wall> & rwalls) const
{
    Point P    = _point1;
    Point Goal = _point2;
    Point GL, GR;
    Point L, R;

    double minAngle = std::numeric_limits<double>::infinity(), angleL, angleR, angle;

    Line l_large, tmpDirectionL, tmpDirectionR;

    bool visibleL = true, visibleR = true;

    double distToRoomL = 20001, distToRoomR = 2001;
    double minDistToRoomL = 20001, minDistToRoomR = 2001;

    //     for (unsigned int i = 0; i < owalls.size(); i++) {
    // l = owalls[i];
    for(auto l : owalls) {
        minDistToRoomL = std::numeric_limits<double>::infinity(),
        minDistToRoomR = std::numeric_limits<double>::infinity();
        visibleL       = true;
        visibleR       = true;

        l_large = l.Enlarge(3); // 2*ped->GetLargerAxis()

        L = l_large._point1;
        R = l_large._point2;

        angleL = atan((Goal - P).CrossProduct(L - P) / (Goal - P).ScalarProduct(L - P));
        angleR = atan((Goal - P).CrossProduct(R - P) / (Goal - P).ScalarProduct(R - P));

        GL = (Goal - P).Rotate(cos(angleL), sin(angleL)) + P;
        GR = (Goal - P).Rotate(cos(angleR), sin(angleR)) + P;

        tmpDirectionL = Line(P, GL);
        tmpDirectionR = Line(P, GR);

        for(auto l_other : owalls) {
            if(l_other == l)
                continue;

            if(tmpDirectionL.IntersectionWith(l_other)) {
                visibleL = false;
                break;
            }
        }

        for(auto l_other : owalls) {
            if(l_other == l)
                continue;

            if(tmpDirectionR.IntersectionWith(l_other)) {
                visibleR = false;
                break;
            }
        }

        if(visibleR && visibleL) { // both angles are OK. Get
            // smallest deviation.
            //----------------------- check the subroom walls
            for(unsigned int i = 0; i < rwalls.size(); i++) {
                distToRoomL = tmpDirectionL.GetDistanceToIntersectionPoint(rwalls[i]);

                distToRoomR = tmpDirectionR.GetDistanceToIntersectionPoint(rwalls[i]);

                if(distToRoomL < minDistToRoomL)
                    minDistToRoomL = distToRoomL;

                if(distToRoomR < minDistToRoomR)
                    minDistToRoomR = distToRoomR;
            } //for roome walls
            //-----------------------------------------------
            if(minDistToRoomR > minDistToRoomL)
                angle = angleR;
            else if(minDistToRoomL > minDistToRoomR)
                angle = angleL;
            // both distance equal, prefer right
            else if(almostEqual(angleR, angleL, 0.001))
                angle = angleR;
            else // distances equal, but angles not. Take smallest
                angle = (fabs(angleL) < fabs(angleR)) ? angleL : angleR;
        } //both are visible
        else if(visibleR && !visibleL) {
            angle = angleR;
        } else if(!visibleR && visibleL) {
            angle = angleL;
        } else {
            continue; // both angles are not OK. check next wall
        }
        if(fabs(angle) < fabs(minAngle))
            minAngle = angle;
    } // owalls


    if(minAngle == std::numeric_limits<double>::infinity()) {
        printf("WARNING:  minAngle ist infinity\n");
        getc(stdin);
    }
    return minAngle;
}
