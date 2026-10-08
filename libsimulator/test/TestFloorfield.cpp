// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Multi-region floor fields (libfloorfield) over geometries built by WalkableSurface, driven
// through the cxx bridge the way a routing engine would: translate the region graph, build the
// field, register destinations, follow directions and query travel times.
#include "Geometry/Geometry.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/WalkableSurface.hpp"
#include "GeometryFixtures.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "floorfield_cxx/lib.h"
#include "rust/cxx.h"

#include <CGAL/squared_distance_3.h>
#include <boost/range/iterator_range.hpp>
#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace ff = jupedsim::floorfield;
using test_geometries::rectangle_points;

namespace
{
ff::Ring to_ring(const Poly& poly)
{
    ff::Ring ring{};
    for(const auto& p : poly.container()) {
        ring.points.push_back({CGAL::to_double(p.x()), CGAL::to_double(p.y())});
    }
    return ring;
}

/// The region graph of @p geo in the bridge's layout, which mirrors RegionGraph2D: regions with
/// their rings, and every directed edge as a seam through ring edge (ring, index) of its source.
/// The floor field checks it; this only copies.
ff::RegionGraph to_floorfield(const Geometry::RegionGraph2D& g)
{
    ff::RegionGraph out{};
    for(const auto r : boost::make_iterator_range(boost::vertices(g))) {
        const PolyWithHoles& poly = g[r];
        ff::Region region{};
        region.outer = to_ring(poly.outer_boundary());
        for(const auto& hole : poly.holes()) {
            region.holes.push_back(to_ring(hole));
        }
        out.regions.push_back(std::move(region));
    }
    for(const auto e : boost::make_iterator_range(boost::edges(g))) {
        out.seams.push_back(
            {boost::source(e, g),
             boost::target(e, g),
             static_cast<std::uint32_t>(g[e].ring),
             static_cast<std::uint32_t>(g[e].index)});
    }
    return out;
}

/// A floor field over the region graph of @p geo, which must be built by WalkableSurface.
rust::Box<ff::MultiRegionFloorfield> field_over(const Geometry& geo, double cellSize)
{
    return ff::new_multi_region_floorfield(to_floorfield(*geo.region_graph_2d()), cellSize, 0.5);
}

/// Ground (region 0, z = 0) and upper floor (region 1, z = 3), joined by stairs rising 3 m over
/// 5 m (region 2).
std::unique_ptr<Geometry> two_floors()
{
    WalkableSurface surface{};
    const auto ground =
        surface.AddRegion(WalkableSurface::Polygon{rectangle_points({0, 0}, {5, 5}), {}}, 0.0);
    const auto upper =
        surface.AddRegion(WalkableSurface::Polygon{rectangle_points({10, 0}, {15, 5}), {}}, 3.0);
    surface.ConnectRegions(ground, {{5, 0}, {5, 5}}, upper, {{10, 0}, {10, 5}});
    return surface.CreateGeometry();
}

/// Travel time at @p xy in @p region.
double time_at(ff::MultiRegionFloorfield& field, std::size_t region, std::size_t dest, Point xy)
{
    const auto grid = field.region_grid(region);
    const auto col = static_cast<std::size_t>(std::floor((xy.x - grid.origin_x) / grid.cell_size));
    const auto row = static_cast<std::size_t>(std::floor((xy.y - grid.origin_y) / grid.cell_size));
    return field.region_travel_times(region, dest)[row * grid.width + col];
}
/// Follow the field's directions from @p from the way an agent does, in steps of @p stepLength:
/// every step moves a Location over the surface mesh, which tracks the region across seams and
/// keeps stacked floors apart. Stops on arrival (a zero direction, or the destination's cell with
/// travel time 0), when a step would leave the walkable surface, or after a step budget.
std::vector<Location>
trace(ff::MultiRegionFloorfield& field, Location from, std::size_t dest, double stepLength = 0.1)
{
    std::vector<Location> path{from};
    for(int i = 0; i < 2000; ++i) {
        if(time_at(field, from.region(), dest, from.xy()) == 0.0) {
            break;
        }
        const auto dir = field.region_direction(from.region(), {from.xy().x, from.xy().y}, dest);
        if(dir.x == 0.0 && dir.y == 0.0) {
            break;
        }
        const auto moved = from.try_move_on_surface(Point{dir.x, dir.y} * stepLength);
        if(!moved) {
            break;
        }
        from = *moved;
        path.push_back(from);
    }
    return path;
}

double length_3d(const std::vector<Location>& path)
{
    double length = 0.0;
    for(std::size_t i = 1; i < path.size(); ++i) {
        length += std::sqrt(
            CGAL::to_double(
                CGAL::squared_distance(path[i - 1].position_3d(), path[i].position_3d())));
    }
    return length;
}

} // namespace

TEST(Floorfield, ClimbsTheStairsToTheUpperFloor)
{
    const auto geo = two_floors();
    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_point_destination(1, {14, 2.5});
    const auto path = trace(*field, geo->get_location(1, 2.5, 0), dest);

    ASSERT_GE(path.size(), 3u);
    EXPECT_DOUBLE_EQ(path.front().z(), 0.0);
    EXPECT_NEAR(path.back().xy().x, 14.0, 0.1) << "ends at the target";
    EXPECT_EQ(path.back().region(), 1u);
    EXPECT_NEAR(path.back().z(), 3.0, 1e-9);
    // Height never decreases on the way up.
    for(std::size_t i = 1; i < path.size(); ++i) {
        EXPECT_GE(path[i].z(), path[i - 1].z() - 1e-9) << "step " << i;
    }
    // 4 m ground + sqrt(34) m stairs + 4 m upper floor.
    EXPECT_NEAR(length_3d(path), 8.0 + std::sqrt(34.0), 0.3);
}

/// Down again: the way leaves the upper floor and the stairs through the low-x side of their
/// grids, where the halo past each seam needs room of its own.
TEST(Floorfield, DescendsTheStairsToTheGround)
{
    const auto geo = two_floors();
    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_point_destination(0, {1, 2.5});
    const auto path = trace(*field, geo->get_location(14, 2.5, 1), dest);

    EXPECT_NEAR(path.back().xy().x, 1.0, 0.1) << "ends at the target";
    EXPECT_EQ(path.back().region(), 0u);
    EXPECT_NEAR(path.back().z(), 0.0, 1e-9);
    for(std::size_t i = 1; i < path.size(); ++i) {
        EXPECT_LE(path[i].z(), path[i - 1].z() + 1e-9) << "step " << i;
    }
    EXPECT_NEAR(length_3d(path), 8.0 + std::sqrt(34.0), 0.3);
}

TEST(Floorfield, DirectionIsAUnitVectorTowardsTheTarget)
{
    const auto geo = two_floors();
    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_point_destination(1, {14, 2.5});

    const auto dir = field->region_direction(0, {1, 2.5}, dest);
    EXPECT_NEAR(std::hypot(dir.x, dir.y), 1.0, 1e-12);
    EXPECT_GT(dir.x, 0.99);
}

/// A doorway narrower than a cell holds no cell centre: the grid cannot pass it, and the room
/// behind it has no way to the destination. Asking for a direction there throws; a straight
/// line to the destination would lead through the wall.
TEST(Floorfield, RoomBehindADoorwayNarrowerThanACellThrows)
{
    WalkableSurface surface{};
    const auto a =
        surface.AddRegion({{{0, 0}, {10, 0}, {10, 5}, {10, 5.04}, {10, 10}, {0, 10}}, {}}, 0.0);
    const auto b =
        surface.AddRegion({{{11, 0}, {21, 0}, {21, 10}, {11, 10}, {11, 5.04}, {11, 5}}, {}}, 0.0);
    surface.ConnectRegions(a, {{10, 5}, {10, 5.04}}, b, {{11, 5}, {11, 5.04}});
    const auto geo = surface.CreateGeometry();
    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_point_destination(a, {2, 2});

    EXPECT_NO_THROW(field->region_direction(a, {8, 8}, dest));
    EXPECT_THROW(field->region_direction(b, {15, 5}, dest), rust::Error);
}

TEST(Floorfield, RoutableLocations)
{
    const auto geo = two_floors();
    auto field = field_over(*geo, 0.1);
    EXPECT_TRUE(field->region_is_routable(0, {2.5, 2.5}));
    EXPECT_TRUE(field->region_is_routable(1, {12.5, 2.5}));
    EXPECT_FALSE(field->region_is_routable(0, {12.5, 2.5})) << "on the upper floor, not the ground";
    EXPECT_FALSE(field->region_is_routable(0, {20, 2.5})) << "off the surface";
}

/// An exit across the doorway between two rooms, cut into one piece per region, is one
/// destination: inside any piece the travel time is zero, and each room heads for its own side.
TEST(Floorfield, ExitSpanningSeveralRegions)
{
    const auto geo = test_geometries::two_rooms(); // rooms 0 and 1, doorway (connector) 2
    const std::vector<Point2D> corners{{9.5, 4}, {11.5, 4}, {11.5, 6}, {9.5, 6}};
    const Poly exit(corners.begin(), corners.end());
    const auto pieces = geo->split_into_region_pieces(exit, 0);
    ASSERT_EQ(pieces.size(), 3u);

    std::vector<ff::AreaPiece> ffPieces{};
    for(const auto& piece : pieces) {
        ffPieces.push_back({piece.region, to_ring(piece.polygon)});
    }

    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_area_destination({ffPieces.data(), ffPieces.size()});

    EXPECT_EQ(time_at(*field, 0, dest, {9.8, 5}), 0.0) << "piece in room 0";
    EXPECT_EQ(time_at(*field, 2, dest, {10.5, 5}), 0.0) << "piece in the doorway";
    EXPECT_EQ(time_at(*field, 1, dest, {11.2, 5}), 0.0) << "piece in room 1";

    // Inside the exit, in each of its three pieces: there, the zero vector.
    for(const auto& [region, p] : std::vector<std::pair<std::size_t, ff::Point2d>>{
            {0, {9.8, 5}}, {2, {10.5, 5}}, {1, {11.2, 5}}}) {
        const auto dir = field->region_direction(region, p, dest);
        EXPECT_EQ(dir.x, 0.0) << "region " << region;
        EXPECT_EQ(dir.y, 0.0) << "region " << region;
    }
    // Just outside it in room 0 (the exit spans x 9.5..11.5): a unit vector towards it.
    const auto outside = field->region_direction(0, {8.5, 5}, dest);
    EXPECT_NEAR(std::hypot(outside.x, outside.y), 1.0, 1e-12);
    EXPECT_GT(outside.x, 0.9);

    const auto path = trace(*field, geo->get_location(2, 5, 0), dest);
    EXPECT_EQ(path.back().region(), 0u) << "room 0 heads for its own piece";
    EXPECT_GT(path.back().xy().x, 9.4);
}

/// A seam on a hole edge: the ground floor opens through the inner edge of its hole onto a ramp
/// leading up to a platform inside the hole.
TEST(Floorfield, SeamOnAHoleEdge)
{
    WalkableSurface surface{};
    const auto ground = surface.AddRegion(
        {{{0, 0}, {20, 0}, {20, 10}, {0, 10}},
         {{{2, 2}, {18, 2}, {18, 8}, {2, 8}, {2, 7}, {2, 3}}}},
        0.0);
    const auto platform = surface.AddRegion({{{12, 3}, {17, 3}, {17, 7}, {12, 7}}, {}}, 1.0);
    surface.ConnectRegions(ground, {{2, 3}, {2, 7}}, platform, {{12, 3}, {12, 7}});
    const auto geo = surface.CreateGeometry();

    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_point_destination(platform, {15, 5});
    const auto path = trace(*field, geo->get_location(1, 5, ground), dest);
    EXPECT_EQ(path.back().region(), platform);
    EXPECT_NEAR(path.back().xy().x, 15.0, 0.15);
    EXPECT_NEAR(path.back().z(), 1.0, 1e-9);
}

/// Two floors over the same footprint: a destination on the upper floor is reached by the ramp,
/// never straight through the floor above or below.
TEST(Floorfield, StackedFloorsAreKeptApart)
{
    const auto geo = test_geometries::stacked_floors_with_ramp(); // ground 0, upper 1, ramp 2
    auto field = field_over(*geo, 0.1);
    const auto dest = field->add_point_destination(1, {2, 2});

    // Right below the target: 4.5 m to the ramp, ~8.5 m up it, ~12.4 m back on the upper floor.
    EXPECT_GT(time_at(*field, 0, dest, {2, 2}), 20.0);

    const auto path = trace(*field, geo->get_location(2, 2, 0), dest);
    bool on_ramp = false;
    for(const auto& p : path) {
        on_ramp = on_ramp || p.region() == 2;
    }
    EXPECT_TRUE(on_ramp);
    EXPECT_EQ(path.back().region(), 1u);
    EXPECT_NEAR(path.back().xy().x, 2.0, 0.15);
}

/// The length walked along the field's directions, against the exact geodesic.
TEST(Floorfield, ComparedWithTheExactGeodesic)
{
    struct Case {
        const char* name;
        std::unique_ptr<Geometry> geo;
        std::size_t fromRegion;
        Point3D from;
        std::size_t toRegion;
        Point3D to;
    };
    std::vector<Case> cases{};
    cases.push_back({"two floors + stairs", two_floors(), 0, {1, 2.5, 0}, 1, {14, 4, 3}});
    cases.push_back({"doorway", test_geometries::two_rooms(), 0, {5, 1, 0}, 1, {20, 9, 0}});

    for(auto& c : cases) {
        SurfaceMeshShortestPathRoutingEngine geodesic{*c.geo};
        const auto geodesic_path = geodesic.GetShortestPath(c.from, c.to);
        double g = 0.0;
        for(std::size_t i = 1; i < geodesic_path.size(); ++i) {
            g += std::sqrt(CGAL::squared_distance(geodesic_path[i - 1], geodesic_path[i]));
        }
        auto field = field_over(*c.geo, 0.1);
        const auto dest = field->add_point_destination(c.toRegion, {c.to.x(), c.to.y()});
        const auto from = c.geo->get_location(c.from.x(), c.from.y(), c.fromRegion);
        const double walked = length_3d(trace(*field, from, dest));

        EXPECT_LT(std::abs(walked / g - 1), 0.03) << c.name << ": walked " << walked << " vs " << g;
    }
}

TEST(Floorfield, ErrorsBecomeExceptions)
{
    const auto geo = two_floors();
    auto field = field_over(*geo, 0.1);
    EXPECT_THROW(field->add_point_destination(0, {50, 50}), rust::Error) << "not walkable";
    EXPECT_THROW(field->add_point_destination(7, {1, 1}), rust::Error) << "unknown region";
    EXPECT_THROW(field->region_direction(0, {1, 1}, 42), rust::Error) << "unknown destination";
    const auto dest = field->add_point_destination(0, {1, 2.5});
    EXPECT_THROW(field->add_point_destination(0, {std::nan(""), 1}), rust::Error) << "NaN";
    EXPECT_THROW(field->region_direction(0, {INFINITY, 1}, dest), rust::Error) << "infinity";
    EXPECT_THROW(field->region_direction(0, {1e300, 1}, dest), rust::Error) << "far off the grid";
    EXPECT_THROW(field_over(*geo, 0.0), rust::Error) << "cell size";
    EXPECT_THROW(field_over(*geo, 1e-5), rust::Error) << "grid too large";

    auto graph = to_floorfield(*geo->region_graph_2d());
    graph.seams[0].index = 99;
    EXPECT_THROW(ff::new_multi_region_floorfield(graph, 0.1, 0.5), rust::Error) << "no such edge";
    graph = to_floorfield(*geo->region_graph_2d());
    graph.seams.truncate(graph.seams.size() - 1);
    EXPECT_THROW(ff::new_multi_region_floorfield(graph, 0.1, 0.5), rust::Error)
        << "one direction only";
}
