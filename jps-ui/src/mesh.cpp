// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "mesh.hpp"
#include "disjoint_set.hpp"
#include "wkt.hpp"

#include <CGAL/number_utils.h>

#include <algorithm>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <format>
#include <queue>

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

    updateBoundingBoxes();
};

void Mesh::MergeGreedy()
{
    DisjointSet djs{polygons.size()};
    // 1) Merge polygons with only one nieghbor, aka "dead-ends"
    mergeDeadEnds(djs);
    // 2) "Smart" merge remaining polygons
    smartMerge(true);
    // 3) Validate correctness

    updateBoundingBoxes();
}

void Mesh::mergeDeadEnds(DisjointSet& djs)
{
    std::vector<bool> merged_polygons(polygons.size(), false);
    bool merged = false;
    do {
        merged = false;
        size_t merge_candidate{};
        size_t merge_target{};
        for(size_t index = 0; index < polygons.size() && !merged; ++index) {
            if(merged_polygons[index]) {
                continue;
            }
            const auto& p = polygons[index];

            auto isValidNeighbor = [index](const auto& idx) {
                if((idx != Polygon::InvalidIndex) && (idx != index)) {
                    return true;
                }
                return false;
            };

            const auto num_valid_neigbors =
                std::count_if(std::begin(p.neighbors), std::end(p.neighbors), isValidNeighbor);

            if(num_valid_neigbors != 1) {
                continue;
            }

            const auto neighbor =
                std::find_if(std::begin(p.neighbors), std::end(p.neighbors), isValidNeighbor);
            assert(neighbor != std::end(p.neighbors));
            const auto valid_neighbor = std::distance(std::begin(p.neighbors), neighbor);
            merge_candidate = p.neighbors[valid_neighbor];

            merge_target = index;

            // due to data structure valid_neighbor is also first_common_vertex_in_a
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

double Mesh::polygonArea(const std::vector<size_t> indices) const
{
    const auto next = [&indices](size_t index) -> size_t {
        const auto count = indices.size();
        assert(count >= 3);
        return (index + 1) % count;
    };

    double area = 0.;

    for(size_t index = 0; index < indices.size(); ++index) {
        const auto& current_vertex = vertices[indices[index]];
        const auto& next_vertex = vertices[indices[next(index)]];

        area += (current_vertex.x * next_vertex.y) - (current_vertex.y - next_vertex.x);
    }
    area = std::abs(area) / 2.0;
    return area;
}

void Mesh::smartMerge(bool keep_deadends = true)
{
    constexpr double InvalidArea{std::numeric_limits<double>::lowest()};

    struct SearchNode {

        size_t source;
        double area;

        // Comparison.
        // Always take the "biggest" search node in a priority queue.
        bool operator<(const SearchNode& other) const { return area < other.area; }
        bool operator>(const SearchNode& other) const { return area > other.area; }
    };

    std::vector<double> bestMerge(polygons.size(), InvalidArea);
    std::priority_queue<SearchNode> polygonQueue;

    auto rateMerge = [&](size_t index) {
        if(index == Polygon::InvalidIndex) {
            return;
        }
        auto const& polygon = polygons[index];

        auto isValidNeighbor = [index](const auto& idx) {
            if((idx != Polygon::InvalidIndex) && (idx != index)) {
                return true;
            }
            return false;
        };

        const auto num_valid_neigbors = std::count_if(
            std::begin(polygon.neighbors), std::end(polygon.neighbors), isValidNeighbor);

        if(keep_deadends && num_valid_neigbors == 1) {
            // It's a dead end and we don't want to merge it.
            return;
        }

        SearchNode thisNode{index, InvalidArea};
        size_t best = Polygon::InvalidIndex;
        for(size_t i = 0; i < polygon.neighbors.size(); ++i) {
            const auto& neighbor = polygon.neighbors[i];
            if(neighbor == Polygon::InvalidIndex || neighbor == index ||
               polygons[neighbor].neighbors.size() == 0) {
                continue;
            }
            const auto& valid_neighbor = i;
            size_t mergeIndex = polygon.neighbors[valid_neighbor];

            auto [indices, neighbors] = mergedPolygon(index, mergeIndex, valid_neighbor);

            if(isConvex(indices)) {
                thisNode.area = std::max(thisNode.area, polygonArea(indices));
                best = neighbor;
            }
        }
        if(thisNode.area != InvalidArea) {
            polygonQueue.push(thisNode);
            bestMerge[index] = thisNode.area;

        } else {
            bestMerge[index] = InvalidArea;
        }
    };

    for(size_t i = 0; i < polygons.size(); ++i) {
        rateMerge(i);
    }

    size_t mergePartner = Polygon::InvalidIndex;
    size_t firstCommonVertex = Polygon::InvalidIndex;

    while(!polygonQueue.empty()) {
        const auto node = polygonQueue.top();
        polygonQueue.pop();

        if(abs(node.area - bestMerge[node.source]) > 1e-8) {
            // Not the right node.
            continue;
        }

        const auto& polygon = polygons[node.source];

        for(size_t i = 0; i < polygon.neighbors.size(); ++i) {
            const auto& neighbor = polygon.neighbors[i];
            if(neighbor == Polygon::InvalidIndex || neighbor == node.source ||
               polygons[neighbor].vertices.size() == 0) {
                continue;
            }
            size_t mergeIndex = polygon.neighbors[i];

            auto [indices, neighbors] = mergedPolygon(node.source, mergeIndex, i);
            if(isConvex(indices)) {
                auto area = polygonArea(indices);
                if(std::abs(node.area - area) < 1e-8) {
                    mergePartner = mergeIndex;
                    firstCommonVertex = i;
                    break;
                }
            }
        }
        assert(mergePartner != Polygon::InvalidIndex);
        assert(firstCommonVertex != Polygon::InvalidIndex);

        auto mergeSuccess = tryMerge(node.source, mergePartner, firstCommonVertex);
        if(!mergeSuccess) {
            continue;
        }

        assert(mergeSuccess);
        bestMerge[mergePartner] = InvalidArea;
        polygons[mergePartner].neighbors.clear();
        polygons[mergePartner].vertices.clear();
        for(size_t index = 0; index < polygons.size(); ++index) {
            // if(best[index]) {
            //     continue;
            // }
            auto& polygon = polygons[index];
            std::replace(
                std::begin(polygon.neighbors),
                std::end(polygon.neighbors),
                mergePartner,
                node.source);
        }

        // Update THIS merge
        rateMerge(node.source);

        // Update the polygons around this merge.
        for(size_t i = 0; i < polygon.neighbors.size(); ++i) {
            rateMerge(polygon.neighbors[i]);
        }
    }
}

bool Mesh::isMergable(
    size_t polygon_a_index,
    size_t polygon_b_index,
    size_t first_common_vertex_in_a)
{
    auto [indices, neighbors] =
        mergedPolygon(polygon_a_index, polygon_b_index, first_common_vertex_in_a);

    return isConvex(indices);
}

bool Mesh::isValid() const
{
    return true;
}

bool Mesh::isConvex(const std::vector<size_t>& indices) const
{
    const auto prev = [&indices](size_t index) -> size_t {
        const auto count = indices.size();
        assert(count >= 3);
        return (count + index - 1) % count;
    };
    const auto next = [&indices](size_t index) -> size_t {
        const auto count = indices.size();
        assert(count >= 3);
        return (index + 1) % count;
    };

    for(size_t index = 0; index < indices.size(); ++index) {
        const auto& current_vertex = vertices[indices[index]];
        const auto& prev_vertex = vertices[indices[prev(index)]];
        const auto& next_vertex = vertices[indices[next(index)]];

        const auto segment_a = current_vertex - prev_vertex;
        const auto segment_b = next_vertex - current_vertex;
        const auto cp = glm::cross(glm::dvec3(segment_a, 0), glm::dvec3(segment_b, 0));

        if(cp.z < 0.0) {
            // This indicates CW winding between consecutive segments
            return false;
        }
    }
    return true;
}

std::tuple<std::vector<size_t>, std::vector<size_t>>
Mesh::mergedPolygon(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a)
{
    auto& polygon_a = polygons[polygon_a_index];
    const auto& polygon_b = polygons[polygon_b_index];
    const auto new_vertex_count = polygon_a.vertices.size() + polygon_b.vertices.size() - 2;

    std::vector<size_t> indices{};
    indices.reserve(new_vertex_count);
    std::vector<size_t> neighbors{};
    neighbors.reserve(new_vertex_count);

    const auto iter_b = std::find(
        std::begin(polygon_b.vertices),
        std::end(polygon_b.vertices),
        polygon_a.vertices[first_common_vertex_in_a]);
    assert(iter_b != std::end(polygon_b.vertices));
    const auto vertex_in_b = std::distance(std::begin(polygon_b.vertices), iter_b);

    for(size_t index = 0; index < polygon_b.vertices.size() - 1; ++index) {
        indices.emplace_back(polygon_b.vertices[(index + vertex_in_b) % polygon_b.vertices.size()]);
        neighbors.emplace_back(
            polygon_b.neighbors[(index + vertex_in_b) % polygon_b.neighbors.size()]);
    }
    for(size_t index = 0; index < polygon_a.vertices.size() - 1; ++index) {
        indices.emplace_back(
            polygon_a.vertices[(index + first_common_vertex_in_a + 1) % polygon_a.vertices.size()]);
        neighbors.emplace_back(
            polygon_a
                .neighbors[(index + first_common_vertex_in_a + 1) % polygon_a.neighbors.size()]);
    }

    return std::make_tuple(indices, neighbors);
}

bool Mesh::tryMerge(size_t polygon_a_index, size_t polygon_b_index, size_t first_common_vertex_in_a)
{
    auto [indices, neighbors] =
        mergedPolygon(polygon_a_index, polygon_b_index, first_common_vertex_in_a);

    if(isConvex(indices)) {
        auto& polygon_a = polygons[polygon_a_index];
        const auto& polygon_b = polygons[polygon_b_index];

        polygon_a.vertices = indices;
        polygon_a.neighbors = neighbors;
        return true;
    }

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

void Mesh::updateBoundingBoxes()
{
    boundingBoxes.clear();
    boundingBoxes.reserve(polygons.size());

    std::transform(
        std::begin(polygons),
        std::end(polygons),
        std::back_inserter(boundingBoxes),
        [this](const auto& polygon) {
            float xMin = std::numeric_limits<float>::max();
            float xMax = std::numeric_limits<float>::lowest();
            float yMin = std::numeric_limits<float>::max();
            float yMax = std::numeric_limits<float>::lowest();

            for(const auto& pIndex : polygon.vertices) {
                const auto& p = vertices[pIndex];
                xMin = std::min(xMin, static_cast<float>(p.x));
                xMax = std::max(xMax, static_cast<float>(p.x));
                yMin = std::min(yMin, static_cast<float>(p.y));
                yMax = std::max(yMax, static_cast<float>(p.y));
            }

            return AABB{{xMin, yMin}, {xMax, yMax}};
        });
}

size_t Mesh::FindContainingPolygon(const glm::vec2& p) const
{
    for(size_t index = 0; index < polygons.size(); ++index) {
        if(boundingBoxes[index].Contains(p) && polygonContains(index, p)) {
            return index;
        }
    }

    return Polygon::InvalidIndex;
}

bool Mesh::polygonContains(const size_t polygonIndex, glm::vec2 p) const
{
    const auto& poly = polygons[polygonIndex];

    for(size_t i = 0; i < poly.vertices.size(); ++i) {

        const glm::vec2 diff = vertices[(i + 1) % poly.vertices.size()] - vertices[i];
        const glm::vec2 xp = p - vertices[i];

        const auto cp = glm::cross(glm::vec3(diff, 0), glm::vec3(xp, 0));
        if(cp.z < 0.0) {
            return false;
        }
    }
    return true;
}