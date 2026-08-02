// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"
#include "Geometry/RegionSplit.hpp"
#include "Geometry/WallMerge.hpp"
#include "GeometryBuilder.hpp"
#include "MeshFixtures.hpp"

#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/boost/graph/iterator.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace
{
using namespace fixtures;

struct Merged {
    SurfaceMesh mesh;
    std::vector<MergedWall> walls;
    std::size_t region_count;
};

Merged merge(SurfaceMesh mesh)
{
    const auto split = split_into_regions(mesh);
    auto walls = merge_border_walls(mesh, split.region, wall_merge_tolerance(mesh));
    return Merged{std::move(mesh), std::move(walls), split.count};
}

double length(const MergedWall& w)
{
    return (w.segment.p2 - w.segment.p1).Norm();
}

/// The x extents of all walls lying on a given y, sorted. The fixtures are laid out so that
/// one such side carries the whole shape, which is where fusing shows up.
std::vector<std::pair<double, double>> extents_along(const std::vector<MergedWall>& walls, double y)
{
    std::vector<std::pair<double, double>> extents{};
    for(const auto& w : walls) {
        if(w.segment.p1.y == y && w.segment.p2.y == y) {
            extents.emplace_back(
                std::min(w.segment.p1.x, w.segment.p2.x), std::max(w.segment.p1.x, w.segment.p2.x));
        }
    }
    std::sort(extents.begin(), extents.end());
    return extents;
}

/// The wall running the full length of the far side at z = 3, across landing and upper
/// floor. It is the one the region boundary can cut.
std::vector<MergedWall>::const_iterator find_far_side_wall(const std::vector<MergedWall>& walls)
{
    return std::find_if(walls.begin(), walls.end(), [](const MergedWall& w) {
        return w.segment.p1.y == 8.0 && w.segment.p2.y == 8.0 &&
               std::abs(w.segment.p2.x - w.segment.p1.x) == 18.0;
    });
}

} // namespace

TEST(WallMerge, CollinearRunBecomesOneWall)
{
    const auto m = merge(flat_rectangle());

    // Four sides. The mid vertex subdividing them must not survive as an extra wall -- the
    // models add up repulsion per wall, so it would make the force depend on how finely the
    // mesh happens to be cut.
    ASSERT_EQ(m.walls.size(), 4u);

    std::vector<double> lengths{};
    for(const auto& w : m.walls) {
        lengths.push_back(length(w));
    }
    std::sort(lengths.begin(), lengths.end());
    EXPECT_EQ(lengths, (std::vector<double>{10.0, 10.0, 10.0, 10.0}));
}

TEST(WallMerge, IndicesAreDenseAndUnique)
{
    const auto m = merge(flat_rectangle());

    std::set<std::uint32_t> indices{};
    for(const auto& w : m.walls) {
        indices.insert(w.index);
    }
    EXPECT_EQ(indices.size(), m.walls.size());
    EXPECT_EQ(*indices.begin(), 0u);
    EXPECT_EQ(*indices.rbegin(), m.walls.size() - 1);
}

TEST(WallMerge, EveryWallKnowsItsRegion)
{
    const auto m = merge(flat_rectangle());

    ASSERT_EQ(m.region_count, 1u);
    for(const auto& w : m.walls) {
        ASSERT_EQ(w.regions.size(), 1u);
        EXPECT_EQ(w.regions.front(), 0u);
    }
}

TEST(WallMerge, RunTurningBackOnItselfIsNotFused)
{
    const auto m = merge(two_levels_with_stair());
    ASSERT_EQ(m.region_count, 2u);

    // The corridor wall runs out along y = 0 from x = 0 to the stair head at x = 15, then
    // back to x = 5 on the upper level. Lower level and flight are one wall: the climb does
    // not matter to a model that repels horizontally. The turn at the stair head does -- a
    // chord from x = 0 to x = 5 would claim the wall stops there.
    EXPECT_EQ(
        extents_along(m.walls, 0.0),
        (std::vector<std::pair<double, double>>{{0.0, 15.0}, {5.0, 15.0}}));
}

TEST(WallMerge, UndulatingTerrainKeepsAOnePieceOutline)
{
    const auto m = merge(wavy_terrain());

    // Straight in plan, never straight in 3D. Judging the bend in 3D would give one wall per
    // mesh edge -- ten a side here, and as many as the mesh is fine anywhere else -- which is
    // the resolution-dependent repulsion the merge exists to prevent.
    ASSERT_EQ(m.region_count, 1u);
    EXPECT_EQ(m.walls.size(), 4u);
    EXPECT_EQ(extents_along(m.walls, 0.0), (std::vector<std::pair<double, double>>{{0.0, 20.0}}));
}

TEST(WallMerge, WallsFollowTheLevelTheyBelongTo)
{
    const auto m = merge(two_levels_with_stair());

    // Lower corridor and stair form one region, the overhanging upper corridor the other.
    // A wall belongs to the level whose surface touches it, which is what lets an agent
    // downstairs be given the walls downstairs.
    std::set<std::size_t> regions{};
    for(const auto& w : m.walls) {
        ASSERT_EQ(w.regions.size(), 1u);
        regions.insert(w.regions.front());
    }
    EXPECT_EQ(regions.size(), 2u);
}

TEST(WallMerge, StackedFloorsNeverShareAWall)
{
    const auto m = merge(stacked_floors());

    ASSERT_EQ(m.region_count, 2u);
    // Four sides per floor. Their border edges are identical once projected, so rebuilding
    // the chains from 2D geometry would join them; walking the mesh border cannot.
    EXPECT_EQ(m.walls.size(), 8u);
    for(const auto& w : m.walls) {
        EXPECT_EQ(w.regions.size(), 1u);
    }
}

TEST(WallMerge, WallCrossingARegionBoundaryStaysOneWall)
{
    const auto m = merge(switchback_stair());
    ASSERT_EQ(m.region_count, 2u);

    // Landing and upper floor lie at the same height and their y = 8 edges continue each
    // other, so this is one straight wall from x = 0 to x = 18 -- and the region flood cuts
    // it in the middle. Breaking the wall there would make the repulsion depend on where the
    // overlay happened to draw its line.
    const auto crossing = find_far_side_wall(m.walls);
    ASSERT_NE(crossing, m.walls.end()) << "the wall across the region boundary was split";

    // Spanning the boundary means it borders both regions, and has to be findable from
    // either side.
    EXPECT_EQ(crossing->regions.size(), 2u);
}

TEST(WallMerge, WallsDoNotMoveWhenTheRegionCutDoes)
{
    const auto ground_first = merge(switchback_stair(false));
    const auto upper_first = merge(switchback_stair(true));
    ASSERT_EQ(ground_first.region_count, 2u);
    ASSERT_EQ(upper_first.region_count, 2u);

    // The flood is greedy from an arbitrary seed, and the two overlapping ends are separated
    // by pieces that overlap neither -- so feeding the same geometry in the other order cuts
    // it somewhere else. Seeded at the ground floor the cut falls between landing and upper
    // floor, which is across the far wall; seeded at the upper floor it falls inside the
    // ground floor, leaving that wall untouched.
    const auto ground_crossing = find_far_side_wall(ground_first.walls);
    const auto upper_crossing = find_far_side_wall(upper_first.walls);
    ASSERT_NE(ground_crossing, ground_first.walls.end());
    ASSERT_NE(upper_crossing, upper_first.walls.end());
    ASSERT_EQ(ground_crossing->regions.size(), 2u);
    ASSERT_EQ(upper_crossing->regions.size(), 1u)
        << "the cut no longer moves with build order, so this fixture tests nothing";

    // The walls themselves must not care. That is what the merge exists for: geometry
    // decides what a wall is, the overlay does not.
    const auto geometry = [](const Merged& m) {
        std::set<std::array<double, 4>> segments{};
        for(const auto& w : m.walls) {
            const std::array<double, 4> s{
                w.segment.p1.x, w.segment.p1.y, w.segment.p2.x, w.segment.p2.y};
            const std::array<double, 4> flipped{s[2], s[3], s[0], s[1]};
            segments.insert(std::min(s, flipped));
        }
        return segments;
    };
    EXPECT_EQ(geometry(ground_first), geometry(upper_first));
}

namespace
{
SurfaceMesh copy_mesh(const SurfaceMesh& mesh)
{
    SurfaceMesh out{};
    std::vector<SurfaceMesh::Vertex_index> moved(mesh.number_of_vertices());
    for(const auto v : mesh.vertices()) {
        moved[static_cast<std::size_t>(v)] = out.add_vertex(mesh.point(v));
    }
    for(const auto f : mesh.faces()) {
        std::vector<SurfaceMesh::Vertex_index> corners{};
        for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            corners.push_back(moved[static_cast<std::size_t>(v)]);
        }
        out.add_face(corners);
    }
    return out;
}

/// Cut every boundary edge down to @p target, leaving the outline exactly where it was.
/// Without this there would be nothing for the merge to put back together.
void refine_boundary(SurfaceMesh& mesh, double target)
{
    std::vector<SurfaceMesh::Edge_index> border{};
    for(const auto e : mesh.edges()) {
        if(mesh.is_border(mesh.halfedge(e)) || mesh.is_border(mesh.opposite(mesh.halfedge(e)))) {
            border.push_back(e);
        }
    }
    CGAL::Polygon_mesh_processing::split_long_edges(border, target, mesh);
}

std::set<std::array<double, 4>> as_directed_segments(const std::vector<MergedWall>& walls)
{
    std::set<std::array<double, 4>> out{};
    for(const auto& w : walls) {
        out.insert({w.segment.p1.x, w.segment.p1.y, w.segment.p2.x, w.segment.p2.y});
    }
    return out;
}
} // namespace

TEST(WallMerge, FusedWallsAreTheOnesThePolygonPathWouldGive)
{
    // A room with a pillar, lifted to a mesh and then cut far finer along its boundary than
    // the polygon was ever drawn. Fusing has to give the polygon's own walls back -- every
    // one of them, and especially each with its ends the same way round. If the direction
    // changes, the closest point to an agent might flip in the last bit.
    GeometryBuilder b{};
    b.AddAccessibleArea({{0, 0}, {20, 0}, {20, 20}, {0, 20}});
    b.ExcludeFromAccessibleArea({{9, 9}, {11, 9}, {11, 11}, {9, 11}});
    const Geometry3D geo{b.Build().Polygon()};

    auto mesh = copy_mesh(geo.mesh());
    refine_boundary(mesh, 0.25);
    const auto split = split_into_regions(mesh);
    const auto fused = merge_border_walls(mesh, split.region, wall_merge_tolerance(mesh));

    std::set<std::array<double, 4>> expected{};
    for(const auto& s : geo.geometry_2d()->LineSegmentsInDistanceTo(1e6, Point{10, 10})) {
        expected.insert({s.p1.x, s.p1.y, s.p2.x, s.p2.y});
    }
    ASSERT_EQ(expected.size(), 8u) << "four for the room, four for the pillar";
    EXPECT_EQ(as_directed_segments(fused), expected);
}
