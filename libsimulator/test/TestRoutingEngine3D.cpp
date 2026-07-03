// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include <CGAL/mark_domain_in_triangulation.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <span>
#include <vector>

namespace
{
/// A single flat 10x10 square at z=0, split into two triangles (CCW).
SurfaceMesh unit_square_mesh()
{
    SurfaceMesh mesh{};
    const auto a = mesh.add_vertex({0, 0, 0});
    const auto b = mesh.add_vertex({10, 0, 0});
    const auto c = mesh.add_vertex({10, 10, 0});
    const auto d = mesh.add_vertex({0, 10, 0});
    mesh.add_face(a, b, c);
    mesh.add_face(a, c, d);
    return mesh;
}

/// A flat floor (z=0, y in [0,10]) joined at the seam y=10 to a 45-degree ramp
/// (z = y-10, y in [10,15]). The ramp is tilted, not vertical, so face_below's
/// -z ray projects onto it cleanly.
SurfaceMesh folded_mesh()
{
    SurfaceMesh mesh{};
    const auto v0 = mesh.add_vertex({0, 0, 0});
    const auto v1 = mesh.add_vertex({10, 0, 0});
    const auto v2 = mesh.add_vertex({10, 10, 0}); // seam
    const auto v3 = mesh.add_vertex({0, 10, 0}); // seam
    const auto v4 = mesh.add_vertex({10, 15, 5}); // ramp top
    const auto v5 = mesh.add_vertex({0, 15, 5}); // ramp top
    mesh.add_face(v0, v1, v2);
    mesh.add_face(v0, v2, v3);
    mesh.add_face(v3, v2, v4);
    mesh.add_face(v3, v4, v5);
    return mesh;
}

/// Build a SurfaceMesh from a 2D polygon by constrained-Delaunay triangulating it.
/// Default height => flat z=0.
SurfaceMesh mesh_from_polygon(const std::vector<K::Point_2>& outer, double z = 0.0)
{
    // Inspired by https://doc.cgal.org/latest/Mesh_2, example Mesh_2/mesh_marked_domain.cpp
    CDT cdt{};
    cdt.insert_constraint(outer.begin(), outer.end(), true); // true => closed
    CGAL::mark_domain_in_triangulation(cdt);

    SurfaceMesh mesh{};
    std::map<CDT::Vertex_handle, SurfaceMesh::Vertex_index> idx;
    const auto vertex_of = [&](CDT::Vertex_handle v) {
        const auto it = idx.find(v);
        if(it != idx.end()) {
            return it->second;
        }
        const auto p = v->point();
        return idx[v] = mesh.add_vertex({p.x(), p.y(), z});
    };
    for(auto f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); ++f) {
        if(f->get_in_domain()) {
            mesh.add_face(
                vertex_of(f->vertex(0)), vertex_of(f->vertex(1)), vertex_of(f->vertex(2)));
        }
    }
    return mesh;
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
    void SetUp() override { engine.set_geometry(unit_square_mesh()); }

protected:
    SurfaceMeshShortestPathRoutingEngine engine{};
};

TEST(RoutingEngine3D, ThrowsWithoutGeometry)
{
    SurfaceMeshShortestPathRoutingEngine engine{};
    EXPECT_THROW(engine.is_valid_location({5, 5, 1}), SimulationError);
}

TEST_F(FlatSquare, PointAboveSurfaceIsValid)
{
    // z above the surface: the -z ray of face_below projects down onto z=0.
    EXPECT_TRUE(engine.is_valid_location({5, 5, 1}));
    EXPECT_TRUE(engine.is_valid_location({0.5, 0.5, 100}));
}

TEST_F(FlatSquare, PointOutsideFootprintIsInvalid)
{
    EXPECT_FALSE(engine.is_valid_location({20, 20, 1}));
    EXPECT_FALSE(engine.is_valid_location({-1, 5, 1}));
}

TEST_F(FlatSquare, StraightPathCostIsEuclidean)
{
    // Both endpoints inside the lower-right triangle (y < x): single-face path.
    // --> euclidian distance in 2D (flat)
    const Point3D source{6, 2, 1};
    const Point3D target{9, 5, 1};
    engine.set_target(target);

    const auto [path, cost] = engine.get_shortest_path(source);

    EXPECT_NEAR(cost, std::sqrt(3. * 3. + 3. * 3.), 1e-6);
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
    engine.set_target(target);

    const auto [path, cost] = engine.get_shortest_path(source);

    EXPECT_NEAR(cost, std::sqrt(6. * 6. + 4. * 4.), 1e-6);
    // CGAL emits waypoints at each face edge it crosses, therefore not just 2 points returned.
    // But all points need to be collinear as it is a straight line.
    ASSERT_EQ(path.size(), 3u);
    EXPECT_TRUE(PointsCollinearXY(path));
}

TEST_F(FlatSquare, OrientationPointsToTarget)
{
    engine.set_target({9, 5, 1});

    const Point dir = engine.get_orientation({6, 2, 1});

    // Direction to the target (3, 3) normalized.
    const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    EXPECT_NEAR(dir.x, inv_sqrt2, 1e-6);
    EXPECT_NEAR(dir.y, inv_sqrt2, 1e-6);
}

TEST_F(FlatSquare, OrientationRobustWhenSourceOnEdge)
{
    // source sits exactly on the shared diagonal (y=x). CGAL then emits a
    // duplicate leading waypoint; get_orientation must skip it and still return
    // the real heading instead of a spurious (0,0).
    const Point3D source{4, 4, 1};
    engine.set_target({8, 7, 1});

    const Point dir = engine.get_orientation(source);

    // Heading towards (8,7) from (4,4): (4,3) normalized = (0.8, 0.6).
    EXPECT_NEAR(dir.x, 0.8, 1e-6);
    EXPECT_NEAR(dir.y, 0.6, 1e-6);
}

TEST(RoutingEngine3DFold, GeodesicCarriesLengthAcrossSeam)
{
    SurfaceMeshShortestPathRoutingEngine engine{};
    engine.set_geometry(folded_mesh());

    const Point3D source{3, 2, 1}; // on the floor
    const Point3D target{4, 13, 5}; // on the ramp, projects to z = 3
    engine.set_target(target);

    const auto [path, cost] = engine.get_shortest_path(source);

    // Unfold the ramp about the seam (y=10): the ramp point (4,13,3) lies at
    // surface distance (13-10)*sqrt(2) from the seam, so it maps to
    // (4, 10 + 3*sqrt(2)). The geodesic is the straight line to it.
    const double unfolded_y = 10.0 + 3.0 * std::sqrt(2.0);
    const double dx = 4.0 - 3.0;
    const double dy = unfolded_y - 2.0;
    EXPECT_NEAR(cost, std::sqrt(dx * dx + dy * dy), 1e-6);

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

TEST(RoutingEngine3DLShape, GeodesicBendsAroundReflexCorner)
{
    // L-shape (CCW) with a single reflex corner at (1, 1).
    const std::vector<K::Point_2> outer{{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 3}, {0, 3}};

    SurfaceMeshShortestPathRoutingEngine engine{};
    engine.set_geometry(mesh_from_polygon(outer)); // flat z = 0

    const Point3D source{2.5, 0.5, 1};
    const Point3D target{0.5, 2.5, 1};
    engine.set_target(target);

    const auto [path, cost] = engine.get_shortest_path(source);

    // Straight line would cut the missing quadrant (x>1, y>1), so the any-angle
    // geodesic must pivot on the reflex corner (1,1):
    //   |S->(1,1)| + |(1,1)->T| = sqrt(2.5) + sqrt(2.5) = 2*sqrt(2.5).
    EXPECT_NEAR(cost, 2.0 * std::sqrt(2.5), 1e-6);
    ASSERT_EQ(path.size(), 3u);
    EXPECT_NEAR(path[1].x(), 1.0, 1e-6);
    EXPECT_NEAR(path[1].y(), 1.0, 1e-6);
}
