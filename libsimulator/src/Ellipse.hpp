/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Line.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"

class JEllipse
{
private:
    Point _vel; // velocity vector
    Point _center; // cartesian-coord of the centre
    double _cosPhi; // cos(phi)
    double _sinPhi; // sin(phi)
    double _Xp; // x Ellipse-coord of the centre (Center in (xc,yc) )
    double _Amin; //   pAmin + V * pAv
    double _Av;
    double _Bmin; //  pBmax - V *[(pBmax - pBmin) / V0]
    double _Bmax;
    double _vel0; // desired speed
    bool _do_stretch = true;

public:
    JEllipse();
    void SetV(const Point& v);
    void SetCenter(Point pos);
    void SetCosPhi(double c);
    void SetSinPhi(double s);
    void SetXp(double xp);
    void SetAmin(double a_min);
    void SetAv(double a_v);
    void SetBmin(double b_min);
    void SetBmax(double b_max);
    void SetV0(double v0);
    void DoStretch(bool stretch);
    const Point& GetV() const;
    const Point& GetCenter() const;
    double GetCosPhi() const;
    double GetSinPhi() const;
    double GetXp() const;
    double GetAmin() const;
    double GetAv() const;
    double GetBmin() const;
    double GetBmax() const;
    double GetV0() const;
    double GetEA() const; // ellipse semi-axis in the direction of the velocity
    double GetEB() const; // ellipse semi-axis in the orthogonal direction of the velocity
    double GetMaxEA() const;
    double GetMaxEB() const;
    double GetArea() const;
    bool DoesStretch() const;
    // Effective distance between two ellipses
    double EffectiveDistanceToEllipse(const JEllipse& other, double* dist) const;
    // Effective distance between ellipse and line segment
    double EffectiveDistanceToLine(const Line& l) const;
    // Schnittpunkt der Ellipse mit der Gerade durch P und AP (=ActionPoint von E)
    Point PointOnEllipse(const Point& p) const;
    // Schnittpunkt der Ellipse mit dem Liniensegment line
    Point PointOnEllipse(const Line& line, const Point& P) const;
    // Check if point p is inside the ellipse
    bool IsInside(const Point& p) const;
    // Check if point p is outside the ellipse
    bool IsOutside(const Point& p) const;
    // Check if point p is on the ellipse
    bool IsOn(const Point& p) const;
    bool IntersectionWithLine(const Line& line);
};
