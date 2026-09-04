// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "Geometry/ProjectedBoundary.hpp"
#include "MeshFixtures.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <vector>

namespace
{
/// Flat 2x2 grid of unit cells, each cell split into two triangles. Faces are
/// added cell by cell: cell (cx, cy) owns faces 2*(2*cy+cx) and 2*(2*cy+cx)+1.
SurfaceMesh flat_two_by_two_grid()
{
    SurfaceMesh mesh{};
    std::array<std::array<SurfaceMesh::Vertex_index, 3>, 3> v{};
    for(int y = 0; y < 3; ++y) {
        for(int x = 0; x < 3; ++x) {
            v[y][x] = mesh.add_vertex({static_cast<double>(x), static_cast<double>(y), 0});
        }
    }
    for(int cy = 0; cy < 2; ++cy) {
        for(int cx = 0; cx < 2; ++cx) {
            mesh.add_face(v[cy][cx], v[cy][cx + 1], v[cy + 1][cx + 1]);
            mesh.add_face(v[cy][cx], v[cy + 1][cx + 1], v[cy + 1][cx]);
        }
    }
    return mesh;
}

/// A 4x4 square with a vertical slit cut from (2,0) up to (2,2): the slit's rim
/// uses an unwelded duplicate of the bottom vertex, so the border runs up one
/// side of the slit and straight back down the other -- a zero-angle fold.
SurfaceMesh slit_square()
{
    SurfaceMesh mesh{};
    const auto a = mesh.add_vertex({0, 0, 0});
    const auto b = mesh.add_vertex({2, 0, 0});
    const auto c = mesh.add_vertex({4, 0, 0});
    const auto d = mesh.add_vertex({4, 4, 0});
    const auto e = mesh.add_vertex({0, 4, 0});
    const auto b2 = mesh.add_vertex({2, 0, 0}); // unwelded duplicate of b
    const auto s = mesh.add_vertex({2, 2, 0}); // slit tip
    mesh.add_face(b, c, s); // right of the slit
    mesh.add_face(c, d, s);
    mesh.add_face(a, b2, s); // left of the slit
    mesh.add_face(a, s, e);
    mesh.add_face(s, d, e); // above the slit
    return mesh;
}

std::vector<SurfaceMesh::Halfedge_index> whole_mesh_boundary(const SurfaceMesh& mesh)
{
    return region_boundary(mesh, [](SurfaceMesh::Face_index) { return true; });
}
} // namespace

TEST(ProjectedBoundary, SimpleSquareIsStrictlySimple)
{
    const auto mesh = fixtures::flat_square();
    const auto boundary = whole_mesh_boundary(mesh);
    ASSERT_EQ(boundary.size(), 4u);
    EXPECT_TRUE(is_projection_strictly_simple(mesh, boundary));
}

TEST(ProjectedBoundary, RingWithHoleIsStrictlySimple)
{
    // Two disjoint simple loops (outer square + hole) are fine.
    const auto mesh = fixtures::flat_rectangle();
    EXPECT_TRUE(is_projection_strictly_simple(mesh, whole_mesh_boundary(mesh)));
}

TEST(ProjectedBoundary, CollinearContinuationIsStrictlySimple)
{
    // flat_rectangle subdivides its long sides with a collinear mid vertex: consecutive
    // boundary segments continue in a straight line, which is NOT a violation.
    const auto mesh = fixtures::flat_rectangle();
    const auto boundary = whole_mesh_boundary(mesh);
    ASSERT_GT(boundary.size(), 4u); // the collinear subdivision is actually present
    EXPECT_TRUE(is_projection_strictly_simple(mesh, boundary));
}

TEST(ProjectedBoundary, PinchAtSharedVertexIsRejected)
{
    // Keep only the two diagonal cells of a 2x2 grid: their union touches itself at the
    // grid's centre vertex. The boundary visits that vertex twice (degree 4) -- under the
    // strict certificate a pinch is a violation even though the vertex is genuinely shared.
    const auto mesh = flat_two_by_two_grid();
    const auto keep = [](SurfaceMesh::Face_index f) {
        const auto cell = static_cast<std::size_t>(f) / 2;
        return cell == 0 || cell == 3; // cells (0,0) and (1,1)
    };
    EXPECT_FALSE(is_projection_strictly_simple(mesh, region_boundary(mesh, keep)));
}

TEST(ProjectedBoundary, ZeroAngleSpikeIsRejected)
{
    // The slit square's border folds back on itself along the slit: two consecutive
    // boundary segments meet at the slit tip at a zero angle.
    const auto mesh = slit_square();
    EXPECT_FALSE(is_projection_strictly_simple(mesh, whole_mesh_boundary(mesh)));
}

TEST(ProjectedBoundary, FoldedSurfaceBoundaryIsRejected)
{
    // two_levels_with_stair folds the upper corridor back over the lower one: the projected
    // boundary of the whole surface crosses itself, so the certificate must reject it.
    const auto mesh = fixtures::two_levels_with_stair();
    EXPECT_FALSE(is_projection_strictly_simple(mesh, whole_mesh_boundary(mesh)));
}

TEST(ProjectedBoundary, ForeignPointTouchIsRejected)
{
    // A boundary vertex of one lobe lands in the interior of another lobe's boundary
    // segment (a T-touch with no shared mesh vertex).
    SurfaceMesh mesh{};
    const auto a0 = mesh.add_vertex({0, 0, 0});
    const auto a1 = mesh.add_vertex({10, 0, 0});
    const auto a2 = mesh.add_vertex({10, 2, 0});
    const auto a3 = mesh.add_vertex({0, 2, 0});
    mesh.add_face(a0, a1, a2); // floor
    mesh.add_face(a0, a2, a3);
    const auto w = mesh.add_vertex({8, 2, 1}); // over the interior of edge y=2
    const auto b3 = mesh.add_vertex({12, 3, 1});
    const auto b4 = mesh.add_vertex({12, 4, 1});
    mesh.add_face(w, b3, b4); // detached wing, tip touching the floor edge in (x, y)
    EXPECT_FALSE(is_projection_strictly_simple(mesh, whole_mesh_boundary(mesh)));
}
