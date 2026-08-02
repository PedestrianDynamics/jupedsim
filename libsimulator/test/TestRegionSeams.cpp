// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionSeams.hpp"
#include "Geometry/RegionSplit.hpp"
#include "MeshFixtures.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <set>
#include <utility>
#include <vector>

namespace
{
using namespace fixtures;

struct Seams {
    SurfaceMesh mesh;
    std::vector<RegionSeam> seams;
    std::size_t region_count;
};

Seams seams_of(SurfaceMesh mesh)
{
    const auto split = split_into_regions(mesh);
    auto seams = extract_region_seams(mesh, split.region);
    return Seams{std::move(mesh), std::move(seams), split.count};
}

/// Which region can be reached from which, as the seams report it.
std::set<std::pair<std::size_t, std::size_t>> connections(const std::vector<RegionSeam>& seams)
{
    std::set<std::pair<std::size_t, std::size_t>> out{};
    for(const auto& s : seams) {
        out.emplace(s.region, s.neighbor);
    }
    return out;
}

} // namespace

TEST(RegionSeams, OneRegionHasNoSeams)
{
    const auto s = seams_of(flat_rectangle());

    ASSERT_EQ(s.region_count, 1u);
    EXPECT_TRUE(s.seams.empty());
}

TEST(RegionSeams, UndulatingTerrainIsStillOneRegion)
{
    const auto s = seams_of(wavy_terrain());

    // Height alone does not split anything: a region ends where the surface starts to lie
    // over itself, and a field does not.
    ASSERT_EQ(s.region_count, 1u);
    EXPECT_TRUE(s.seams.empty());
}

TEST(RegionSeams, LevelsJoinedByAStairAreConnectedBothWays)
{
    const auto s = seams_of(two_levels_with_stair());

    ASSERT_EQ(s.region_count, 2u);
    // The seam has two sides, and each is the way to the other level. A query starting
    // downstairs has to be able to find upstairs, and the other way round.
    EXPECT_EQ(
        connections(s.seams), (std::set<std::pair<std::size_t, std::size_t>>{{0, 1}, {1, 0}}));
    EXPECT_EQ(s.seams.size() % 2, 0u);
}

TEST(RegionSeams, FloorsWithNothingJoiningThemHaveNoSeam)
{
    const auto s = seams_of(stacked_floors());

    // Two regions, but no way from one to the other: a seam needs a shared edge, and these
    // floors share none.
    ASSERT_EQ(s.region_count, 2u);
    EXPECT_TRUE(s.seams.empty());
}

TEST(RegionSeams, SwitchbackStairConnectsWhereverTheCutFalls)
{
    // The cut moves with build order, but whichever way it falls the two parts stay joined:
    // it runs through the middle of a walkable surface, not along a wall.
    for(const bool upper_first : {false, true}) {
        const auto s = seams_of(switchback_stair(upper_first));
        ASSERT_EQ(s.region_count, 2u) << "upper_first=" << upper_first;
        EXPECT_EQ(
            connections(s.seams), (std::set<std::pair<std::size_t, std::size_t>>{{0, 1}, {1, 0}}))
            << "upper_first=" << upper_first;
    }
}
