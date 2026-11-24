// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

/// The former fixed wall search radius, kept as upper bound for long-reaching parameters.
constexpr double max_wall_search_radius = 4.0;

/// How far beyond contact a force 'strength' * exp(-distance / 'range') stays above 1% of the
/// 'driving' term it competes with.
inline double reach_beyond_contact(double strength, double range, double driving)
{
    constexpr double negligible_share = 0.01;
    return range * std::max(0.0, std::log(strength / (negligible_share * driving)));
}
