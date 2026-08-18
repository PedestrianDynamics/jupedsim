// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/BoundaryIndex.hpp"
#include "Geometry/Geometry3D.hpp"
#include "Geometry/RegionSplit.hpp"
#include "MeshFixtures.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

struct TestBoundaryIndexTwoLevelsWithStairs : public testing::Test {
    SurfaceMesh mesh;
    RegionSplit split;
    std::unique_ptr<Geometry3D> geo;

    void SetUp() override
    {
        mesh = fixtures::switchback_stair();
        // TODO(kkratz): Investigate segfault if region split is applied before handing mesh to
        // Geometry3D.
        geo = std::make_unique<Geometry3D>(mesh);
        split = split_into_regions(mesh);
    }

    void TearDown() override {}
};

TEST_F(TestBoundaryIndexTwoLevelsWithStairs, CanConstruct)
{
    std::unique_ptr<BoundaryIndex> bi = MakeNaiveBoundaryIndex(mesh, split);
}

TEST_F(TestBoundaryIndexTwoLevelsWithStairs, Neighboors1)
{
    std::unique_ptr<BoundaryIndex> sut = MakeNaiveBoundaryIndex(mesh, split);
    
    auto loc = geo->get_location(2,2,0).value();
    auto res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 11);
    
    loc = geo->get_location(16,6,3).value();
    res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 11);
}

TEST_F(TestBoundaryIndexTwoLevelsWithStairs, Neighboors2)
{
    std::unique_ptr<BoundaryIndex> sut = MakeNaiveBoundaryIndex(mesh, split);
    
    auto loc = geo->get_location(2,2,0).value();
    auto res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 11);
    
    loc = geo->get_location(16,6,3).value();
    res = sut->Query(loc, 100); // Ensure we get everything.
    EXPECT_EQ(res.size(), 11);
}


#include <CGAL/IO/polygon_mesh_io.h>
TEST_F(TestBoundaryIndexTwoLevelsWithStairs, WriteMesh)
{
    ASSERT_TRUE(
        CGAL::IO::write_polygon_mesh(
            "example.obj", mesh, CGAL::parameters::verbose(true)));
    std::unique_ptr<BoundaryIndex> bi = MakeNaiveBoundaryIndex(mesh, split);
}
