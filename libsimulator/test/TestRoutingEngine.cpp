// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "Geometry/Geometry.hpp"
#include "GeometryFixtures.hpp"
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace
{
/// The L-shape (CCW) with a single reflex corner at (1, 1).
const std::vector<Point> l_shape{{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 3}, {0, 3}};

/// Length of a path along the surface. The engine hands out the way, not what it costs -- so
/// a test that is about the length measures it itself.
double length_along_the_surface(const std::vector<Point3D>& path)
{
    double sum = 0.0;
    for(std::size_t i = 1; i < path.size(); ++i) {
        sum += std::sqrt(CGAL::to_double(CGAL::squared_distance(path[i - 1], path[i])));
    }
    return sum;
}

/// All points collinear (in xy) with the segment points.front()->points.back():
/// i.e. the (sub)path is a straight line in the plane. Takes a span so callers
/// can pass a slice of a path (e.g. the part before/after a seam).
::testing::AssertionResult PointsCollinearXY(std::span<const Point3D> points)
{
    const Point d(points.back().x() - points.front().x(), points.back().y() - points.front().y());
    for(const auto& p : points) {
        const Point v(p.x() - points.front().x(), p.y() - points.front().y());
        const double cross = d.CrossProduct(v);
        if(std::abs(cross) > 1e-6) {
            return ::testing::AssertionFailure() << "point (" << p.x() << ", " << p.y()
                                                 << ") off the line (cross=" << cross << ")";
        }
    }
    return ::testing::AssertionSuccess();
}
} // namespace

class FlatSquare : public ::testing::Test
{
public:
    void SetUp() override
    {
        geometry = test_geometries::rectangle({0, 0}, {10, 10});
        engine = std::make_unique<SurfaceMeshShortestPathRoutingEngine>(*geometry);
    }

protected:
    std::unique_ptr<Geometry> geometry{};
    std::unique_ptr<SurfaceMeshShortestPathRoutingEngine> engine{};
};

TEST_F(FlatSquare, PointAboveSurfaceIsValid)
{
    // z above the surface: the -z ray of face_below projects down onto z=0.
    EXPECT_TRUE(engine->IsValidLocation({5, 5, 1}));
    EXPECT_TRUE(engine->IsValidLocation({0.5, 0.5, 100}));
}

TEST_F(FlatSquare, PointOutsideFootprintIsInvalid)
{
    EXPECT_FALSE(engine->IsValidLocation({20, 20, 1}));
    EXPECT_FALSE(engine->IsValidLocation({-1, 5, 1}));
}

TEST_F(FlatSquare, PathWithinOneFaceIsTheDirectConnection)
{
    // Both endpoints below either diagonal, so they share a face however the square is cut:
    // single-face path.
    const Point3D source{3, 1, 1};
    const Point3D target{7, 2, 1};

    const auto path = engine->GetShortestPath(source, target);

    ASSERT_EQ(path.size(), 2u);
    // Endpoints keep the query x/y and are projected onto the surface (z=0).
    EXPECT_NEAR(path.front().x(), source.x(), 1e-6);
    EXPECT_NEAR(path.front().y(), source.y(), 1e-6);
    EXPECT_NEAR(path.front().z(), 0, 1e-6); // projected point has z=0
    EXPECT_NEAR(path.back().x(), target.x(), 1e-6);
    EXPECT_NEAR(path.back().y(), target.y(), 1e-6);
    EXPECT_NEAR(path.back().z(), 0, 1e-6); // projected point has z=0
}

TEST_F(FlatSquare, CrossingInternalEdgeStaysStraight)
{
    // Still straight line (no obstacles), crossing triangle boundaries.
    const Point3D source{2, 3, 1};
    const Point3D target{8, 7, 1};

    const auto path = engine->GetShortestPath(source, target);

    // CGAL emits waypoints at each face edge it crosses, therefore not just 2 points returned.
    // But all points need to be collinear as it is a straight line, and no longer than it.
    ASSERT_EQ(path.size(), 3u);
    EXPECT_TRUE(PointsCollinearXY(path));
    EXPECT_NEAR(length_along_the_surface(path), std::sqrt(6. * 6. + 4. * 4.), 1e-6);
}

TEST_F(FlatSquare, OrientationPointsToTarget)
{
    const Point dir = engine->GetOrientation({6, 2, 1}, {9, 5, 1});

    // Direction to the target (3, 3) normalized.
    const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    EXPECT_NEAR(dir.x, inv_sqrt2, 1e-6);
    EXPECT_NEAR(dir.y, inv_sqrt2, 1e-6);
}

TEST_F(FlatSquare, OrientationRobustWhenSourceOnEdge)
{
    // source sits exactly on the shared diagonal (y=x). CGAL then emits a
    // duplicate leading waypoint; GetOrientation must skip it and still return
    // the real heading instead of a spurious (0,0).
    const Point3D source{4, 4, 1};

    const Point dir = engine->GetOrientation(source, {8, 7, 1});

    // Heading towards (8,7) from (4,4): (4,3) normalized = (0.8, 0.6).
    EXPECT_NEAR(dir.x, 0.8, 1e-6);
    EXPECT_NEAR(dir.y, 0.6, 1e-6);
}

TEST(RoutingEngineFold, GeodesicCarriesLengthAcrossSeam)
{
    const auto geometry = test_geometries::floor_with_ramp();
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    const Point3D source{3, 2, 1}; // on the floor
    const Point3D target{4, 13, 5}; // on the ramp, projects to z = 3

    const auto path = engine.GetShortestPath(source, target);

    // Unfold the ramp about the seam (y=10): the ramp point (4,13,3) lies at
    // surface distance (13-10)*sqrt(2) from the seam, so it maps to
    // (4, 10 + 3*sqrt(2)). The geodesic is the straight line to it.
    const double unfolded_y = 10.0 + 3.0 * std::sqrt(2.0);
    const double dx = 4.0 - 3.0;
    const double dy = unfolded_y - 2.0;
    EXPECT_NEAR(length_along_the_surface(path), std::sqrt(dx * dx + dy * dy), 1e-6);

    ASSERT_GE(path.size(), 3u);
    EXPECT_NEAR(path.front().x(), source.x(), 1e-6);
    EXPECT_NEAR(path.front().y(), source.y(), 1e-6);
    EXPECT_NEAR(path.back().x(), target.x(), 1e-6);
    EXPECT_NEAR(path.back().y(), target.y(), 1e-6);
    EXPECT_NEAR(path.back().z(), 3, 1e-6); // projected onto the ramp (z = y-10)

    // The geodesic crosses the fold at a waypoint on the seam (y=10, z=0).
    const auto seam = std::find_if(
        path.begin(), path.end(), [](const Point3D& p) { return std::abs(p.y() - 10.0) < 1e-6; });
    ASSERT_NE(seam, path.end()) << "geodesic does not cross the seam";
    EXPECT_NEAR(seam->z(), 0.0, 1e-6);

    // Straight within each planar region: collinear on the floor up to the seam,
    // and collinear on the ramp from the seam onwards (the xy-direction bends
    // only at the fold). The seam point belongs to both slices.
    const auto seam_idx = static_cast<std::size_t>(std::distance(path.begin(), seam));
    EXPECT_TRUE(PointsCollinearXY(std::span(path).first(seam_idx + 1)));
    EXPECT_TRUE(PointsCollinearXY(std::span(path).subspan(seam_idx)));
}

TEST(RoutingEngineLShape, GeodesicBendsAroundReflexCorner)
{

    const auto geometry = test_geometries::from_polygons({l_shape});
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    const Point3D source{2.5, 0.5, 1};
    const Point3D target{0.5, 2.5, 1};

    const auto path = engine.GetShortestPath(source, target);

    // Straight line would cut the missing quadrant (x>1, y>1), so the any-angle geodesic must
    // pivot on the reflex corner (1,1). The route is held off that corner, diagonally into the
    // open -- which pins the pivot just as tightly, since only one point lies that way.
    ASSERT_EQ(path.size(), 3u);
    const Point turn{path[1].x(), path[1].y()};
    const Point expected = Point{1, 1} + Point{-1, -1}.Normalized() * engine.WallClearance();
    EXPECT_NEAR(turn.x, expected.x, 1e-6);
    EXPECT_NEAR(turn.y, expected.y, 1e-6);
}

TEST(RoutingEngineLShape, OrientationBendsTowardsReflexCorner)
{

    const auto geometry = test_geometries::from_polygons({l_shape});
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    // The route bends around the reflex corner (1,1), so the heading points there rather than at
    // the target -- at the turn the route makes, which is held off the corner itself. Heading for
    // the corner exactly is what leaves an agent stuck against it.
    const Point turn = Point{1, 1} + Point{-1, -1}.Normalized() * engine.WallClearance();
    const Point dir = engine.GetOrientation({2.5, 0.5, 1}, {0.5, 2.5, 1});
    const Point expected = (turn - Point{2.5, 0.5}).Normalized();
    EXPECT_NEAR(dir.x, expected.x, 1e-6);
    EXPECT_NEAR(dir.y, expected.y, 1e-6);

    // Already at the target: no heading.
    const Point at_goal = engine.GetOrientation({2.5, 0.5, 1}, {2.5, 0.5, 1});
    EXPECT_EQ(at_goal.x, 0.0);
    EXPECT_EQ(at_goal.y, 0.0);
}

TEST(RoutingEngineLShape, WaypointIsTheNextTurnOfTheGeodesic)
{

    const auto geometry = test_geometries::from_polygons({l_shape});
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    const auto from = geometry->get_location(2.5, 0.5, 0.0);
    const auto to = geometry->get_location(0.5, 2.5, 0.0);
    ASSERT_TRUE(from.has_value() && to.has_value());

    // Where the route bends, held off the corner it bends around.
    const Point turn = Point{1, 1} + Point{-1, -1}.Normalized() * engine.WallClearance();
    const Point waypoint = engine.ComputeWaypoint(*from, *to);
    EXPECT_NEAR(waypoint.x, turn.x, 1e-6);
    EXPECT_NEAR(waypoint.y, turn.y, 1e-6);

    // Standing on the target: nowhere else to head for.
    EXPECT_EQ(engine.ComputeWaypoint(*to, *to), to->xy());
}

TEST(RoutingEngineWallClearance, NegativeIsNoDistance)
{
    const auto geometry = test_geometries::rectangle({0, 0}, {10, 10});
    EXPECT_THROW(SurfaceMeshShortestPathRoutingEngine(*geometry, -0.1), SimulationError);
}

TEST(RoutingEngineWallClearance, TheSurfaceEngineKeepsItToo)
{
    const auto geometry = test_geometries::from_polygons({l_shape});
    const Point3D source{2.5, 0.5, 1};
    const Point3D target{0.5, 2.5, 1};

    // Zero routes through the corner, which is the bare geodesic.
    SurfaceMeshShortestPathRoutingEngine bare{*geometry, 0.0};
    const auto through_the_corner = bare.GetShortestPath(source, target);
    ASSERT_EQ(through_the_corner.size(), 3u);
    EXPECT_NEAR(through_the_corner[1].x(), 1.0, 1e-9);
    EXPECT_NEAR(through_the_corner[1].y(), 1.0, 1e-9);

    // And whatever distance is asked for is the distance kept.
    SurfaceMeshShortestPathRoutingEngine keeping_distance{*geometry, 0.3};
    const auto around_it = keeping_distance.GetShortestPath(source, target);
    ASSERT_EQ(around_it.size(), 3u);
    const Point turn{around_it[1].x(), around_it[1].y()};
    EXPECT_NEAR((turn - Point{1, 1}).Norm(), 0.3, 1e-9);
}

TEST(RoutingEngineCorridor, TheWaypointIsNeverTheSpotAlreadyStoodOn)
{
    // On triangles this long the path comes back with its own source point far enough off to
    // survive as a waypoint of its own -- and a step that short has no direction, so the agent
    // is sent to where it stands and stays there.
    const auto geometry = test_geometries::corridor_with_door_recesses();
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    auto walker = geometry->get_location(2.0, 0.9, 0.0);
    const auto exit = geometry->get_location(44.0, 1.0, 0.0);
    ASSERT_TRUE(walker.has_value() && exit.has_value());

    // Never overshoot the waypoint, so a step is as long as the way on is -- which is what
    // makes standing still show up as never arriving.
    constexpr double stride = 0.05;
    int steps = 0;
    while(walker->distance_to(*exit) > stride) {
        const Point onwards = engine.ComputeWaypoint(*walker, *exit) - walker->xy();
        ASSERT_LT(++steps, 2000) << "stuck at x=" << walker->xy().x;
        walker->move_on_surface(onwards.Normalized() * std::min(stride, onwards.Norm()));
    }
    EXPECT_LE(walker->distance_to(*exit), stride);
}
