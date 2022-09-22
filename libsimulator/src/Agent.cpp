/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Agent.hpp"

#include "Line.hpp"

#include <Logger.hpp>
#include <cassert>

void Agent::SetEllipse(const Ellipse& e)
{
    _ellipse = e;
}

void Agent::SetPos(const Point& pos)
{
    _ellipse.SetCenter(pos);
}

void Agent::SetV(const Point& v)
{
    _ellipse.SetV(v);
}

const Ellipse& Agent::GetEllipse() const
{
    return _ellipse;
}

const Point& Agent::GetPos() const
{
    return _ellipse.center;
}

const Point& Agent::GetV() const
{
    return _ellipse.vel;
}

const Point& Agent::GetE0() const
{
    return _e0;
}

double Agent::GetV0() const
{
    const double smoothFactor = 15;
    const double f = 2.0 / (1 + exp(-smoothFactor)) - 1;
    const double g = 2.0 / (1 + exp(-smoothFactor)) - 1;
    return (1 - f * g) * v0 + f * g * v0;
}

void Agent::SetPhiPed()
{
    double cosPhi;
    double sinPhi;
    double vx = GetV().x;
    double vy = GetV().y;

    if(fabs(vx) > J_EPS || fabs(vy) > J_EPS) {
        double normv = sqrt(vx * vx + vy * vy);
        cosPhi = vx / normv;
        sinPhi = vy / normv;
    } else {
        cosPhi = GetEllipse().cosPhi;
        sinPhi = GetEllipse().sinPhi;
    }
    _ellipse.SetCosPhi(cosPhi);
    _ellipse.SetSinPhi(sinPhi);
}

void Agent::InitE0(const Point& target)
{
    const Point& pos = GetPos();
    Point delta = target - pos;

    _e0 = delta.Normalized();
}

Point Agent::GetE0(const Point& target, double deltaT) const
{
    constexpr double _tau = 0.5;
    const Point& pos = GetPos();
    const Point delta = target - pos;
    const Point new_e0 = delta.Normalized();
    const double t = _newOrientationDelay * deltaT;

    // Handover new target
    return _e0 + (new_e0 - _e0) * (1 - exp(-t / _tau));
}

void Agent::IncrementOrientationDelay()
{
    ++_newOrientationDelay;
}

void Agent::SetSmoothTurning()
{
    _newOrientationDelay = 0;
}

std::ostream& operator<<(std::ostream& out, const Agent&)
{
    // TODO(kkratz) Fix
    return out << "";
}
