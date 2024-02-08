// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "mesh.hpp"
#include "wkt.hpp"

#include <geos_c.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

const auto initGeos = []() -> bool {
    initGEOS(nullptr, nullptr);
    return true;
};

const bool geos_is_initialized = initGeos();

class MeshFixture : public ::testing::Test
{
protected:
    const std::string geometry_wkt =
        "GEOMETRYCOLLECTION (POLYGON ((10 5, 25 20, 25 25, 25 30, 30 30, 30 25, 30 20, 30 15, 30 "
        "5, 30 "
        "0, 25 0, 15 0, 10 0, 0 0, 0 5, 10 5), (25 5, 25 15, 15 5, 25 5)))";

    std::unique_ptr<Mesh> mesh{};

public:
    void SetUp() override
    {
        auto geo = std::make_unique<DrawableGEOS>(read_wkt(geometry_wkt));
        mesh = std::make_unique<Mesh>(geo->tri());
        mesh->MergeGreedy();
    }
};

TEST_F(MeshFixture, SimpleContains)
{
    ASSERT_NE(mesh->FindContainingPolygon({1, 1}), Mesh::Polygon::InvalidIndex);
    ASSERT_EQ(mesh->FindContainingPolygon({-1, 1}), Mesh::Polygon::InvalidIndex);
}
