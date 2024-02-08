// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "disjoint_set.hpp"
#include "wkt.hpp"

#include <glm/vec2.hpp>

#include <limits>
#include <vector>

class Mesh
{
public:
    struct Polygon {
        /// Represent invalid indices, e.g. neightbor index where no adjacent polygon is defined
        static constexpr size_t InvalidIndex{std::numeric_limits<size_t>::max()};
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
    std::vector<AABB> boundingBoxes{};

public:
    Mesh(const CDT& cdt);
    ~Mesh() = default;
    void MergeGreedy();
    std::vector<glm::vec2> FVertices() const;
    std::vector<uint16_t> TriangleIndices() const;
    std::vector<uint16_t> SegmentIndices() const;
    size_t FindContainingPolygon(const glm::vec2& p) const;
    glm::vec2 Vertex(size_t index) const;
    size_t CountVertices() const { return vertices.size(); }

private:
    Mesh(const Mesh& other) = delete;
    void operator=(const Mesh& other) = delete;
    Mesh(Mesh&& other) = delete;
    void operator=(Mesh&& other) = delete;
    void mergeDeadEnds(DisjointSet& djs);
    void smartMerge(bool keep_deadends);
    bool isValid() const;
    bool isConvex(const std::vector<size_t>& indices) const;
    bool tryMerge(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a);
    bool
    isMergable(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a);
    std::tuple<std::vector<size_t>, std::vector<size_t>>
    mergedPolygon(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a);
    double polygonArea(const std::vector<size_t> indices) const;
    bool polygonContains(const size_t, glm::vec2 p) const;
    void updateBoundingBoxes();
};
