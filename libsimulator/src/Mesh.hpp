// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AABB.hpp"
#include "CfgCgal.hpp"
#include "Clonable.hpp"

#include <glm/vec2.hpp>

#include <limits>
#include <memory>
#include <sstream>
#include <vector>

class Mesh : public Clonable<Mesh>
{
public:
    static constexpr size_t InvalidIndex{std::numeric_limits<size_t>::max()};
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
    explicit Mesh(const CDT& cdt);
    ~Mesh() override = default;
    Mesh(const Mesh& other) = default;
    Mesh& operator=(const Mesh& other) = default;
    Mesh(Mesh&& other) = default;
    Mesh& operator=(Mesh&& other) = default;
    std::unique_ptr<Mesh> Clone() const override;
    void MergeGreedy();
    std::vector<glm::vec2> FVertices() const;
    std::vector<uint16_t> TriangleIndices() const;
    std::vector<uint16_t> SegmentIndices() const;
    size_t FindContainingPolygon(const glm::dvec2& p) const;
    glm::dvec2 Vertex(size_t index) const;
    size_t CountVertices() const { return vertices.size(); }
    size_t CountPolygons() const { return polygons.size(); }
    std::stringstream IntoLibPolyanyaMeshDescription() const;
    const Mesh::Polygon& Polygons(size_t index) const { return polygons.at(index); }
    const AABB& AxisAlignedBoundingBox(size_t index) const { return boundingBoxes.at(index); }
    bool TriangleContains(const size_t, glm::dvec2 p) const;

private:
    void mergeDeadEnds();
    void smartMerge(bool keep_deadends);
    bool isValid() const;
    bool polygonIsConvex(const std::vector<size_t>& indices) const;
    bool tryMerge(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a);
    bool
    isMergable(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a);
    std::tuple<std::vector<size_t>, std::vector<size_t>>
    mergedPolygon(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a);
    double polygonArea(const std::vector<size_t> indices) const;
    void trimEmptyPolygons();
    void updateBoundingBoxes();
};
