// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionReach.hpp"

#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Mathematics.hpp"

#include <algorithm>
#include <deque>

namespace
{
/// True iff looking from @p outer already covers everything looking from @p inner would.
/// Two entrances into the same region often stand close enough together that one of them is
/// pointless; without this the walk would keep finding its way back through every seam it
/// already used.
bool covers(const RegionVisit& outer, const RegionVisit& inner)
{
    return Distance(outer.from, inner.from) + inner.radius <= outer.radius;
}
} // namespace

std::vector<std::vector<RegionSeam>>
group_seams_by_region(const std::vector<RegionSeam>& seams, std::size_t region_count)
{
    std::vector<std::vector<RegionSeam>> grouped(region_count);
    for(const auto& seam : seams) {
        grouped[seam.region].push_back(seam);
    }
    return grouped;
}

std::vector<RegionVisit> regions_within_reach(
    const std::function<const std::vector<RegionSeam>&(std::size_t)>& seams_of,
    std::size_t region,
    Point start,
    double radius)
{
    std::vector<RegionVisit> visits{};
    std::deque<RegionVisit> frontier{RegionVisit{region, start, radius}};

    while(!frontier.empty()) {
        const auto visit = frontier.front();
        frontier.pop_front();

        if(std::any_of(visits.begin(), visits.end(), [&visit](const RegionVisit& seen) {
               return seen.region == visit.region && covers(seen, visit);
           })) {
            continue;
        }
        visits.push_back(visit);

        for(const auto& seam : seams_of(visit.region)) {
            const Point crossing = seam.segment.ShortestPoint(visit.from);
            const double left = visit.radius - Distance(visit.from, crossing);
            if(left <= 0.0) {
                continue;
            }
            frontier.push_back(RegionVisit{seam.neighbor, crossing, left});
        }
    }
    return visits;
}
