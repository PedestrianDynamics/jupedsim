// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/BoundaryIndex.hpp"
#include "Geometry/Geometry3D.hpp"
#include "Geometry/RegionSplit.hpp"
#include "MeshFixtures.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

struct TestBoundaryIndexTwoLevelsWithStairs : public testing::Test {
    SurfaceMesh mesh;
    RegionSplit split;
    std::unique_ptr<Geometry3D> geo;

    void SetUp() override
    {
        mesh = fixtures::switchback_stair();
        // TODO(kkratz): Investigate segfault if region split is applied before handing mesh to
        // Geometry3D.
        geo = std::make_unique<Geometry3D>(mesh);
        split = split_into_regions(mesh);
    }

    void TearDown() override {}
};

TEST_F(TestBoundaryIndexTwoLevelsWithStairs, CanConstruct)
{
    std::unique_ptr<BoundaryIndex> bi = MakeNaiveBoundaryIndex(mesh, split);
}

TEST_F(TestBoundaryIndexTwoLevelsWithStairs, Neighboors1)
{
    std::unique_ptr<BoundaryIndex> sut = MakeNaiveBoundaryIndex(mesh, split);

    auto loc = geo->get_location(2, 2, 0).value();
    auto res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 7);

    loc = geo->get_location(16, 6, 3).value();
    res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 7);
}

TEST_F(TestBoundaryIndexTwoLevelsWithStairs, Neighboors2)
{
    std::unique_ptr<BoundaryIndex> sut = MakeNaiveBoundaryIndex(mesh, split);

    auto loc = geo->get_location(2, 2, 0).value();
    auto res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 7);

    loc = geo->get_location(16, 6, 3).value();
    res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 7);
}

#include <CGAL/IO/polygon_mesh_io.h>
TEST_F(TestBoundaryIndexTwoLevelsWithStairs, WriteMesh)
{
    // Into gtest's temp directory rather than the working directory: a test run should not
    // leave a mesh behind wherever it happened to be started from.
    const std::string path = testing::TempDir() + "TestBoundaryIndex_example.obj";
    ASSERT_TRUE(CGAL::IO::write_polygon_mesh(path, mesh, CGAL::parameters::verbose(true)));
    std::unique_ptr<BoundaryIndex> bi = MakeNaiveBoundaryIndex(mesh, split);
}

//==================================================================================================
// Portal visibility
//==================================================================================================
namespace
{
/// The expected coordinates below are the fixtures' own, so the answers should land on them
/// exactly; this only leaves room for the arithmetic of the clipping.
constexpr double kTol = 1e-7;

bool same_point(Point a, Point b)
{
    return Distance(a, b) <= kTol;
}

/// A piece is the same piece of geometry whichever way round it was stored.
bool same_piece(const LineSegment& ls, Point a, Point b)
{
    return (same_point(ls.p1, a) && same_point(ls.p2, b)) ||
           (same_point(ls.p1, b) && same_point(ls.p2, a));
}

std::string describe(const std::vector<LineSegment>& seen)
{
    std::string text{};
    for(const auto& piece : seen) {
        text += fmt::format("\n  {}", piece);
    }
    return text;
}

/// The answer's piece running between @p a and @p b, or nullptr if it has none.
const LineSegment* piece_between(const std::vector<LineSegment>& seen, Point a, Point b)
{
    const auto it =
        std::find_if(std::begin(seen), std::end(seen), [&](const LineSegment& candidate) {
            return same_piece(candidate, a, b);
        });
    return it == std::end(seen) ? nullptr : &*it;
}

/// Asks @p sut and checks what has to hold of any answer, whatever the geometry it came from.
std::vector<LineSegment> ask(BoundaryIndex& sut, const Location& loc, double radius)
{
    const auto seen = sut.Query(loc, radius);
    for(std::size_t i = 0; i < seen.size(); ++i) {
        // No piece is answered for twice ...
        for(std::size_t j = i + 1; j < seen.size(); ++j) {
            EXPECT_FALSE(same_piece(seen[j], seen[i].p1, seen[i].p2))
                << "piece reported twice: " << fmt::format("{}", seen[i]);
        }
        // ... and nothing reaches past the radius.
        EXPECT_LE(Distance(loc.xy(), seen[i].p1), radius + kTol);
        EXPECT_LE(Distance(loc.xy(), seen[i].p2), radius + kTol);
    }
    return seen;
}

/// Asserts the answer is exactly the pieces between the given pairs of points.
void ExpectPieces(const std::vector<LineSegment>& seen, const std::vector<LineSegment>& expected)
{
    EXPECT_EQ(seen.size(), expected.size()) << "answered:" << describe(seen);
    for(const auto& piece : expected) {
        EXPECT_NE(piece_between(seen, piece.p1, piece.p2), nullptr)
            << "missing " << fmt::format("{}", piece) << ", answered:" << describe(seen);
    }
}

/// Asserts the piece running from @p from to @p to is among the answer.
void ExpectSeen(const std::vector<LineSegment>& seen, Point from, Point to)
{
    EXPECT_NE(piece_between(seen, from, to), nullptr)
        << "missing " << fmt::format("{}", LineSegment(from, to)) << ", answered:"
        << describe(seen);
}
} // namespace

class PortalVisibility : public testing::Test
{
protected:
    std::unique_ptr<Geometry3D> geo{};

    /// Builds the index under test on @p mesh and keeps the geometry alive, so a test can ask
    /// it where to query from. Both come from the same region overlay on purpose: splitting
    /// the same mesh twice would leave it to chance that the region ids agree.
    std::unique_ptr<BoundaryIndex> IndexOn(SurfaceMesh mesh)
    {
        geo = std::make_unique<Geometry3D>(std::move(mesh));
        return MakePortalBoundaryIndex(geo->mesh(), geo->region_split());
    }

    Location At(double x, double y, double z) const
    {
        const auto loc = geo->get_location(x, y, z);
        EXPECT_TRUE(loc.has_value()) << "no surface at " << fmt::format("({}, {}, {})", x, y, z);
        return loc.value();
    }
};

TEST_F(PortalVisibility, EveryWallOfAConvexRoomIsSeenWhole)
{
    // The 10x10 rectangle, with a mid vertex splitting the two sides the profile was swept
    // along: six border segments, and from the middle of a convex room nothing hides anything,
    // so each wall comes back as itself.
    auto sut = IndexOn(fixtures::flat_rectangle());
    const auto seen = ask(*sut, At(5, 5, 0), 100.0);

    ExpectPieces(
        seen,
        {{{0, 0}, {5, 0}},
         {{5, 0}, {10, 0}},
         {{0, 10}, {5, 10}},
         {{5, 10}, {10, 10}},
         {{0, 0}, {0, 10}},
         {{10, 0}, {10, 10}}});
}

TEST_F(PortalVisibility, TheRadiusCutsBackWhatIsSeenAndDropsWhatItDoesNotReach)
{
    // From (2, 5) the near side is 2 m away and every other wall 5 m or more, so a radius of 3
    // leaves one wall -- and of that only the piece inside the disc, y = 5 +/- sqrt(3^2 - 2^2).
    auto sut = IndexOn(fixtures::flat_rectangle());
    const auto seen = ask(*sut, At(2, 5, 0), 3.0);

    const double half = std::sqrt(5.0);
    ExpectPieces(seen, {{{0, 5 - half}, {0, 5 + half}}});
}

TEST_F(PortalVisibility, ADoorRecessIsHiddenFromDownTheCorridor)
{
    // The recesses are 1 m wide and 0.3 m deep, so from far down the corridor the sight cone
    // through a mouth has swung past the recess long before it is 0.3 m deep: from (1, 1) the
    // cone through the mouth at x in [41, 42] covers x in [53, 54.3] at y = 2.3. Every back
    // wall, and every jamb on the near side of a mouth, is out of sight.
    //
    // What the cone does still catch is the bottom of the far jamb. The line from (1, 1) past
    // the near corner of a mouth at x = n - 1 climbs at 1/(n - 2) and so meets the jamb at
    // x = n at y = 1 + (n - 1)/(n - 2) -- a couple of centimetres of it for the far recesses,
    // a quarter of a metre for the nearest.
    //
    // The rest is the corridor's own outline: the near wall, the two ends, and the far wall in
    // the eight pieces the mouths cut it into, each seen whole.
    auto sut = IndexOn(fixtures::corridor_with_door_recesses());
    const auto seen = ask(*sut, At(1, 1, 0), 100.0);

    // The recess mouths, as the fixture lays them out: [n - 1, n] for each of these.
    const std::vector<double> mouths{42, 36, 30, 24, 18, 12, 6};

    std::vector<LineSegment> expected{
        {{0, 0}, {45, 0}},
        {{45, 0}, {45, 2}},
        {{0, 0}, {0, 2}},
        {{42, 2}, {45, 2}},
        {{36, 2}, {41, 2}},
        {{30, 2}, {35, 2}},
        {{24, 2}, {29, 2}},
        {{18, 2}, {23, 2}},
        {{12, 2}, {17, 2}},
        {{6, 2}, {11, 2}},
        {{0, 2}, {5, 2}}};
    for(const double n : mouths) {
        expected.emplace_back(Point{n, 2}, Point{n, 1.0 + (n - 1.0) / (n - 2.0)});
    }
    ExpectPieces(seen, expected);
}

TEST_F(PortalVisibility, ADoorRecessIsWholeFromInFrontOfIt)
{
    // Standing in the middle of the mouth, both corners are passed: the line from (41.5, 1) to
    // either end of the back wall crosses y = 2 at 41.5 -/+ 0.5/1.3, inside the mouth.
    auto sut = IndexOn(fixtures::corridor_with_door_recesses());
    const auto seen = ask(*sut, At(41.5, 1, 0), 3.0);

    ExpectSeen(seen, {41, 2.3}, {42, 2.3});
}

TEST_F(PortalVisibility, ADoorRecessIsCutBackByTheCornerItIsSeenPast)
{
    // From (40, 1) the near corner (41, 2) is in the way: the line past it reaches y = 2.3 at
    // x = 40 + 1.3 = 41.3, so the left third of the back wall is hidden and the rest is not.
    // The far corner cuts nothing -- past (42, 2) that line is already at x = 42.6.
    auto sut = IndexOn(fixtures::corridor_with_door_recesses());
    const auto seen = ask(*sut, At(40, 1, 0), 5.0);

    ExpectSeen(seen, {41.3, 2.3}, {42, 2.3});
}

TEST_F(PortalVisibility, AFoldedLevelIsNotSeenThroughItsOwnSeam)
{
    // The lower level runs x in [0, 15] and the upper folds back over x in [5, 15], so their
    // seam stands at x = 15 -- at the far end of both. A sight line reaching it is travelling
    // away from the level it crosses into and leaves that level's footprint at once. Each
    // level therefore sees its own walls whole and nothing else, however generous the radius.
    auto sut = IndexOn(fixtures::two_levels_with_stair());

    const auto from_below = ask(*sut, At(2, 2, 0), 100.0);
    ExpectPieces(
        from_below,
        {{{0, 0}, {10, 0}},
         {{10, 0}, {15, 0}},
         {{0, 4}, {10, 4}},
         {{10, 4}, {15, 4}},
         {{0, 0}, {0, 4}}});

    const auto from_above = ask(*sut, At(12, 2, 3), 100.0);
    ExpectPieces(from_above, {{{5, 0}, {15, 0}}, {{5, 4}, {15, 4}}, {{5, 0}, {5, 4}}});
}

TEST_F(PortalVisibility, NoWallOfTheFloorOverheadIsSeenFromUnderIt)
{
    // (5, 6) is on the ground floor with the upper floor 3 m above it. Reaching the upper
    // floor means crossing the seam at x = 14, y in [4, 8], and every line from (5, 6) towards
    // it runs into the ground floor's own wall at x = 10 first. What is seen is the ground
    // floor, the flight and -- through the opening the flight leaves at x = 10, y in [0, 4] --
    // the near part of the landing. The line grazing the corner (10, 4) reaches x = 18 at
    // y = 6 - 2 * 13/5 = 0.8, so of the landing's end wall only the bottom 0.8 m is visible.
    // Nothing of the upper floor's walls at y = 4 and y = 8 shows up: no piece reaches past
    // x = 10 on either line.
    auto sut = IndexOn(fixtures::switchback_stair());
    const auto seen = ask(*sut, At(5, 6, 0), 100.0);

    ExpectPieces(
        seen,
        {{{0, 0}, {10, 0}},
         {{0, 0}, {0, 4}},
         {{10, 4}, {10, 8}},
         {{0, 8}, {10, 8}},
         {{0, 4}, {0, 8}},
         {{10, 0}, {14, 0}},
         {{14, 0}, {18, 0}},
         {{18, 0}, {18, 0.8}}});
}

TEST_F(PortalVisibility, WallsBeyondASeamAreSeenThroughIt)
{
    // The other side of the same seam. From (16, 6) on the landing the upper floor lies
    // straight ahead through the seam at x = 14, y in [4, 8], and the sight lines crossing it
    // sweep the whole of the upper floor's three walls: the line to its far corner (0, 8)
    // passes the seam at y = 6 + 2/8 and the one to (14, 8) at the seam's own end. Down the
    // other way, past the seam rather than through it, the flight's wall at y = 0 is seen
    // whole too. What the ground floor has at y = 8 and x = 10 stays out of the answer: it is
    // behind the fold, and no piece of the y = 8 line stops at x = 10.
    auto sut = IndexOn(fixtures::switchback_stair());
    const auto seen = ask(*sut, At(16, 6, 3), 100.0);

    ExpectPieces(
        seen,
        {{{14, 8}, {18, 8}},
         {{18, 4}, {18, 8}},
         {{18, 0}, {18, 4}},
         {{14, 0}, {18, 0}},
         {{10, 0}, {14, 0}},
         {{0, 4}, {14, 4}},
         {{0, 8}, {14, 8}},
         {{0, 4}, {0, 8}}});
}

TEST_F(PortalVisibility, ARadiusThatStopsShortOfTheSeamShowsNothingBeyondIt)
{
    // Same spot, 2.5 m of reach. The seam is 2 m away but everything past it is 2*sqrt(2) m
    // off, so only the two walls of the landing corner are left, each cut to its chord.
    auto sut = IndexOn(fixtures::switchback_stair());
    const auto seen = ask(*sut, At(16, 6, 3), 2.5);

    ExpectPieces(seen, {{{14.5, 8}, {17.5, 8}}, {{18, 4.5}, {18, 7.5}}});
}

TEST_F(PortalVisibility, StandingOnASeamStillAnswers)
{
    // (14, 6) is exactly on the seam, so the seam sits at zero distance along every ray. Which
    // of the two regions the location lands in is up to the lookup, and a ray is only ever
    // taken to have crossed a seam it reaches after leaving the agent -- so what comes back
    // depends on that. It still has to be an answer: real pieces, and nothing beyond the
    // radius.
    auto sut = IndexOn(fixtures::switchback_stair());
    const auto seen = ask(*sut, At(14, 6, 3), 3.0);

    EXPECT_FALSE(seen.empty()) << "nothing at all was reported from the seam";
}

TEST_F(PortalVisibility, AFloorWithNothingLeadingToItNeverShowsUp)
{
    // Two 10x10 floors over the same footprint and no way between them: the second one has the
    // same four walls in plan as the first, and no seam ever opens a window into its region.
    // The answer is exactly the ground floor's four walls, seen whole. (The two floors are
    // indistinguishable in plan, so a leak of the upper floor could only ever surface as extra
    // or differing pieces -- which the exact comparison rules out.)
    auto sut = IndexOn(fixtures::stacked_floors());
    const auto seen = ask(*sut, At(5, 5, 0), 100.0);

    ExpectPieces(
        seen, {{{0, 0}, {10, 0}}, {{10, 0}, {10, 10}}, {{0, 10}, {10, 10}}, {{0, 0}, {0, 10}}});
}
