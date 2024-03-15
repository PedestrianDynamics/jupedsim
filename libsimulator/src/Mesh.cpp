// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Mesh.hpp"

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <CGAL/number_utils.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

Mesh::Mesh(const CDT& cdt)
{
    std::unordered_map<CDT::Vertex_handle, size_t> vertex_handle_to_index_map{};
    vertex_handle_to_index_map.reserve(cdt.finite_vertex_handles().size());
    vertices.reserve(cdt.finite_vertex_handles().size());
    size_t index = 0;
    for(const auto& h : cdt.finite_vertex_handles()) {
        vertex_handle_to_index_map[h] = index;
        const auto& pt = h->point();
        vertices.emplace_back(CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
        ++index;
    }
    std::unordered_map<CDT::Face_handle, size_t> face_handle_to_index_map{};
    face_handle_to_index_map.reserve(cdt.finite_face_handles().size());
    polygons.reserve(cdt.finite_face_handles().size());
    std::vector<std::vector<std::optional<CDT::Face_handle>>> neighbor_face_handles{};
    neighbor_face_handles.reserve(cdt.finite_face_handles().size());

    index = 0;
    for(const CDT::Face_handle t : cdt.finite_face_handles()) {
        if(!t->get_in_domain()) {
            continue;
        }
        Polygon p{};
        p.vertices.reserve(3);
        p.vertices.push_back(vertex_handle_to_index_map[t->vertex(0)]);
        p.vertices.push_back(vertex_handle_to_index_map[t->vertex(1)]);
        p.vertices.push_back(vertex_handle_to_index_map[t->vertex(2)]);

        polygons.push_back(p);
        neighbor_face_handles.emplace_back();
        neighbor_face_handles.back().reserve(3);
        for(int index = 0; index < 3; ++index) {
            const auto& n = t->neighbor(CDT::cw(index));
            if(n->get_in_domain()) {
                neighbor_face_handles.back().emplace_back(n);
            } else {
                neighbor_face_handles.back().emplace_back(std::nullopt);
            }
        }
        face_handle_to_index_map[t] = index;
        ++index;
    }

    assert(polygons.size() == neighbor_face_handles.size());
    for(size_t index = 0; index < polygons.size(); ++index) {
        auto& p = polygons[index];
        const auto& neighbors = neighbor_face_handles[index];
        p.neighbors.reserve(neighbors.size());
        for(const auto& opt : neighbors) {
            if(opt) {
                p.neighbors.emplace_back(face_handle_to_index_map.at(*opt));
            } else {
                p.neighbors.emplace_back(Polygon::InvalidIndex);
            }
        }
    }

    updateBoundingBoxes();
};

std::unique_ptr<Mesh> Mesh::Clone() const
{
    return std::make_unique<Mesh>(*this);
}

void Mesh::MergeGreedy()
{
    mergeDeadEnds();
    smartMerge(true);
    trimEmptyPolygons();
    assert(isValid());
    updateBoundingBoxes();
}

void Mesh::mergeDeadEnds()
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
        for(size_t i = 0; i < polygon.neighbors.size(); ++i) {
            const auto& neighbor = polygon.neighbors[i];
            if(neighbor == Polygon::InvalidIndex || neighbor == index ||
               polygons[neighbor].neighbors.size() == 0) {
                continue;
            }
            const auto& valid_neighbor = i;
            size_t mergeIndex = polygon.neighbors[valid_neighbor];

            auto [indices, neighbors] = mergedPolygon(index, mergeIndex, valid_neighbor);

            if(polygonIsConvex(indices)) {
                thisNode.area = std::max(thisNode.area, polygonArea(indices));
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
            if(polygonIsConvex(indices)) {
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

    return polygonIsConvex(indices);
}

bool Mesh::isValid() const
{
    for(const auto& p : polygons) {
        if(!polygonIsConvex(p.vertices)) {
            return false;
        }
    }
    return true;
}

bool Mesh::polygonIsConvex(const std::vector<size_t>& indices) const
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

    if(polygonIsConvex(indices)) {
        auto& polygon_a = polygons[polygon_a_index];
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

void Mesh::trimEmptyPolygons()
{
    const auto polygon_count =
        std::count_if(std::begin(polygons), std::end(polygons), [](const auto& p) {
            return !p.vertices.empty();
        });
    std::vector<Polygon> trimed_polygons{};
    trimed_polygons.reserve(polygon_count);
    std::unordered_map<size_t, size_t> index_mapping{};

    for(size_t index = 0; index < polygons.size(); ++index) {
        if(polygons[index].vertices.empty()) {
            continue;
        }
        index_mapping[index] = trimed_polygons.size();
        trimed_polygons.emplace_back(polygons[index]);
    }

    for(auto& p : trimed_polygons) {
        for(auto& n : p.neighbors) {
            const auto iter = index_mapping.find(n);
            if(iter != std::end(index_mapping)) {
                n = iter->second;
            }
        }
    }
    polygons = trimed_polygons;
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

size_t Mesh::FindContainingPolygon(const glm::dvec2& p) const
{
    for(size_t index = 0; index < polygons.size(); ++index) {
        if(boundingBoxes[index].Inside({p.x, p.y}) && TriangleContains(index, p)) {
            return index;
        }
    }

    return Polygon::InvalidIndex;
}

glm::dvec2 Mesh::Vertex(size_t index) const
{
    return vertices.at(index);
}

static int toPolyanyaIndex(size_t idx)
{
    if(idx == std::numeric_limits<size_t>::max()) {
        return -1;
    }
    assert(idx <= std::numeric_limits<int>::max());
    return idx;
}

std::stringstream Mesh::IntoLibPolyanyaMeshDescription() const
{
    std::stringstream buf{};
    buf << "mesh\n2\n";
    buf << CountVertices() << " " << CountPolygons() << "\n\n";

    auto indexOfValue = [](const auto& vec, const auto& val) -> std::optional<size_t> {
        const auto iter = std::find(std::begin(vec), std::end(vec), val);
        if(iter == std::end(vec)) {
            return std::nullopt;
        }
        return std::distance(std::begin(vec), iter);
    };

    // clang-format off
    // clang-format-15 as enforced by out CI want to reformat this code block differently
    // than newer versions although the format for other lamda declarations like this is identical
    // Hence for this block of code clang-format is disabled until we migrate to a newer clang-format
    // TODO(kkratz): Re-emable clang-format here once we can upgrade the clang-format version
    auto findNeighborIndices = [this, indexOfValue](size_t vertex_index) -> auto {
        // Collect the incoming and outgoing edge (as indices) for this polygon at the specified
        // vertex index.
        struct Wedge {
            /// index 0 is previous index, index 2 is the next index
            size_t vertex_indices[3];
            /// represents the orientation of the wedge as ccw rotation in radians vs the [0,1]
            /// vector
            size_t polygon_index;
            double angle;
            bool operator<(const Wedge& other) const { return angle < other.angle; }
        };

        std::vector<Wedge> neighbor_wedge{};
        const glm::dvec2 ref{0, 1};
        for(size_t polygon_index = 0; polygon_index < polygons.size(); ++polygon_index) {
            const auto& polygon_vertices = polygons[polygon_index].vertices;
            const auto count_vertices = polygon_vertices.size();
            if(const auto idx = indexOfValue(polygon_vertices, vertex_index); idx) {
                const size_t prev_idx = (*idx + count_vertices - 1) % count_vertices;
                const size_t next_idx = (*idx + 1) % count_vertices;
                const auto next_vec = glm::normalize(vertices[next_idx] - vertices[*idx]);
                const auto prev_vec = glm::normalize(vertices[prev_idx] - vertices[*idx]);
                const auto middle_vec = glm::normalize(next_vec + prev_vec);
                const double cos = glm::dot(ref, middle_vec);
                const double sin = glm::cross(glm::dvec3(ref, 0), glm::dvec3(middle_vec, 0)).z;
                const double angle = std::atan2(cos, sin);
                neighbor_wedge.emplace_back(
                    Wedge{{prev_idx, *idx, next_idx}, polygon_index, angle});
            }
        }
        // Sort wedges by orientation
        std::sort(std::begin(neighbor_wedge), std::end(neighbor_wedge));
        std::vector<size_t> neighbor_indices{};
        neighbor_indices.reserve(2 * neighbor_wedge.size());
        for(size_t idx = 0; idx < neighbor_wedge.size(); ++idx) {
            neighbor_indices.emplace_back(neighbor_wedge[idx].polygon_index);
            // If the wedges do not share an edge that means they are not touching and there is a
            // gap in between
            const size_t next_idx = (idx + 1) % neighbor_wedge.size();
            if(neighbor_wedge[idx].vertex_indices[0] !=
               neighbor_wedge[next_idx].vertex_indices[2]) {
                neighbor_indices.emplace_back(Mesh::Polygon::InvalidIndex);
            }
        }
        return neighbor_indices;
    };
    // clang-format on

    for(size_t vertex_index = 0; vertex_index < vertices.size(); ++vertex_index) {
        const auto& vertex = vertices[vertex_index];
        buf << vertex.x << " " << vertex.y << " ";

        const auto neighbors = findNeighborIndices(vertex_index);
        buf << neighbors.size() << " ";
        for(const auto n : neighbors) {
            buf << toPolyanyaIndex(n) << " ";
        }
        buf << "\n";
    }

    buf << "\n";

    for(const auto& p : polygons) {
        buf << p.vertices.size() << " ";
        for(const auto& v : p.vertices) {
            buf << toPolyanyaIndex(v) << " ";
        }
        for(size_t idx = 0; idx < p.neighbors.size(); ++idx) {
            const size_t shifted_idx = (idx + p.neighbors.size() - 1) % p.neighbors.size();
            const auto n = p.neighbors[shifted_idx];
            buf << toPolyanyaIndex(n) << " ";
        }
        buf << "\n";
    }

    return buf;
}

/// 2D pseudo cross product
static double cross2D(glm::dvec2 a, glm::dvec2 b)
{
    return a.y * b.x - a.x * b.y;
}

bool Mesh::TriangleContains(const size_t polygonIndex, glm::dvec2 p) const
{
    const auto& poly = polygons[polygonIndex];
    assert(poly.vertices.size() == 3);
    const auto a = vertices[poly.vertices[0]];
    const auto b = vertices[poly.vertices[1]];
    const auto c = vertices[poly.vertices[2]];

    if(cross2D(p - a, b - a) < 0) {
        return false;
    }
    if(cross2D(p - b, c - b) < 0) {
        return false;
    }
    if(cross2D(p - c, a - c) < 0) {
        return false;
    }
    return true;
}
