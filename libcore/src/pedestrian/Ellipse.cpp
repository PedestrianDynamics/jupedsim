/**
 * \file        Ellipse.cpp
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
#include "Ellipse.h"

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/************************************************************
 Konstruktoren
 ************************************************************/

JEllipse::JEllipse()
{
    _vel        = Point(); // velocity vector
    _center     = Point(); // cartesian coordinates of the center
    _cosPhi     = 1;       // = cos(0)
    _sinPhi     = 0;       // = sin(0)
    _Xp         = 0;       //x Ellipse-coord of the centre (Center in (xc,yc) )
    _Amin       = 0.18;    // Semi-axis in direction of motion:  pAmin + V * pAv
    _Av         = 0.53;
    _Bmin       = 0.20; // Semi-axis in direction of shoulders: pBmax - V *[(pBmax - pBmin) / V0]
    _Bmax       = 0.25;
    _do_stretch = true;
    _vel0       = 0; // desired speed
}


/*************************************************************
 Setter-Funktionen
 ************************************************************/

void JEllipse::SetV(const Point & v)
{
    _vel = v;
}

void JEllipse::SetCenter(Point pos)
{
    _center = pos;
}

void JEllipse::SetCosPhi(double c)
{
    _cosPhi = c;
}

void JEllipse::SetSinPhi(double s)
{
    _sinPhi = s;
}

void JEllipse::SetXp(double xp)
{
    _Xp = xp;
}

void JEllipse::SetAmin(double a_min)
{
    _Amin = a_min;
}

void JEllipse::SetAv(double a_v)
{
    _Av = a_v;
}

void JEllipse::SetBmin(double b_min)
{
    _Bmin = b_min;
}

void JEllipse::SetBmax(double b_max)
{
    _Bmax = b_max;
}

void JEllipse::SetV0(double v0)
{
    _vel0 = v0;
}

void JEllipse::DoStretch(bool stretch)
{
    _do_stretch = stretch;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

const Point & JEllipse::GetV() const
{
    return _vel;
}

const Point & JEllipse::GetCenter() const
{
    return _center;
}

double JEllipse::GetCosPhi() const
{
    return _cosPhi;
}

double JEllipse::GetSinPhi() const
{
    return _sinPhi;
}

double JEllipse::GetXp() const
{
    return _Xp;
}

double JEllipse::GetAmin() const
{
    return _Amin;
}

double JEllipse::GetAv() const
{
    return _Av;
}

double JEllipse::GetBmin() const
{
    return _Bmin;
}

double JEllipse::GetBmax() const
{
    if(_do_stretch) {
        return _Bmax;
    } else
        return _Bmin;
}

double JEllipse::GetV0() const
{
    return _vel0;
}
double JEllipse::GetArea() const
{
    double ea = GetEA(); // _Amin + V * _Av;
    double eb = GetEB(); //_Bmax - V * x;
    return ea * eb * M_PI;
}

// ellipse  semi-axis in the direction of the velocity
double JEllipse::GetEA() const
{
    return _Amin + _vel.Norm() * _Av;
}

bool JEllipse::DoesStretch() const
{
    return _do_stretch;
}

// ellipse semi-axis in the orthogonal direction of the velocity
double JEllipse::GetEB() const
{
    if(_do_stretch) {
        double x = (_vel0 <= 0.001) ? 0 : (_Bmax - _Bmin) / _vel0;
        return _Bmax - _vel.Norm() * x;
    } else {
        return _Bmin;
    }
}

double JEllipse::EffectiveDistanceToEllipse(const JEllipse & E2, double * dist) const
{
    //  E1 ist Objekt auf dem aufgerufen wird
    Point E1center = this->GetCenter();
    Point E2center = E2.GetCenter();
    Point R1, R2;
    Point E1inE2, //center of E1 in coordinate system of E2
        E2inE1;
    E2inE1 = E2center.TransformToEllipseCoordinates(
        this->GetCenter(), this->GetCosPhi(), this->GetSinPhi());
    E1inE2 = E1center.TransformToEllipseCoordinates(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
    // distance between centers of E1 and E2
    *dist = (E1center - E2center).Norm();
    R1    = this->PointOnEllipse(E2inE1);
    R2    = E2.PointOnEllipse(E1inE2);
    //effective distance
    return *dist - (E1center - R1).Norm() - (E2center - R2).Norm();
}

// input: P is a point in the ellipse world.
// output: The point on the ellipse (in cartesian coord) that lays on the same line OP
// O being the center of the ellipse
// if P approx equal to Center of ellipse return cartesian coordinats of the point (a,0)/ellipse
Point JEllipse::PointOnEllipse(const Point & P) const
{
    double x = P._x, y = P._y;
    double r = x * x + y * y;
    if(r < J_EPS * J_EPS) {
        Point CP(this->GetEA(), 0);
        return CP.TransformToCartesianCoordinates(
            this->GetCenter(), this->GetCosPhi(), this->GetSinPhi());
    }
    r = sqrt(r);

    double cosTheta = x / r;
    double sinTheta = y / r;
    double a        = GetEA();
    double b        = GetEB();
    Point S;
    S._x = a * cosTheta;
    S._y = b * sinTheta;
    return S.TransformToCartesianCoordinates(_center, _cosPhi, _sinPhi);
}

double JEllipse::EffectiveDistanceToLine(const Line & l) const
{
    Point C = this->GetCenter();
    Point P = l.ShortestPoint(C);
    Point R = this->PointOnEllipse(P);
    return (P - R).Norm();
}

// check if point given in the ellipse coordinates is inside an ellipse
bool JEllipse::IsInside(const Point & p) const
{
    double a         = GetEA();
    double b         = GetEB();
    double x         = p._x;
    double y         = p._y;
    double condition = (x * x) / (a * a) + (y * y) / (b * b) - 1;
    return condition < 0;
}

// check if point given in the ellipse coordinates is outside an ellipse
bool JEllipse::IsOutside(const Point & p) const
{
    double a         = GetEA();
    double b         = GetEB();
    double x         = p._x;
    double y         = p._y;
    double condition = (x * x) / (a * a) + (y * y) / (b * b) - 1;
    return condition > 0;
}

// check if point given in the ellipse coordinates is on an ellipse
bool JEllipse::IsOn(const Point & p) const
{
    double a         = GetEA();
    double b         = GetEB();
    double x         = p._x;
    double y         = p._y;
    double condition = (x * x) / (a * a) + (y * y) / (b * b) - 1;
    return (-J_EPS_DIST < condition) && (condition < J_EPS_DIST);
}
