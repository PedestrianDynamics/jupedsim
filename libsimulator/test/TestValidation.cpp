// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Validation.hpp"

#include <CGAL/boost/graph/helpers.h>
#include <gtest/gtest.h>

TEST(NormaliseAndValidateMesh, TriangulatesNonTriangleFaces)
{
    SurfaceMesh mesh{};
    const auto v0 = mesh.add_vertex(Point3D{0, 0, 0});
    const auto v1 = mesh.add_vertex(Point3D{10, 0, 0});
    const auto v2 = mesh.add_vertex(Point3D{10, 10, 0});
    const auto v3 = mesh.add_vertex(Point3D{0, 10, 0});
    mesh.add_face(v0, v1, v2, v3);

    NormaliseAndValidateMesh(mesh);

    EXPECT_TRUE(CGAL::is_triangle_mesh(mesh));
    EXPECT_EQ(mesh.number_of_faces(), 2u);
}
