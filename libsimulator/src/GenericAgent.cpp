/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GenericAgent.hpp"

#include "Line.hpp"

#include <Logger.hpp>

#include <cassert>

// TODO(kkratz): This does neither belong here nor is this a good name.
Point MollifyE0(
    const Point& target,
    const Point& pos,
    double deltaT,
    int orientationDelay,
    const Point& e0)
{
    constexpr double _tau = 0.5;
    const Point delta = target - pos;
    const Point new_e0 = delta.Normalized();
    const double t = orientationDelay * deltaT;

    // Handover new target
    return e0 + (new_e0 - e0) * (1 - exp(-t / _tau));
}
