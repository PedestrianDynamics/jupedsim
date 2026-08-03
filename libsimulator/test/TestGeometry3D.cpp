// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"
#include "LineSegment.hpp"
#include "MeshFixtures.hpp"

#include <CGAL/mark_domain_in_triangulation.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <set>
#include <utility>
#include <vector>

namespace
{
/// Two triangles sharing a diagonal; corners given counter-clockwise.
void add_quad(SurfaceMesh& mesh, const std::array<Point3D, 4>& corners)
{
    const auto v0 = mesh.add_vertex(corners[0]);
    const auto v1 = mesh.add_vertex(corners[1]);
    const auto v2 = mesh.add_vertex(corners[2]);
    const auto v3 = mesh.add_vertex(corners[3]);
    mesh.add_face(v0, v1, v2);
    mesh.add_face(v0, v2, v3);
}

SurfaceMesh flat_room()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}});
    return mesh;
}

/// A ramp climbing from z=0 at y=0 to z=4 at y=10 (so z = 0.4*y), one region.
SurfaceMesh ramp()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{5, 0, 0}, {15, 0, 0}, {15, 10, 4}, {5, 10, 4}});
    return mesh;
}

/// Two disjoint floors sharing the same (x,y) footprint at different heights:
/// the canonical stacking case that (x,y) alone cannot resolve.
SurfaceMesh stacked_floors()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {10, 0, 0}, {10, 10, 0}, {0, 10, 0}});
    add_quad(mesh, {Point3D{0, 0, 3}, {10, 0, 3}, {10, 10, 3}, {0, 10, 3}});
    return mesh;
}

/// Gather a LineSegmentRange into a set for order-independent comparison.
/// Gather any range of segments into a set for order-independent comparison.
template <typename Range>
std::set<LineSegment> into_set(const Range& range)
{
    return std::set<LineSegment>{range.begin(), range.end()};
}

} // namespace

TEST(Geometry3DLocate, FlatRegionYieldsGroundHeight)
{
    Geometry3D geo{flat_room()};
    ASSERT_EQ(geo.region_count(), 1);

    const auto loc = geo.locate_in_region(0, {5, 5});
    ASSERT_NE(loc.face, SurfaceMesh::null_face());
    EXPECT_NEAR(loc.point.z(), 0.0, 1e-9);
    EXPECT_NEAR(loc.point.x(), 5.0, 1e-9);
    EXPECT_NEAR(loc.point.y(), 5.0, 1e-9);
}

TEST(Geometry3DLocate, PointOutsideRegionFootprintMisses)
{
    Geometry3D geo{flat_room()};
    EXPECT_EQ(geo.locate_in_region(0, {20, 20}).face, SurfaceMesh::null_face());
}

TEST(Geometry3DLocate, RampInterpolatesHeightOnFace)
{
    Geometry3D geo{ramp()};
    ASSERT_EQ(geo.region_count(), 1);

    // by construction: z == 0.4*y
    EXPECT_NEAR(geo.locate_in_region(0, {10, 2}).point.z(), 0.8, 1e-9);
    EXPECT_NEAR(geo.locate_in_region(0, {10, 9}).point.z(), 3.6, 1e-9);
}

TEST(Geometry3DLocate, RegionIdDisambiguatesStackedFloors)
{
    Geometry3D geo{stacked_floors()};
    ASSERT_EQ(geo.region_count(), 2);

    // Same (x,y), two sheets: the region id picks which one.
    EXPECT_NEAR(geo.locate_in_region(0, {5, 5}).point.z(), 0.0, 1e-9);
    EXPECT_NEAR(geo.locate_in_region(1, {5, 5}).point.z(), 3.0, 1e-9);
}

namespace
{
/// 10x10 square with a centred 2x2 hole, the 2D input shape.
PolyWithHoles square_with_hole()
{
    const std::vector<K::Point_2> outer{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    const std::vector<K::Point_2> hole{{4, 4}, {4, 6}, {6, 6}, {6, 4}};
    PolyWithHoles poly{Poly(outer.begin(), outer.end())};
    poly.add_hole(Poly(hole.begin(), hole.end()));
    return poly;
}
} // namespace

TEST(Geometry3DFromPolygon, HoleIsNotWalkable)
{
    Geometry3D geo{square_with_hole()};

    ASSERT_EQ(geo.region_count(), 1);
    EXPECT_TRUE(geo.is_valid_location({1, 1, 1}));
    EXPECT_FALSE(geo.is_valid_location({5, 5, 1})); // inside the hole
    EXPECT_FALSE(geo.is_valid_location({20, 20, 1})); // outside geometry
    // All lifted vertices sit at z=0.
    for(const auto& v : geo.vertices()) {
        EXPECT_EQ(v[2], 0.0);
    }
}

TEST(Geometry3DFromPolygon, OwnsTheProjected2DView)
{
    Geometry3D geo{square_with_hole()};

    const auto* view = geo.geometry_2d();
    ASSERT_NE(view, nullptr);
    EXPECT_TRUE(view->InsideGeometry({1, 1}));
    EXPECT_FALSE(view->InsideGeometry({5, 5})); // inside the hole
}

TEST(Geometry3DFromMesh, HasNoProjected2DView)
{
    Geometry3D geo{flat_room()};
    // A mesh carries no 2D view -- unlike a polygon-built geometry.
    EXPECT_EQ(geo.geometry_2d(), nullptr);
}

TEST(Geometry3DFromPolygon, LiftReproducesThe2DTriangulation)
{
    const auto poly = square_with_hole();
    Geometry3D geo{poly};

    // The reference: the CDT exactly as the 2D RoutingEngine builds it.
    CDT cdt{};
    cdt.insert_constraint(
        poly.outer_boundary().vertices_begin(), poly.outer_boundary().vertices_end(), true);
    for(const auto& hole : poly.holes()) {
        cdt.insert_constraint(hole.vertices_begin(), hole.vertices_end(), true);
    }
    CGAL::mark_domain_in_triangulation(cdt);

    // Coordinates are copied verbatim (both kernels store doubles), so the
    // centroid sets must match exactly in x/y.
    std::vector<std::pair<double, double>> expected{};
    for(auto f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); ++f) {
        if(f->get_in_domain()) {
            const auto c =
                CGAL::centroid(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
            expected.emplace_back(c.x(), c.y());
        }
    }
    const auto vertices = geo.vertices();
    std::vector<std::pair<double, double>> actual{};
    for(const auto& tri : geo.triangles()) {
        double x = 0;
        double y = 0;
        for(const auto v : tri) {
            x += vertices[v][0];
            y += vertices[v][1];
        }
        actual.emplace_back(x / 3.0, y / 3.0);
    }
    // Set equality: the lift produces the same triangles.
    std::sort(expected.begin(), expected.end());
    std::sort(actual.begin(), actual.end());
    EXPECT_EQ(actual, expected);
    // Exactly the triangulation's corner vertices.
    EXPECT_EQ(vertices.size(), cdt.number_of_vertices());
}

// Compare 3D queries to underlying 2D ones.
TEST(Geometry3DModelQueries, LineSegmentsInRangeMatchesFlatView)
{
    Geometry3D geo{square_with_hole()};
    const auto who = geo.get_location(2, 5, 0.0);
    ASSERT_TRUE(who.has_value());

    // No distance -> approximate grid neighbourhood.
    EXPECT_EQ(
        into_set(geo.line_segments_in_range(*who)),
        into_set(geo.geometry_2d()->LineSegmentsInApproxDistanceTo(who->xy())));

    // Explicit distance -> exact distance query.
    EXPECT_EQ(
        into_set(geo.line_segments_in_range(*who, 3.0)),
        into_set(geo.geometry_2d()->LineSegmentsInDistanceTo(3.0, who->xy())));
}

TEST(Geometry3DModelQueries, NoGeometryBetweenMatchesFlatView)
{
    Geometry3D geo{square_with_hole()};
    const auto left = geo.get_location(2, 5, 0.0);
    const auto right = geo.get_location(8, 5, 0.0);
    const auto below = geo.get_location(2, 2, 0.0);
    ASSERT_TRUE(left.has_value() && right.has_value() && below.has_value());

    // Straight line 2,5 -> 8,5 runs through the central hole: blocked.
    EXPECT_FALSE(geo.no_geometry_between(*left, right->xy() - left->xy()));
    // 2,5 -> 2,2 stays clear of the hole: visible.
    EXPECT_TRUE(geo.no_geometry_between(*left, below->xy() - left->xy()));

    // Delegation pinning against the 2D view.
    EXPECT_EQ(
        geo.no_geometry_between(*left, right->xy() - left->xy()),
        !geo.geometry_2d()->IntersectsAny(LineSegment{left->xy(), right->xy()}));
    EXPECT_EQ(
        geo.no_geometry_between(*left, below->xy() - left->xy()),
        !geo.geometry_2d()->IntersectsAny(LineSegment{left->xy(), below->xy()}));
}

TEST(Geometry3DModelQueries, AStepIsJudgedByTheWayThereNotByWhereItLands)
{
    Geometry3D geo{square_with_hole()};
    const auto who = geo.get_location(2, 5, 0.0);
    ASSERT_TRUE(who.has_value());

    const Point to_free{1, 1}; // free
    const Point to_hole{5, 5}; // inside the central hole
    const Point to_outside{20, 20}; // outside geometry
    const Point beyond_hole{8, 5}; // free again, but only reachable around the hole
    EXPECT_TRUE(geo.no_geometry_between(*who, to_free - who->xy()));
    EXPECT_FALSE(geo.no_geometry_between(*who, to_hole - who->xy()));
    EXPECT_FALSE(geo.no_geometry_between(*who, to_outside - who->xy()));
    EXPECT_FALSE(geo.no_geometry_between(*who, beyond_hole - who->xy()));

    // The last one is where the two questions part ways: it lands on walkable ground, and
    // there is still no straight step to it.
    EXPECT_TRUE(geo.geometry_2d()->InsideGeometry(beyond_hole));
}

TEST(Geometry3DVisibility, WithinOneRegionTheFlatTestAnswers)
{
    Geometry3D geo{fixtures::wavy_terrain()};
    const auto who = geo.get_location(10.0, 5.0, 0.0, 2.0);
    ASSERT_TRUE(who.has_value());

    // A field has no seams, so every chord stays on the one sheet and the flat test decides.
    EXPECT_TRUE(geo.no_geometry_between(*who, {4.0, 0.0}));
    // Out past the edge of the field, across its boundary.
    EXPECT_FALSE(geo.no_geometry_between(*who, {40.0, 0.0}));
}

TEST(Geometry3DVisibility, TheFloorAboveIsNotInSightThoughNothingStandsBetween)
{
    Geometry3D geo{stacked_floors()};
    const auto below = geo.get_location(2, 5, 0.0);
    const auto beside = geo.get_location(8, 5, 0.0);
    const auto above = geo.get_location(8, 5, 3.0);
    ASSERT_TRUE(below.has_value() && beside.has_value() && above.has_value());

    // The two floors are not joined, so the chord crosses no seam and meets no wall either
    // way. What separates them is which sheet each end is on, and nothing else.
    EXPECT_TRUE(geo.no_geometry_between(*below, *beside));
    EXPECT_FALSE(geo.no_geometry_between(*below, *above));
}

TEST(Geometry3DVisibility, LeavingTheSurfaceOverASeamBlocksTheView)
{
    Geometry3D geo{fixtures::two_levels_with_stair()};

    // Standing on the stair, two metres short of its head at x = 15.
    const auto who = geo.get_location(13.0, 2.0, 1.8, 0.5);
    ASSERT_TRUE(who.has_value());

    // Back down the stair: same region, no seam, and nothing in the way.
    EXPECT_TRUE(geo.no_geometry_between(*who, {-2.0, 0.0}));

    // On past the head, where the surface folds back over itself. The chord crosses the
    // seam, so the walk decides - and it runs off the end.
    EXPECT_FALSE(geo.no_geometry_between(*who, {4.0, 0.0}));
}

TEST(Geometry3DVisibility, CrossingASeamOntoWalkableSurfaceDoesNotBlock)
{
    Geometry3D geo{fixtures::switchback_stair()};

    // Upper floor and landing lie at the same height and meet at x = 14, which is also where
    // the region overlay cut. Walking across it stays on the surface all the way.
    const auto who = geo.get_location(12.0, 6.0, 3.0, 0.5);
    ASSERT_TRUE(who.has_value());
    EXPECT_TRUE(geo.no_geometry_between(*who, {4.0, 0.0}));

    // And someone standing where that walk comes out is in sight, even though the query
    // started in a different region than the one they are in.
    const auto beyond = geo.get_location(16.0, 6.0, 3.0, 0.5);
    ASSERT_TRUE(beyond.has_value());
    ASSERT_NE(who->region(), beyond->region());
    EXPECT_TRUE(geo.no_geometry_between(*who, *beyond));
}

TEST(Geometry3DModelQueries, AMeshAnswersWithItsOwnRegionsWalls)
{
    Geometry3D geo{fixtures::wavy_terrain()};
    const auto who = geo.get_location(10.0, 5.0, 0.0, 2.0);
    ASSERT_TRUE(who.has_value());

    // One region, so this is the plain case: the field's outline, four walls, and the query
    // has to reach them from the middle.
    const auto walls = into_set(geo.line_segments_in_range(*who, 100.0));
    EXPECT_EQ(walls.size(), 4u);
}

TEST(Geometry3DModelQueries, ASeamBringsTheOtherLevelsWallsIntoTheAnswer)
{
    Geometry3D geo{fixtures::two_levels_with_stair()};

    // Standing on the lower level right below the stair head, where the levels meet.
    const auto near_seam = geo.get_location(14.0, 2.0, 2.4, 1.0);
    ASSERT_TRUE(near_seam.has_value());
    const auto close = geo.line_segments_in_range(*near_seam, 4.0);
    ASSERT_FALSE(close.empty());

    // Standing at the far end of the lower level, the upper level is out of reach and must
    // not contribute anything.
    const auto far_away = geo.get_location(2.0, 2.0, 0.0);
    ASSERT_TRUE(far_away.has_value());
    const auto few = into_set(geo.line_segments_in_range(*far_away, 4.0));
    const auto many = into_set(close);
    EXPECT_LT(few.size(), many.size());
}

TEST(Geometry3DModelQueries, AWallBorderingTwoRegionsIsDeliveredOnce)
{
    Geometry3D geo{fixtures::switchback_stair()};

    // The wall along y = 8 runs across landing and upper floor and belongs to both regions,
    // so a query looking at both finds it twice unless it recognises it.
    const auto who = geo.get_location(13.0, 7.0, 3.0, 0.5);
    ASSERT_TRUE(who.has_value());

    const auto range = geo.line_segments_in_range(*who, 100.0);
    std::vector<LineSegment> delivered{};
    for(const auto& w : range) {
        delivered.push_back(w);
    }
    const std::set<LineSegment> distinct{delivered.begin(), delivered.end()};
    EXPECT_EQ(delivered.size(), distinct.size()) << "the same wall came back more than once";
}
