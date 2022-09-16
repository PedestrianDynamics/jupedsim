/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Agent.hpp"

#include "Line.hpp"

#include <Logger.hpp>
#include <cassert>

void Agent::SetEllipse(const JEllipse& e)
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

void Agent::SetV0(double v0)
{
    _ellipse.SetV0(v0);
}

void Agent::SetDeltaT(double dt)
{
    _deltaT = dt;
}
Agent::UID Agent::GetUID() const
{
    return _uid;
}

const JEllipse& Agent::GetEllipse() const
{
    return _ellipse;
}

const Point& Agent::GetPos() const
{
    return _ellipse.GetCenter();
}

const Point& Agent::GetV() const
{
    return _ellipse.GetV();
}

const Point& Agent::GetE0() const
{
    return _e0;
}

double Agent::GetV0() const
{
    double smoothFactor = 15;
    double v0 = _ellipse.GetV0();
    double f = 2.0 / (1 + exp(-smoothFactor)) - 1;
    double g = 2.0 / (1 + exp(-smoothFactor)) - 1;

    double walking_speed = (1 - f * g) * _ellipse.GetV0() + f * g * v0;
    return walking_speed;
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
        cosPhi = GetEllipse().GetCosPhi();
        sinPhi = GetEllipse().GetSinPhi();
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

Point Agent::GetE0(const Point& target, double tau) const
{
    // TODO(MC) remove input parameter tau
    double _tau = 0.5;
    const Point& pos = GetPos();
    Point delta = target - pos;
    Point new_e0 = delta.Normalized();
    double t = _newOrientationDelay * _deltaT;

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
