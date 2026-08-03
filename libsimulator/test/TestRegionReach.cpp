// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/PolylineMerge.hpp"
#include "Geometry/RegionReach.hpp"
#include "Geometry/RegionSeams.hpp"
#include "Geometry/RegionSplit.hpp"
#include "MeshFixtures.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <set>
#include <vector>

namespace
{
using namespace fixtures;

struct Reach {
    std::vector<std::vector<RegionSeam>> seams;
    std::size_t region_count;
};

Reach reach_of(SurfaceMesh mesh)
{
    const auto split = split_into_regions(mesh);
    const auto seams = extract_region_seams(mesh, split.region, mesh_merge_tolerance(mesh));
    return Reach{group_seams_by_region(seams, split.count), split.count};
}

std::set<std::size_t> regions_of(const std::vector<RegionVisit>& visits)
{
    std::set<std::size_t> out{};
    for(const auto& v : visits) {
        out.insert(v.region);
    }
    return out;
}

} // namespace

TEST(RegionReach, WithoutSeamsOnlyTheOwnRegionIsReached)
{
    const auto r = reach_of(flat_rectangle());

    const auto visits = regions_within_reach(r.seams, 0, Point{5.0, 5.0}, 1000.0);
    ASSERT_EQ(visits.size(), 1u);
    EXPECT_EQ(visits.front().region, 0u);
    // Nothing was crossed, so the full reach is still there to look with.
    EXPECT_DOUBLE_EQ(visits.front().radius, 1000.0);
    EXPECT_EQ(visits.front().from, Point(5.0, 5.0));
}

TEST(RegionReach, FloorsWithNothingJoiningThemStayApartAtAnyRadius)
{
    const auto r = reach_of(stacked_floors());
    ASSERT_EQ(r.region_count, 2u);

    // The other floor is three metres up and directly overhead, so any radius reaches it
    // through the air. Across the surface there is no way at all.
    const auto visits = regions_within_reach(r.seams, 0, Point{5.0, 5.0}, 1000.0);
    EXPECT_EQ(regions_of(visits), std::set<std::size_t>{0u});
}

TEST(RegionReach, ANearbySeamBringsTheOtherLevelIntoReach)
{
    const auto r = reach_of(two_levels_with_stair());
    ASSERT_EQ(r.region_count, 2u);

    // The stair head, where the levels are joined, sits at x = 15. Standing at x = 13 with
    // four metres of reach gets there; the upper level has to be looked at.
    const auto near_seam = regions_within_reach(r.seams, 0, Point{13.0, 2.0}, 4.0);
    EXPECT_EQ(regions_of(near_seam), (std::set<std::size_t>{0u, 1u}));

    // Standing at the far end with the same reach does not.
    const auto far_away = regions_within_reach(r.seams, 0, Point{2.0, 2.0}, 4.0);
    EXPECT_EQ(regions_of(far_away), std::set<std::size_t>{0u});
}

TEST(RegionReach, CrossingASeamCostsTheWayToIt)
{
    const auto r = reach_of(two_levels_with_stair());

    const Point start{13.0, 2.0};
    const auto visits = regions_within_reach(r.seams, 0, start, 4.0);

    const auto upper = std::find_if(
        visits.begin(), visits.end(), [](const RegionVisit& v) { return v.region == 1u; });
    ASSERT_NE(upper, visits.end());

    // The upper level is looked at from the seam, not from the agent, and with what is left
    // after walking there.
    EXPECT_DOUBLE_EQ(upper->from.x, 15.0);
    EXPECT_DOUBLE_EQ(upper->radius, 4.0 - Distance(start, upper->from));
    EXPECT_GT(upper->radius, 0.0);
}

TEST(RegionReach, AGenerousRadiusStillTerminates)
{
    // Seams lead both ways, so a walk that did not notice it had been somewhere already
    // would bounce between the levels until the budget ran out - or forever, if it kept
    // finding the seam it just came through at zero cost.
    const auto r = reach_of(switchback_stair());

    const auto visits = regions_within_reach(r.seams, 0, Point{5.0, 2.0}, 1000.0);
    EXPECT_EQ(regions_of(visits), (std::set<std::size_t>{0u, 1u}));
    EXPECT_LT(visits.size(), 20u) << "the walk kept re-entering regions it had covered";
}
