// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Location.hpp"
#include "GeometryFixtures.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace
{
/// The L-shape (CCW) with a single reflex corner at (1, 1).
const std::vector<Point> l_shape{{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 3}, {0, 3}};

Location located(const Geometry& geometry, double x, double y, double z_hint = 0.0)
{
    return geometry.get_location_near_z(x, y, z_hint).value();
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

TEST_F(FlatSquare, OrientationRobustWhenSourceOnEdge)
{
    // source sits exactly on the shared diagonal (y=x). CGAL then emits a
    // duplicate leading waypoint; GetOrientation must skip it and still return
    // the real heading instead of a spurious (0,0).
    const Point dir = engine->GetOrientation(located(*geometry, 4, 4), located(*geometry, 8, 7));

    // Heading towards (8,7) from (4,4): (4,3) normalized = (0.8, 0.6).
    EXPECT_NEAR(dir.x, 0.8, 1e-6);
    EXPECT_NEAR(dir.y, 0.6, 1e-6);
}

TEST(SurfaceMeshShortestPathLShape, OrientationBendsTowardsReflexCorner)
{

    const auto geometry = test_geometries::from_polygons({l_shape});
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    // The route bends around the reflex corner (1,1), so the heading points there rather than at
    // the target -- at the turn the route makes, which is held off the corner itself. Heading for
    // the corner exactly is what leaves an agent stuck against it.
    const Point turn = Point{1, 1} + Point{-1, -1}.Normalized() * engine.WallClearance();
    const auto from = located(*geometry, 2.5, 0.5);
    const Point dir = engine.GetOrientation(from, located(*geometry, 0.5, 2.5));
    const Point expected = (turn - Point{2.5, 0.5}).Normalized();
    EXPECT_NEAR(dir.x, expected.x, 1e-6);
    EXPECT_NEAR(dir.y, expected.y, 1e-6);

    // Already at the target: no heading.
    const Point at_goal = engine.GetOrientation(from, from);
    EXPECT_EQ(at_goal.x, 0.0);
    EXPECT_EQ(at_goal.y, 0.0);
}

TEST(SurfaceMeshShortestPathWallClearance, NegativeIsNoDistance)
{
    const auto geometry = test_geometries::rectangle({0, 0}, {10, 10});
    EXPECT_THROW(SurfaceMeshShortestPathRoutingEngine(*geometry, -0.1), SimulationError);
}

TEST(SurfaceMeshShortestPathWallClearance, TheSurfaceEngineKeepsItToo)
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

TEST(SurfaceMeshShortestPathCorridor, TheOrientationNeverVanishesOnTheWay)
{
    // On triangles this long the path comes back with its own source point far enough off to
    // survive as a waypoint of its own -- and a step that short has no direction, so the agent
    // is sent to where it stands and stays there.
    const auto geometry = test_geometries::corridor_with_door_recesses();
    SurfaceMeshShortestPathRoutingEngine engine{*geometry};

    auto walker = geometry->get_location_near_z(2.0, 0.9, 0.0);
    const auto exit = geometry->get_location_near_z(44.0, 1.0, 0.0);
    ASSERT_TRUE(walker.has_value() && exit.has_value());

    constexpr double stride = 0.05;
    int steps = 0;
    while(walker->distance_to(*exit) > stride) {
        const Point onwards = engine.GetOrientation(*walker, *exit);
        ASSERT_FALSE(onwards.isZeroLength()) << "stuck at x=" << walker->xy().x;
        ASSERT_LT(++steps, 2000) << "no progress at x=" << walker->xy().x;
        walker->move_on_surface(onwards * stride);
    }
    EXPECT_LE(walker->distance_to(*exit), stride);
}
