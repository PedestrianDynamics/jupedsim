// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "disjoint_set.hpp"
#include "wkt.hpp"

#include <glm/vec2.hpp>

#include <vector>

class Mesh
{
public:
    struct Polygon {
        /// Index into the vertices store of the Mesh
        std::vector<size_t> vertices{};
        /// Index into the polygon store of the Mesh
        std::vector<size_t> neighbors{};
    };

private:
    /// All vertices in this Mesh
    std::vector<glm::dvec2> vertices{};
    /// All convex polygons in this Mesh in CCW orientation.
    std::vector<Polygon> polygons{};

public:
    Mesh(const CDT& cdt);
    ~Mesh() = default;
    void MergeGreedy();
    std::vector<glm::vec2> FVertices() const;
    std::vector<uint16_t> TriangleIndices() const;
    std::vector<uint16_t> SegmentIndices() const;

private:
    Mesh(const Mesh& other) = delete;
    void operator=(const Mesh& other) = delete;
    Mesh(Mesh&& other) = delete;
    void operator=(Mesh&& other) = delete;
    void mergeDeadEnds(DisjointSet& djs);
    void smartMerge();
    bool isValid() const;
};
