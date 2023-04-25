/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Line.hpp"

#include "Mathematics.hpp"
#include <Macros.hpp>

#include <Logger.hpp>
#include <vector>

int Line::_static_UID = 0;

#define DEBUG 0

/************************************************************
  Konstruktoren
 ************************************************************/
Line::Line()
{
    SetPoint1(Point()); // Default-Constructor  (0.0,0.0)
    SetPoint2(Point());
    _centre = (_point1 + _point2) * 0.5;
    _length = (_point1 - _point2).Norm();
    _uid = _static_UID++;
}

Line::Line(const Point& p1, const Point& p2, int count)
    : _point1(p1), _point2(p2), _centre((p1 + p2) * 0.5), _length((p1 - p2).Norm())
{
    if(count)
        _uid = _static_UID++;
}

Line::Line(const Point& p1, const Point& p2)
    : _point1(p1), _point2(p2), _centre((p1 + p2) * 0.5), _length((p1 - p2).Norm())
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
void Line::SetPoint1(const Point& p)
{
    // if we call this setter and change line, it should get a new UID (ar.graf) @todo:
    _point1 = p;
    _centre = (_point1 + _point2) * 0.5;
    _length = (_point1 - _point2).Norm();
}

void Line::SetPoint2(const Point& p)
{
    // if we call this setter and change line, it should get a new UID (ar.graf) @todo:
    _point2 = p;
    _centre = (_point1 + _point2) * 0.5;
    _length = (_point1 - _point2).Norm();
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/
const Point& Line::GetPoint1() const
{
    return _point1;
}

const Point& Line::GetPoint2() const
{
    return _point2;
}

const Point& Line::GetCentre() const
{
    return _centre;
}

/*************************************************************
 Sonstige Funktionen
 ************************************************************/
// Normalen vector zur Linie

Point Line::NormalVec() const
{
    const Point r = (GetPoint2() - GetPoint1());
    return Point(-r.y, r.x).Normalized();
}

// Normale Komponente von v auf l

double Line::NormalComp(const Point& v) const
{
    // Normierte Vectoren
    Point l = (_point2 - _point1).Normalized();
    const Point& n = NormalVec();

    double alpha;

    if(fabs(l.x) < J_EPS) {
        alpha = v.x / n.x;
    } else if(fabs(l.y) < J_EPS) {
        alpha = v.y / n.y;
    } else {
        alpha = l.CrossProduct(v) / n.CrossProduct(l);
    }

    return fabs(alpha);
}
// Lotfußpunkt zur Geraden Line
// Muss nicht im Segment liegen

Point Line::LotPoint(const Point& p) const
{
    const Point& t = _point1 - _point2;
    Point tmp;
    double lambda;

    tmp = p - _point2;
    lambda = tmp.ScalarProduct(t) / t.ScalarProduct(t);
    Point f = _point2 + t * lambda;
    return f;
}

// return true if point the orthogonal projection of p on Line segment is on the
// line segment.
bool Line::isBetween(const Point& p) const
{
    const Point& t = _point1 - _point2;
    double lambda = (p - _point2).ScalarProduct(t) / t.ScalarProduct(t);
    return (lambda > 0) && (lambda < 1);
}

/* Punkt auf der Linie mit kürzestem Abstand zu p
 * In der Regel Lotfußpunkt, Ist der Lotfußpunkt nicht im Segment
 * wird der entsprechende Eckpunkt der Line genommen
 * */
Point Line::ShortestPoint(const Point& p) const
{
    if(_point1 == _point2)
        return _point1;

    const Point& t = _point1 - _point2;
    double lambda = (p - _point2).ScalarProduct(t) / t.ScalarProduct(t);
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
bool Line::IsInLineSegment(const Point& p) const
{
    Point differenceTwoAndOne = _point2 - _point1;
    Point differencePAndOne = p - _point1;

    // cross product to check if point i colinear
    auto crossProduct = differenceTwoAndOne.CrossProduct(differencePAndOne);

    if(std::abs(crossProduct) > 0.0001) {
        return false;
    }

    // dotproduct and distSquared to check if point is in segment and not just in line
    double dotp = differencePAndOne.ScalarProduct(differenceTwoAndOne);

    if(dotp < 0.) {
        return false;
    }

    return dotp <= (differenceTwoAndOne).NormSquare();
}

bool Line::NearlyInLineSegment(const Point& p) const
{
    return fabs((_point1 - p).Norm() + (_point2 - p).Norm() - (_point2 - _point1).Norm()) <
           J_EPS_DIST; // old version
}

/* Berechnet direkt den Abstand von p zum Segment l
 * dazu wird die Funktion Line::ShortestPoint()
 * benuzt
 * */
double Line::DistTo(const Point& p) const
{
    return (p - ShortestPoint(p)).Norm();
}

double Line::DistToSquare(const Point& p) const
{
    return DistTo(p) * DistTo(p);
}

/* Zwei Linien sind gleich, wenn ihre beiden Punkte
 * gleich sind
 * */
bool Line::operator==(const Line& l) const
{
    return (
        (_point1 == l.GetPoint1() && _point2 == l.GetPoint2()) ||
        (_point2 == l.GetPoint1() && _point1 == l.GetPoint2()));
}

/* Zwei Linien sind ungleich, wenn ihre beiden Punkte
 * ungleich sind.
 * */
bool Line::operator!=(const Line& l) const
{
    return (!(*this == l));
}
// this function is necessary to use std::set and is basically the same as !=
bool Line::operator<(const Line& l) const
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

// TODO unit  test
bool Line::Overlapp(const Line& l) const
{
    // first check if they are colinear
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

// from
// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
int Line::IntersectionWith(const Point& p1, const Point& p2, Point& p3) const
{
    boost::geometry::model::segment<Point> line(_point1, _point2);
    boost::geometry::model::segment<Point> foo(p1, p2);
    std::vector<Point> output;
    boost::geometry::intersection(line, foo, output);
    switch(output.size()) {
        case 0:
            p3 = Point(J_NAN, J_NAN);
            return LineIntersectType::NO_INTERSECTION;
        case 1:
            p3 = output[0];
            return LineIntersectType::INTERSECTION;
        case 2:
            p3 = Point(J_NAN, J_NAN);
            return LineIntersectType::OVERLAP;
    }
    /// This should never be reached
    std::abort();
}

int Line::IntersectionWith(const Line& L, Point& p3) const
{
    return IntersectionWith(L._point1, L._point2, p3);
}

int Line::IntersectionWith(const Point& p1, const Point& p2) const
{
    Point dummy;
    return IntersectionWith(p1, p2, dummy);
}

int Line::IntersectionWith(const Line& l) const
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
    return fabs(_point1.y - _point2.y) <= J_EPS;
}

bool Line::IsVertical()
{
    return fabs(_point1.x - _point2.x) <= J_EPS;
}

int Line::WichSide(const Point& pt)
{
    if(IsLeft(pt))
        return 0;
    return 1;
}

bool Line::ShareCommonPointWith(const Line& line, Point& P) const
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

bool Line::ShareCommonPointWith(const Line& line) const
{
    Point dummy;
    return ShareCommonPointWith(line, dummy);
}

bool Line::HasEndPoint(const Point& point) const
{
    if(_point1 == point)
        return true;
    return _point2 == point;
}

bool Line::NearlyHasEndPoint(const Point& point) const
{
    if((_point1 - point).Norm() <= J_EPS_DIST)
        return true;
    return ((_point2 - point).Norm() <= J_EPS_DIST);
}

bool Line::IntersectionWithCircle(const Point& centre, double radius /*cm for pedestrians*/) const
{
    // this formula assumes that the circle is centered the origin.
    //  so we translate the complete stuff such that the circle ends up at the origin
    Point p1 = _point1 - centre;
    Point p2 = _point2 - centre;
    // xc=xc-xc;yc=yc-yc; to make it perfect

    // we first check the intersection of the circle and the  infinite line defined by the segment
    double dr2 = (p2 - p1).ScalarProduct(p2 - p1);
    double D2 = p1.CrossProduct(p2) * p1.CrossProduct(p2);
    double r2 = radius * radius;

    double delta = r2 * dr2 - D2;
    if(delta <= 0.0)
        return false;

    double a = dr2;

    double b = 2 * 2 * (p2 - p1).ScalarProduct(p1);

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

std::vector<Point> Line::IntersectionPointsWithCircle(const Point& centre, double radius) const
{
    std::vector<Point> intersectionPoints;

    // shift everything, such that the centre of the circle is at the origin
    Point p1 = _point1 - centre;
    Point p2 = _point2 - centre;

    // x1/y1 : coordinates of point 1 of line segment
    // x2/y2 : coordinates of point 2 of line segment
    // circle: x^2 + y^2 = radius^2
    // line:   x = t*x1 + (1-t)*x2    and y = t*y1 + (1-t)*y2
    // substitute  x and y into circle equation and solve for t yields:
    // [(x1-x2)^2 + (y1-y2)^2] t^2 + 2 * [x2 (x1-x2) + y2 (y1-y2)] t + x2^2 +y2^2 - radius ^2 = 0
    // solve quadratic equation: a * t^2 + t * x + c = 0
    double a = std::pow(p1.x - p2.x, 2.) + std::pow(p1.y - p2.y, 2.);
    double b = 2. * (p2.x * (p1.x - p2.x) + p2.y * (p1.y - p2.y));
    double c = std::pow(p2.x, 2.) + std::pow(p2.y, 2.) - std::pow(radius, 2.);
    double discriminant = std::pow(b, 2.) - 4 * a * c;

    // no real intersection points of line segment and circle exist
    if(discriminant < 0) {
        return intersectionPoints;
    }

    // only one intersection exists
    if(std::fabs(discriminant) < J_EPS) {
        double t = -b / (2. * a);
        double x = t * p1.x + (1. - t) * p2.x;
        double y = t * p1.y + (1. - t) * p2.y;

        Point intersection{x, y};
        intersection += centre;
        intersectionPoints.emplace_back(intersection);
        return intersectionPoints;
    }

    // Check if first intersection is in line segment, if so add to intersection points
    if(double t = (-b + std::sqrt(std::pow(b, 2.) - 4. * a * c)) / (2. * a);
       (t >= 0.) && (t <= 1.)) {
        double x = t * p1.x + (1. - t) * p2.x;
        double y = t * p1.y + (1. - t) * p2.y;

        Point intersection{x, y};
        intersection += centre;
        intersectionPoints.emplace_back(intersection);
    }

    // Check if second intersection is in line segment, if so add to intersection points
    if(double t = (-b - std::sqrt(std::pow(b, 2.) - 4. * a * c)) / (2. * a);
       (t >= 0.) && (t <= 1.)) {
        double x = t * p1.x + (1. - t) * p2.x;
        double y = t * p1.y + (1. - t) * p2.y;

        Point intersection{x, y};
        intersection += centre;
        intersectionPoints.emplace_back(intersection);
    }

    return intersectionPoints;
}

// TODO: Consider numerical stability and special case pt is on line
//  Returns true if pt is on the left side ( from point1 toward point2)
bool Line::IsLeft(const Point& pt)
{
    double test =
        (_point2.x - _point1.x) * (pt.y - _point1.y) - (_point2.y - _point1.y) * (pt.x - _point1.x);
    return test > 0.0;
}

const Point& Line::GetLeft(const Point& pt)
{
    if(IsLeft(pt)) {
        return _point2;
    } else {
        return _point1;
    }
}

const Point& Line::GetRight(const Point& pt)
{
    if(!IsLeft(pt)) {
        return _point2;
    } else {
        return _point1;
    }
}

// get distance between first point of line with the intersection point.
// if no intersection return infinity
// this function is exactly the same as GetIntersection(), but returns the distance squared
// insteed of a boolian
double Line::GetDistanceToIntersectionPoint(const Line& l) const
{
    Point PointF;
    int intersection = this->IntersectionWith(l, PointF);
    if(intersection == LineIntersectType::NO_INTERSECTION ||
       intersection == LineIntersectType::OVERLAP) {
        return std::numeric_limits<double>::infinity();
    }
    if(!IsInLineSegment(PointF)) // is point on the line?
        return std::numeric_limits<double>::infinity();
    double dist = (_point1 - PointF).NormSquare();
    return dist;
}

// calculates the angles QPF and QPL
// return the angle of the point (F or L) which is nearer to the Goal
// the calling line: P->Q, Q is the crossing point
//
//                 o P
//                /
//               /
//   F          /              L
//   o --------x---------------o
//            / Q
//           /
//          o Goal

double Line::GetDeviationAngle(const Line& l) const
{
    Point P = _point1;
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
double Line::GetAngle(const Line& l) const
{
    Point P = _point1;
    Point Goal = _point2;

    Point L = l._point1;
    Point R = l._point2;

    double angleL, angleR;
    // we don't need to calculate both angles, but for debugging purposes we do it.
    angleL = atan((Goal - P).CrossProduct(L - P) / (Goal - P).ScalarProduct(L - P));
    angleR = atan((Goal - P).CrossProduct(R - P) / (Goal - P).ScalarProduct(R - P));

    return (fabs(angleL) > fabs(angleR)) ? angleL : angleR;
}
