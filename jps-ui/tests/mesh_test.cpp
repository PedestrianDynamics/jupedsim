// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "mesh.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MeshFixture : ::testing::Test
{
    const std::string geometry_wkt =
        "GEOMETRYCOLLECTION (POLYGON ((10 5, 25 20, 25 25, 25 30, 30 30, 30 25, 30 20, 30 15, 30 "
        "5, 30 "
        "0, 25 0, 15 0, 10 0, 0 0, 0 5, 10 5), (25 5, 25 15, 15 5, 25 5)))";

    Mesh mesh;

    void SetUp()
    {
        auto geo = std::make_unique<DrawableGEOS>(wkt);
        mesh(geo->tri());
        mesh.MergeGreedy();
    }
}

TEST_F(MeshFixture, SimpleContains)
{
    ASSERT_TRUE(mesh.polygonContains())
}
