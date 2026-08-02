// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/RegionSeams.hpp"
#include "Point.hpp"

#include <cstddef>
#include <vector>

/// One region a query has to look at, and the disc to look at it with.
///
/// Not the agent's own disc: after crossing a seam, distance has to be measured from where
/// the search entered the next region, with whatever reach is left. Measuring from the agent
/// would be wrong wherever regions overlap in (x, y) -- a mezzanine directly overhead is a
/// couple of centimetres away in projection and half a staircase away on the surface.
struct RegionVisit {
    std::size_t region;
    Point from;
    double radius;
};

/// Every region reachable from @p start in @p region within @p radius, measured across the
/// surface rather than through the air.
///
/// The first visit is always the region the query starts in, with the full radius. Each
/// further one is a seam crossing: the reach left is what remains after walking to the seam.
/// A region can appear more than once when several seams lead into it -- two doorways into
/// the same room are two places to look from, and neither disc covers the other.
///
/// Reported in no particular order.
std::vector<RegionVisit> regions_within_reach(
    const std::vector<std::vector<RegionSeam>>& seams_by_region,
    std::size_t region,
    Point start,
    double radius);

/// Group the flat seam list by the region a seam belongs to, which is how the reach walk
/// wants to look at it.
std::vector<std::vector<RegionSeam>>
group_seams_by_region(const std::vector<RegionSeam>& seams, std::size_t region_count);
