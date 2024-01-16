// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "aabb.hpp"
#include "shader.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>

#include <OpenGL/OpenGL.h>
#include <geos_c.h>

#include <array>
#include <filesystem>
#include <memory>
#include <vector>

class DrawableGEOS
{
private:
    std::vector<std::vector<glm::vec2>> sequences{};
    std::vector<
        std::tuple<std::vector<glm::vec2>, std::vector<unsigned int>, std::vector<unsigned int>>>
        triangles{};
    std::vector<GLuint> vbos{};
    std::vector<GLuint> vaos{};
    AABB bounds{};

public:
    explicit DrawableGEOS(const GEOSGeometry* geo);
    ~DrawableGEOS();
    void Draw(Shader& shader) const;
    const AABB& Bounds() const { return bounds; };

private:
    AABB initBounds(const GEOSGeometry* geo);
};

const GEOSGeometry* read_wkt(std::filesystem::path file);
