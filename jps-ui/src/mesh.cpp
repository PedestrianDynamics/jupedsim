// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "mesh.hpp"
#include "disjoint_set.hpp"
#include "wkt.hpp"

#include <CGAL/number_utils.h>

#include <algorithm>
#include <iterator>
#include <optional>
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
    std::vector<std::vector<std::optional<CDT::Face_handle>>> neighbor_face_handles{};
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
            const auto& n = t->neighbor(CDT::cw(index));
            if(n->get_in_domain()) {
                neighbor_face_handles.back().emplace_back(n);
            } else {
                neighbor_face_handles.back().emplace_back(std::nullopt);
            }
        }
        handle_map[t] = polygons.size() - 1;
    }
    assert(polygons.size() == neighbor_face_handles.size());
    for(size_t index = 0; index < polygons.size(); ++index) {
        auto& p = polygons[index];
        const auto& neighbors = neighbor_face_handles[index];
        p.neighbors.reserve(neighbors.size());
        for(const auto& opt : neighbors) {
            if(opt) {
                p.neighbors.emplace_back(handle_map.at(*opt));
            } else {
                p.neighbors.emplace_back(Polygon::InvalidIndex);
            }
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
    std::vector<bool> merged_polygons(polygons.size(), false);
    bool merged = false;
    do {
        merged = false;
        size_t merge_candidate{};
        size_t merge_target{};
        for(size_t index; index < polygons.size() && !merged; ++index) {
            if(merged_polygons[index]) {
                continue;
            }
            const auto& p = polygons[index];
            const auto num_valid_neigbors =
                std::count_if(std::begin(p.neighbors), std::end(p.neighbors), [](const auto& idx) {
                    return idx != Polygon::InvalidIndex;
                });
            if(num_valid_neigbors != 1) {
                continue;
            }

            const auto firstValidNeighbor = [](const auto& p) {
                for(size_t nth_neighbor = 0; nth_neighbor < p.neighbors.size(); ++nth_neighbor) {
                    if(p.neighbors[nth_neighbor] != Polygon::InvalidIndex) {
                        return nth_neighbor;
                    }
                }
                return Polygon::InvalidIndex;
            };

            const auto valid_neighbor = firstValidNeighbor(p);
            merge_candidate = p.neighbors[valid_neighbor];
            merge_target = index;
            merged = tryMerge(merge_target, merge_candidate, valid_neighbor);
        }

        if(merged) {
            polygons[merge_candidate].neighbors.clear();
            polygons[merge_candidate].vertices.clear();
            merged_polygons[merge_candidate] = true;
            for(size_t index = 0; index < polygons.size(); ++index) {
                if(merged_polygons[index]) {
                    continue;
                }
                auto& polygon = polygons[index];
                std::replace(
                    std::begin(polygon.neighbors),
                    std::end(polygon.neighbors),
                    merge_candidate,
                    merge_target);
            }
        }

    } while(merged);
}

void Mesh::smartMerge()
{
}

bool Mesh::isValid() const
{
    return true;
}

bool Mesh::isConvex(const std::vector<size_t>& indices) const
{
    const auto prev = [this, &indices](size_t index) -> size_t {
        const auto count = indices.size();
        assert(count >= 3);
        return (count + index - 1) % count;
    };
    const auto next = [this, &indices](size_t index) -> size_t {
        const auto count = indices.size();
        assert(count >= 3);
        return (index + 1) % count;
    };

    for(size_t index = 0; index < indices.size(); ++index) {
        const auto segment_a = vertices[index] - vertices[prev(index)];
        const auto segment_b = vertices[next(index)] - vertices[index];
        const auto cp = glm::cross(glm::dvec3(segment_a, 0), glm::dvec3(segment_b, 0));
        if(cp.z > 0.0) {
            // This indicates CW winding between consecutive segments
            return false;
        }
    }
    return true;
}

bool Mesh::tryMerge(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a)
{
    auto& polygon_a = polygons[polygon_a_index];
    const auto& polygon_b = polygons[polygon_b_index];
    const auto new_vertex_count = polygon_a.vertices.size() + polygon_b.vertices.size() - 2;

    std::vector<size_t> indices{};
    indices.reserve(new_vertex_count);
    const auto iter_b = std::find(
        std::begin(polygon_b.vertices),
        std::end(polygon_b.vertices),
        polygon_a.vertices[first_common_vertex_in_a]);
    assert(iter != std::end(polygon_b.vertices));
    const auto vertex_in_b = *iter_b;
    for(size_t index; index < polygon_b.vertices.size(); ++index) {
        vertices.emplace_back(
            polygon_b.vertices[(index + vertex_in_b) % polygon_b.vertices.size()]);
    }
    for(size_t index; index < polygon_a.vertices.size() - 2; ++index) {
        vertices.emplace_back(
            polygon_a.vertices[(index + first_common_vertex_in_a + 2) % polygon_a.vertices.size()]);
    }

    indices.insert(std::end(indices), iter_b, std::end(polygon_b.vertices));
    indices.insert(std::end(indices), std::begin(polygon_b.vertices), iter_b);
    auto iter_a = std::begin(polygon_a.vertices);
    std::advance(iter_a, first_common_vertex_in_a);
    indices.insert(std::end(indices), iter_a, std::end(polygon_a.vertices));
    indices.insert(std::end(indices), std::begin(polygon_a.vertices), iter_a);

    return false;
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
        if(p.vertices.size() < 3) {
            continue;
        }
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
        if(vertex_count < 3) {
            continue;
        }
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
