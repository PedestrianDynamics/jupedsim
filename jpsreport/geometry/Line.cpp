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


#include "Point.h"
//#include "SubRoom.h"
#include "../general/Macros.h"
#include "Line.h"

#include <Logger.h>
#include <cmath>
#include <sstream>

int Line::_static_UID = 0;

using namespace std;

/************************************************************
  Konstruktoren
 ************************************************************/
Line::Line()
{
    SetPoint1(Point()); //Default-Constructor  (0.0,0.0)
    SetPoint2(Point());
    _uid = _static_UID++;
}

Line::Line(const Point & p1, const Point & p2)
{
    SetPoint1(p1);
    SetPoint2(p2);
    _uid = _static_UID++;
}

int Line::GetUniqueID() const
{
    return _uid;
}

Line::Line(const Line & orig)
{
    _point1 = orig.GetPoint1();
    _point2 = orig.GetPoint2();
    _centre = orig.GetCentre();
    _uid    = orig.GetUniqueID();
}

Line::~Line() {}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Line::SetPoint1(const Point & p)
{
    _point1 = p;
    _centre = (_point1 + _point2) * 0.5;
}

void Line::SetPoint2(const Point & p)
{
    _point2 = p;
    _centre = (_point1 + _point2) * 0.5;
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
string Line::Write() const
{
    string geometry;
    char wall[500] = "";
    geometry.append("\t\t<wall color=\"100\">\n");
    sprintf(
        wall,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
        (GetPoint1().GetX()) * FAKTOR,
        (GetPoint1().GetY()) * FAKTOR);
    geometry.append(wall);
    sprintf(
        wall,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
        (GetPoint2().GetX()) * FAKTOR,
        (GetPoint2().GetY()) * FAKTOR);
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
    double nx, ny, norm;
    Point r = GetPoint2() - GetPoint1();

    if(r.GetX() == 0.0) {
        nx = 1;
        ny = 0;
    } else {
        nx = -r.GetY() / r.GetX();
        ny = 1;
        /* Normieren */
        norm = sqrt(nx * nx + ny * ny);
        if(fabs(norm) < J_EPS) {
            LOG_ERROR("Line::NormalVec() norm==0\n");
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
    Point l         = (GetPoint2() - GetPoint1()).Normalized();
    const Point & n = NormalVec();


    double lx = l.GetX();
    double ly = l.GetY();
    double nx = n.GetX();
    double ny = n.GetY();
    double alpha;

    if(fabs(lx) < J_EPS) {
        alpha = v.GetX() / nx;
    } else if(fabs(ly) < J_EPS) {
        alpha = v.GetY() / ny;
    } else {
        alpha = (v.GetY() * lx - v.GetX() * ly) / (nx * ly - ny * lx);
    }

    return fabs(alpha);
}
// Lotfußpunkt zur Geraden Line
// Muss nicht im Segment liegen

Point Line::LotPoint(const Point & p) const
{
    const Point & r = GetPoint1();
    const Point & s = GetPoint2();
    const Point & t = r - s;
    Point tmp;
    double lambda;

    tmp     = p - s;
    lambda  = tmp.ScalarProduct(t) / t.ScalarProduct(t);
    Point f = s + t * lambda;
    return f;
}

/* Punkt auf der Linie mit kürzestem Abstand zu p
 * In der Regel Lotfußpunkt, Ist der Lotfußpunkt nicht im Segment
 * wird der entsprechende Eckpunkt der Line genommen
 * */
Point Line::ShortestPoint(const Point & p) const
{
    const Point & t = _point1 - _point2;
    if(_point1 == _point2)
        return _point1;
    Point tmp     = p - _point2;
    double lambda = tmp.ScalarProduct(t) / t.ScalarProduct(t);
    Point f       = _point2 + t * lambda;

    /* Prüfen ob Punkt in der Linie,sonst entsprechenden Eckpunkt zurückgeben */
    if(lambda < 0)
        f = _point2;
    if(lambda > 1)
        f = _point1;

    return f;
}

/*
 *  Prüft, ob Punkt p im Liniensegment enthalten ist
 * algorithm from:
 * http://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
 *
 * */
bool Line::IsInLineSegment(const Point & p) const
{
    return fabs((_point1 - p).Norm() + (_point2 - p).Norm() - (_point2 - _point1).Norm()) < J_EPS;
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

// bool Line::operator*(const Line& l) const {
//      return ((_point1*l.GetPoint1() && _point2 == l.GetPoint2()) ||
//                      (_point2 == l.GetPoint1() && _point1 == l.GetPoint2()));
// }


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
            //Log->Write("ERROR: 1. Overlapping walls %s and %s ", toString().c_str(),l.toString().c_str());
            return true;
        }

        if(IsInLineSegment(l.GetPoint2()) && !HasEndPoint(l.GetPoint2())) {
            //Log->Write("ERROR: 2. Overlapping walls %s and %s ", toString().c_str(),l.toString().c_str());
            return true;
        }
    }
    return false;
}

//FIXME no equals check with == on double or float bring in an epsilon
bool Line::IntersectionWith(const Point & p1, const Point & p2) const
{
    Point AC           = _point1 - p1;
    Point DC           = p2 - p1;
    Point BA           = _point2 - _point1;
    double denominator = BA.CrossProduct(DC);
    double numerator   = DC.CrossProduct(AC);

    if(denominator == 0.0) {
        // the lines are superposed
        if(numerator == 0.0) {
            // the segment are superposed
            return IsInLineSegment(p1) || IsInLineSegment(p2);

        } else { // the lines are just parallel and do not share a common point

            return false;
        }
    }

    double r = numerator / denominator;
    if(r < 0.0 || r > 1.0) {
        return false;
    }

    double s = (BA.CrossProduct(AC)) / denominator;
    if(s < 0.0 || s > 1.0) {
        return false;
    }

    return true;
}

bool Line::IntersectionWith(const Line & l) const
{
    return IntersectionWith(l._point1, l._point2);
}

Line Line::Enlarge(double d) const
{
    const Point & p1 = _point1;
    const Point & p2 = _point2;
    Point diff       = (p1 - p2).Normalized() * d;

    return Line(p1 + diff, p2 - diff);
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


bool Line::ShareCommonPointWith(const Line & line) const
{
    if(line.GetPoint1() == _point1)
        return true;
    if(line.GetPoint2() == _point1)
        return true;

    if(line.GetPoint1() == _point2)
        return true;
    return line.GetPoint2() == _point2;
}

bool Line::HasEndPoint(const Point & point) const
{
    if(_point1 == point)
        return true;
    return _point2 == point;
}

bool Line::IntersectionWithCircle(const Point & centre, double radius /*cm for pedestrians*/)
{
    double r  = radius;
    double x1 = _point1.GetX();
    double y1 = _point1.GetY();

    double x2 = _point2.GetX();
    double y2 = _point2.GetY();

    double xc = centre.GetX();
    double yc = centre.GetY();

    //this formula assumes that the circle is centered the origin.
    // so we translate the complete stuff such that the circle ends up at the origin
    x1 = x1 - xc;
    y1 = y1 - yc;
    x2 = x2 - xc;
    y2 = y2 - yc;
    //xc=xc-xc;yc=yc-yc; to make it perfect

    // we first check the intersection of the circle and the  infinite line defined by the segment
    double dr2 = ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    double D2  = (x1 * y2 - x2 * y1) * (x1 * y2 - x2 * y1);
    double r2  = radius * radius;

    double delta = r2 * dr2 - D2;
    if(delta <= 0.0)
        return false;


    double a = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    double b = 2 * ((x1 * (x2 - x1)) + y1 * (y2 - y1));
    double c = x1 * x1 + y1 * y1 - r * r;

    delta = b * b - 4 * a * c;

    if((x1 == x2) && (y1 == y2)) {
        LOG_INFO("isLineCrossingCircle: Your line is a point");
        return false;
    }
    if(delta < 0.0) {
        LOG_INFO(
            "there is a bug in 'isLineCrossingCircle', delta({:.2f}) can t be <0 at this point.",
            delta);
        LOG_INFO("press ENTER");
        return false; //fixme
        //getc(stdin);
    }

    double t1 = (-b + sqrt(delta)) / (2 * a);
    double t2 = (-b - sqrt(delta)) / (2 * a);
    if((t1 < 0.0) || (t1 > 1.0))
        return false;
    if((t2 < 0.0) || (t2 > 1.0))
        return false;
    return true;
}

//TODO: Consider numerical stability and special case pt is on line
// Returns true if pt is on the left side ( from point1 toward point2)
bool Line::IsLeft(const Point & pt)
{
    double test = (_point2._x - _point1._x) * (pt.GetY() - _point1._y) -
                  (_point2._y - _point1._y) * (pt.GetX() - _point1._x);
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
double Line::GetIntersectionDistance(const Line & l) const
{
#define DEBUG 0
    double deltaACy = _point1.GetY() - l.GetPoint1().GetY();
    double deltaDCx = l.GetPoint2().GetX() - l.GetPoint1().GetX();
    double deltaACx = _point1.GetX() - l.GetPoint1().GetX();
    double deltaDCy = l.GetPoint2().GetY() - l.GetPoint1().GetY();
    double deltaBAx = _point2.GetX() - _point1.GetX();
    double deltaBAy = _point2.GetY() - _point1.GetY();

    double denominator = deltaBAx * deltaDCy - deltaBAy * deltaDCx;
    double numerator   = deltaACy * deltaDCx - deltaACx * deltaDCy;
    double infinity    = 100000;
    // the lines are parallel
    if(denominator == 0.0) {
        // the lines are superposed
        if(numerator == 0.0) {
            // the segment are superposed
            if(IsInLineSegment(l.GetPoint1()) || IsInLineSegment(l.GetPoint2()))
                return infinity; //really?
            else
                return infinity;

        } else { // the lines are just parallel and do not share a common point

            return infinity;
        }
    }

    // the lines intersect
    double r = numerator / denominator;
    if(r < 0.0 || r > 1.0) {
        return infinity;
    }

    double s = (deltaACy * deltaBAx - deltaACx * deltaBAy) / denominator;
    if(s < 0.0 || s > 1.0) {
        return infinity;
    }

    Point PointF = Point((float) (_point1._x + r * deltaBAx), (float) (_point1._y + r * deltaBAy));
    if(!IsInLineSegment(PointF)) //is point on the line?
        return infinity;
    double dist = (_point1 - PointF).NormSquare();
#if DEBUG
    printf("Enter GetIntersection\n");
    cout << "\t" << l.toString() << " intersects with " << toString() << endl;
    cout << "\t at point " << PointF.toString() << endl;
    cout << "\t\t --> distance is " << sqrt(dist) << "... return " << dist << endl;
    printf("Leave GetIntersection\n");
#endif
    return dist;
}

// calculates the angles QPF and QPL
// return the snagle of the point (F or L) which is nearer to the Goal
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
    // const double PI = 3.14159258;
#define DEBUG 0
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
#if DEBUG
    printf("Enter GetAngel()\n");
    printf("\tP=[%f,%f]\n", P.GetX(), P.GetY());
    printf("\tGoal=[%f,%f]\n", Goal.GetX(), Goal.GetY());
    printf("\tL=[%f,%f]\n", L.GetX(), L.GetY());
    printf("\tR=[%f,%f]\n", R.GetX(), R.GetY());
    printf("\t\tdist_Goal_L=%f\n", dist_Goal_L);
    printf("\t\tdist_Goal_R=%f\n", dist_Goal_R);
    printf("\t\t --> angleL=%f\n", angleL);
    printf("\t\t --> angleR=%f\n", angleR);
    printf("\t\t --> angle=%f\n", angle);
    printf("Leave GetAngel()\n");
#endif
    return angle;
}
