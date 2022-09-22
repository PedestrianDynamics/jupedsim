/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Ellipse.hpp"

void Ellipse::SetV(const Point& v)
{
    vel = v;
}

void Ellipse::SetCenter(Point pos)
{
    center = pos;
}

void Ellipse::SetCosPhi(double c)
{
    cosPhi = c;
}

void Ellipse::SetSinPhi(double s)
{
    sinPhi = s;
}

void Ellipse::SetAmin(double a_min)
{
    _Amin = a_min;
}

void Ellipse::SetAv(double a_v)
{
    _Av = a_v;
}

void Ellipse::SetBmin(double b_min)
{
    _Bmin = b_min;
}

void Ellipse::SetBmax(double b_max)
{
    Bmax = b_max;
}

void Ellipse::SetV0(double v0)
{
    vel0 = v0;
}

// ellipse  semi-axis in the direction of the velocity
double Ellipse::GetEA() const
{
    return _Amin + vel.Norm() * _Av;
}

// ellipse semi-axis in the orthogonal direction of the velocity
double Ellipse::GetEB() const
{
    double x = (vel0 <= 0.001) ? 0 : (Bmax - _Bmin) / vel0;
    return Bmax - vel.Norm() * x;
}

double Ellipse::EffectiveDistanceToEllipse(const Ellipse& E2, double* dist) const
{
    Point R1, R2;
    Point E1inE2, // center of E1 in coordinate system of E2
        E2inE1;
    E2inE1 = E2.center.TransformToEllipseCoordinates(center, this->cosPhi, this->sinPhi);
    E1inE2 = center.TransformToEllipseCoordinates(E2.center, E2.cosPhi, E2.sinPhi);
    // distance between centers of E1 and E2
    *dist = (center - E2.center).Norm();
    R1 = this->PointOnEllipse(E2inE1);
    R2 = E2.PointOnEllipse(E1inE2);
    // effective distance
    return *dist - (center - R1).Norm() - (E2.center - R2).Norm();
}

// input: P is a point in the ellipse world.
// output: The point on the ellipse (in cartesian coord) that lays on the same line OP
// O being the center of the ellipse
// if P approx equal to Center of ellipse return cartesian coordinats of the point (a,0)/ellipse
Point Ellipse::PointOnEllipse(const Point& P) const
{
    double x = P.x, y = P.y;
    double r = x * x + y * y;
    if(r < J_EPS * J_EPS) {
        Point CP(this->GetEA(), 0);
        return CP.TransformToCartesianCoordinates(center, this->cosPhi, this->sinPhi);
    }
    r = sqrt(r);

    double cosTheta = x / r;
    double sinTheta = y / r;
    double a = GetEA();
    double b = GetEB();
    Point S;
    S.x = a * cosTheta;
    S.y = b * sinTheta;
    return S.TransformToCartesianCoordinates(center, cosPhi, sinPhi);
}
