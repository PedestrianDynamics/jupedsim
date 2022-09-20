/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Agent.hpp"

#include "Line.hpp"

#include <Logger.hpp>
#include <cassert>

const Point& Agent::GetE0() const
{
    return _e0;
}

// double Agent::GetV0() const
//{
//     // TODO(kkratz): This is broken atm, this should smooth current to desired speed (i.e. should
//     // work on speed / v0)
//     const double smoothFactor = 15;
//     const double f = 2.0 / (1 + exp(-smoothFactor)) - 1;
//     const double g = 2.0 / (1 + exp(-smoothFactor)) - 1;
//     return (1 - f * g) * v0 + f * g * v0;
// }

Point Agent::GetE0(const Point& target, double deltaT) const
{
    constexpr double _tau = 0.5;
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
