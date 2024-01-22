// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "mesh.hpp"
#include "disjoint_set.hpp"
#include "wkt.hpp"

#include <CGAL/number_utils.h>

#include <_types/_uint16_t.h>
#include <algorithm>
#include <vector>

template <typename T>
T into(const Point& p)
{
    return {CGAL::to_double(p.x()), CGAL::to_double(p.y())};
};

Mesh::Mesh(const CDT& cdt)
{
    vertices.resize(cdt.finite_vertex_handles().size());
    for(const auto& v : cdt.finite_vertex_handles()) {
        vertices.at(v->info()) = into<glm::dvec2>(v->point());
    }
    polygons.reserve(cdt.finite_face_handles().size());

    std::unordered_map<CDT::Face_handle, size_t> handle_map{};
    std::vector<std::vector<CDT::Face_handle>> neighbor_face_handles{};
    neighbor_face_handles.reserve(polygons.capacity());
    for(const CDT::Face_handle& t : cdt.finite_face_handles()) {
        if(!t->get_in_domain()) {
            continue;
        }
        Polygon p{};
        p.vertices.reserve(3);
        p.vertices.push_back(t->vertex(0)->info());
        p.vertices.push_back(t->vertex(1)->info());
        p.vertices.push_back(t->vertex(2)->info());

        polygons.push_back(p);
        neighbor_face_handles.emplace_back();
        for(int index = 0; index < 3; ++index) {
            const auto& n = t->neighbor(index);
            if(n->get_in_domain()) {
                neighbor_face_handles.back().emplace_back(n);
            }
        }
        handle_map[t] = polygons.size() - 1;
    }
    assert(polygons.size() == neighbor_face_handles.size());
    for(size_t index = 0; index < polygons.size(); ++index) {
        auto& p = polygons[index];
        const auto& neighbors = neighbor_face_handles[index];
        p.neighbors.reserve(neighbors.size());
        for(const auto& n : neighbors) {
            p.neighbors.emplace_back(handle_map.at(n));
        }
    }
};

void Mesh::MergeGreedy()
{
    DisjointSet djs{polygons.size()};
    // 1) Merge polygons with only one nieghbor, aka "dead-ends"
    mergeDeadEnds(djs);
    // 2) "Smart" merge remaining polygons
    // 3) Validate correctness
}

void Mesh::mergeDeadEnds(DisjointSet& djs)
{
    while(true) {
        for(auto&& p : polygons) {
            if(p.neighbors.size() != 1) {
                continue;
            }
        }
    }
}

void Mesh::smartMerge()
{
}

bool Mesh::isValid() const
{
    return true;
}

std::vector<glm::vec2> Mesh::FVertices() const
{
    std::vector<glm::vec2> f_vertices{};
    f_vertices.reserve(vertices.size());
    std::transform(
        std::begin(vertices),
        std::end(vertices),
        std::back_inserter(f_vertices),
        [](const auto& v) {
            return glm::vec2{v.x, v.y};
        });
    return f_vertices;
}

std::vector<uint16_t> Mesh::TriangleIndices() const
{
    std::vector<uint16_t> indices{};
    indices.reserve(polygons.size() * 3);

    for(const auto& p : polygons) {
        const uint16_t first = p.vertices.front();
        const auto count_indices = p.vertices.size();
        for(size_t index = 2; index < count_indices; ++index) {
            indices.push_back(first);
            indices.push_back(p.vertices[index - 1]);
            indices.push_back(p.vertices[index]);
        }
    }
    return indices;
}

std::vector<uint16_t> Mesh::SegmentIndices() const
{
    using segment = std::tuple<uint16_t, uint16_t>;
    const auto make_segment = [](uint16_t a, uint16_t b) {
        if(a < b) {
            return std::make_tuple(a, b);
        }
        return std::make_tuple(b, a);
    };

    std::set<segment> segments{};

    for(const auto& p : polygons) {
        const auto vertex_count = p.vertices.size();
        uint16_t a = p.vertices[vertex_count - 1];
        for(size_t index = 0; index < vertex_count; ++index) {
            uint16_t b = p.vertices[index];
            segments.insert(make_segment(a, b));
            a = b;
        }
    }

    std::vector<uint16_t> indices{};
    indices.reserve(segments.size() * 2);
    for(const auto& s : segments) {
        indices.emplace_back(std::get<0>(s));
        indices.emplace_back(std::get<1>(s));
    }
    return indices;
}
