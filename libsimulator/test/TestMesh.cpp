// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CfgCgal.hpp"
#include "Mesh.hpp"

#include <CGAL/mark_domain_in_triangulation.h>
#include <glm/vec2.hpp>
#include <gtest/gtest.h>

class SingleTriangeMesh : public ::testing::Test
{
public:
    void SetUp() override
    {
        CDT cdt{};
        // POLYGON ((-7 15, -7 -7, 23 -7, 23 0, 0 0, 0 15, -7 15))
        cdt.insert_constraint(K::Point_2{0, 0}, K::Point_2{1, 0});
        cdt.insert_constraint(K::Point_2{1, 0}, K::Point_2{0.5, 0.5});
        cdt.insert_constraint(K::Point_2{0.5, 0.5}, K::Point_2{0, 0});
        CGAL::mark_domain_in_triangulation(cdt);
        m = std::make_unique<Mesh>(cdt);
    }

protected:
    std::unique_ptr<Mesh> m{};
};

TEST_F(SingleTriangeMesh, CornerPointsAreConsideredInside)
{
    EXPECT_NE(m->FindContainingPolygon({0, 0}), Mesh::Polygon::InvalidIndex);
    EXPECT_NE(m->FindContainingPolygon({1, 0}), Mesh::Polygon::InvalidIndex);
    EXPECT_NE(m->FindContainingPolygon({0.5, 0.5}), Mesh::Polygon::InvalidIndex);
}

TEST_F(SingleTriangeMesh, PointsOnBoundaryAreInside)
{
    EXPECT_NE(m->FindContainingPolygon({0.5, 0}), Mesh::Polygon::InvalidIndex);
    EXPECT_NE(m->FindContainingPolygon({0.25, 0.25}), Mesh::Polygon::InvalidIndex);
    EXPECT_NE(m->FindContainingPolygon({0.75, 0.25}), Mesh::Polygon::InvalidIndex);
}

class DoubleBottleNeckMesh : public ::testing::Test
{
public:
    void SetUp() override
    {
        CDT cdt{};
        // POLYGON ((10 4.6, 10 0, 0 0, 0 10, 10 10, 10 5.4, 15 5.4, 15 10, 25 10, 25 5.4, 28 5.4,
        // 28 4.6, 25 4.6, 25 0, 15 0, 15 4.6, 10 4.6))
        std::vector<K::Point_2> boundary = {
            {10, 4.6},
            {10, 0},
            {0, 0},
            {0, 10},
            {10, 10},
            {10, 5.4},
            {15, 5.4},
            {15, 10},
            {25, 10},
            {25, 5.4},
            {28, 5.4},
            {28, 4.6},
            {25, 4.6},
            {25, 0},
            {15, 0},
            {15, 4.6},
            {10, 4.6}};

        for(size_t index = 0; index < boundary.size(); ++index) {
            cdt.insert_constraint(boundary[index], boundary[(index + 1) % boundary.size()]);
        }
        CGAL::mark_domain_in_triangulation(cdt);
        m = std::make_unique<Mesh>(cdt);
    }

protected:
    std::unique_ptr<Mesh> m{};
};

TEST_F(DoubleBottleNeckMesh, RegressionInContains)
{
    ASSERT_NE(
        m->FindContainingPolygon({26.690912185191067, 4.94908998002494}),
        Mesh::Polygon::InvalidIndex);
}
