// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Integration of the two 3D-neighborhood building blocks: the broad-phase grid
// (NeighborhoodSearch3D) narrowed by all three guards (horizontal radius,
// vertical band, and wall visibility via WallIndex). Milestone 2 built the wall
// query; this proves it composes into the correct neighbor set before a real
// model wires it in.
#include "NeighborhoodSearch3D.hpp"
#include "WallIndex.hpp"

#include <gtest/gtest.h>

#include <array>
#include <ranges>
#include <set>
#include <span>
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

/// The same [0,10]x[0,10] floor, but split at x=5. This tests whether neighborhood
/// works the same in "original" and "split" room as the split is not defined as a wall.
SurfaceMesh partitioned_flat_room()
{
    SurfaceMesh mesh{};
    const auto bl = mesh.add_vertex({0, 0, 0});
    const auto bm = mesh.add_vertex({5, 0, 0});
    const auto br = mesh.add_vertex({10, 0, 0});
    const auto tr = mesh.add_vertex({10, 10, 0});
    const auto tm = mesh.add_vertex({5, 10, 0});
    const auto tl = mesh.add_vertex({0, 10, 0});
    mesh.add_face(bl, bm, tm); // left half
    mesh.add_face(bl, tm, tl);
    mesh.add_face(bm, br, tr); // right half
    mesh.add_face(bm, tr, tm);
    return mesh;
}

SurfaceMesh two_rooms_with_gap()
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{0, 0, 0}, {4, 0, 0}, {4, 10, 0}, {0, 10, 0}});
    add_quad(mesh, {Point3D{6, 0, 0}, {10, 0, 0}, {10, 10, 0}, {6, 10, 0}});
    return mesh;
}

/// A bridge deck spanning x in [4,6], y in [0,10] at the given height.
SurfaceMesh bridge_at(double z)
{
    SurfaceMesh mesh{};
    add_quad(mesh, {Point3D{4, 0, z}, {6, 0, z}, {6, 10, z}, {4, 10, z}});
    return mesh;
}

/// broad filtering followed by all "narrowing" filters:
/// over-inclusive grid-based candidates
///     + filter horizontal with radius
///     + filter by height
///     + visibility check
std::set<std::size_t> neighbors(
    const NeighborhoodSearch3D& search,
    const WallIndex& walls,
    std::span<const Point3D> positions,
    const Point3D& p,
    double radius,
    double height)
{
    auto passes = [positions,
                   g1 = within_horizontal_distance(p, radius),
                   g2 = within_vertical_band(p, height),
                   g3 = walls.visible_from(p, height)](std::size_t i) {
        return g1(positions[i]) && g2(positions[i]) && g3(positions[i]);
    };
    auto matches = search.candidates(p, radius, height) | std::views::filter(passes);
    return {matches.begin(), matches.end()};
}

/// Ground truth ignoring walls: only the horizontal and vertical guards.
std::set<std::size_t>
oracle_no_walls(std::span<const Point3D> positions, const Point3D& p, double r, double h)
{
    auto filters =
        [positions, g1 = within_horizontal_distance(p, r), g2 = within_vertical_band(p, h)](
            std::size_t i) { return g1(positions[i]) && g2(positions[i]); };
    auto matches = std::views::iota(std::size_t{0}, positions.size()) | std::views::filter(filters);
    return {matches.begin(), matches.end()};
}
} // namespace

TEST(Neighbors3D, PartitionedFlatRoomMatchesUnpartitioned)
{
    std::vector<Point3D> positions{};
    for(int x = 1; x < 10; ++x) {
        for(int y = 1; y < 10; ++y) {
            positions.emplace_back(x * 1.0, y * 1.0, 0.0);
        }
    }
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);

    const WallIndex whole{flat_room()};
    const WallIndex split{partitioned_flat_room()};
    ASSERT_EQ(whole.segments().size(), 4);
    ASSERT_EQ(split.segments().size(), 6); // outer boundary only
    constexpr double radius = 2.5;
    constexpr double height = 2.2;

    for(const auto& q : {Point3D{2, 2, 0}, Point3D{5, 5, 0}, Point3D{5, 2, 0}, Point3D{8, 8, 0}}) {
        const auto n_whole = neighbors(search, whole, positions, q, radius, height);
        const auto n_split = neighbors(search, split, positions, q, radius, height);
        EXPECT_EQ(n_whole, n_split) << "query " << q;
        // No interior wall occludes anything, so both equal the wall-free oracle.
        EXPECT_EQ(n_whole, oracle_no_walls(positions, q, radius, height)) << "query " << q;
    }
}

TEST(Neighbors3D, WallBetweenRoomsRemovesCrossNeighbors)
{
    // 0,1 in the left room; 2,3 in the right room; the gap wall sits between.
    const std::vector<Point3D> positions{{3, 5, 0}, {3.5, 5, 0}, {7, 5, 0}, {7.5, 5, 0}};
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);
    const WallIndex walls{two_rooms_with_gap()};
    constexpr double radius = 5.0;
    constexpr double height = 2.2;

    // Radius 5 reaches across the gap.
    // Without walls all four would be mutual neighbors.
    EXPECT_EQ(
        oracle_no_walls(positions, positions[0], radius, height),
        (std::set<std::size_t>{0, 1, 2, 3}));
    // Adding the "visible_from"-filter drops the ones crossing the gap
    EXPECT_EQ(
        neighbors(search, walls, positions, positions[0], radius, height),
        (std::set<std::size_t>{0, 1}));
    EXPECT_EQ(
        neighbors(search, walls, positions, positions[2], radius, height),
        (std::set<std::size_t>{2, 3}));
}

TEST(Neighbors3D, BridgeAboveDoesNotSeparateGroundNeighbors)
{
    // Two agents on the ground; a bridge deck 3 m up spans between them.
    const std::vector<Point3D> positions{{3, 5, 0}, {7, 5, 0}};
    NeighborhoodSearch3D search{2.0};
    search.rebuild_index(positions);
    const WallIndex walls{bridge_at(3.0)};

    // The 2D line crosses the bridge borders, but they are 3 m above -> no block.
    EXPECT_EQ(
        neighbors(search, walls, positions, positions[0], 5.0, 2.2), (std::set<std::size_t>{0, 1}));
}
