/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Line.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"

class Ellipse
{
public:
    /// cartesian-coord of the centre
    Point center{};
    /// velocity vector
    Point vel{};
    double cosPhi{1};
    double sinPhi{0};
    double Bmax{0.25};

private:
    /// pAmin + V * pAv
    double _Amin{0.18};
    double _Av{0.53};
    /// Semi-axis in direction of shoulders: pBmax - V *[(pBmax - pBmin) / V0]
    double _Bmin{0.20};

public:
    void SetV(const Point& v);
    void SetCenter(Point pos);
    void SetCosPhi(double c);
    void SetSinPhi(double s);
    void SetAmin(double a_min);
    void SetAv(double a_v);
    void SetBmin(double b_min);
    void SetBmax(double b_max);
    double GetEA() const; // ellipse semi-axis in the direction of the velocity
    // ellipse semi-axis in the orthogonal direction of the velocity
    double GetEB(double scale) const;
    // Effective distance between two ellipses
    double
    EffectiveDistanceToEllipse(const Ellipse& other, double scale_first, double scale_second) const;
    // Schnittpunkt der Ellipse mit der Gerade durch P und AP (=ActionPoint von E)
    Point PointOnEllipse(const Point& p, double scale) const;
};
