// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/validation.hpp"

#include "cfg_cgal.hpp"
#include "geometry/region_split.hpp"
#include "simulation_error.hpp"

#include <CGAL/Bbox_2.h>
#include <CGAL/Box_intersection_d/Box_with_info_d.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/box_intersection_d.h>
#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

bool is_walkable_normal(const epick::Vector3D& n)
{
    constexpr double max_incline_deg = 50.0;
    constexpr double max_incline_rad = max_incline_deg * std::numbers::pi / 180.0;
    // @TODO(kkratz): Replace with constexpr when moving to C++23
    static const double min_z = std::cos(max_incline_rad);
    return n.z() >= min_z;
}

namespace
{
std::optional<std::pair<epick::Mesh::Face_index, epick::Mesh::Face_index>>
first_self_intersection(const epick::Mesh& mesh)
{
    std::vector<std::pair<epick::Mesh::Face_index, epick::Mesh::Face_index>> intersecting{};
    CGAL::Polygon_mesh_processing::self_intersections(mesh, std::back_inserter(intersecting));
    if(intersecting.empty()) {
        return std::nullopt;
    }
    return intersecting.front();
}
} // namespace

bool is_face_in_mesh_planar(
    const epick::Mesh& mesh,
    FaceDescriptor<epick::Mesh> face,
    std::vector<VertexDescriptor<epick::Mesh>>& buffer)
{
    buffer.clear();
    for(auto half_edge : CGAL::halfedges_around_face(CGAL::halfedge(face, mesh), mesh)) {
        buffer.emplace_back(CGAL::source(half_edge, mesh));
    }

    if(buffer.size() < 3) {
        return false;
    }

    const auto& p0 = mesh.point(buffer[0]);
    const auto& p1 = mesh.point(buffer[1]);
    const auto& p2 = mesh.point(buffer[2]);

    if(CGAL::collinear(p0, p1, p2)) {
        return false;
    }

    return std::all_of(std::begin(buffer), std::end(buffer), [&p0, &p1, &p2, &mesh](auto v) {
        return CGAL::coplanar(p0, p1, p2, mesh.point(v));
    });
}

bool all_faces_in_mesh_planar(const epick::Mesh& mesh)
{
    std::vector<VertexDescriptor<epick::Mesh>> buffer{};
    buffer.reserve(3);
    return std::all_of(
        std::begin(CGAL::faces(mesh)), std::end(CGAL::faces(mesh)), [&buffer, &mesh](auto face) {
            return is_face_in_mesh_planar(mesh, face, buffer);
        });
}

void normalise_and_validate_mesh(epick::Mesh& mesh, const RegionMap* regions)
{
    namespace pmp = CGAL::Polygon_mesh_processing;
    if(!CGAL::is_triangle_mesh(mesh)) {
        pmp::triangulate_faces(mesh);
    }

    if(mesh.number_of_faces() == 0) {
        throw SimulationError("No Geometry defined.");
    }

    if(!all_faces_in_mesh_planar(mesh)) {
        throw SimulationError("Not all faces are planar.");
    }

    const auto n = pmp::compute_face_normal(*std::begin(CGAL::faces(mesh)), mesh);
    if(!is_walkable_normal(n)) {
        pmp::reverse_face_orientations(mesh);
    }

    validate_mesh(mesh, regions);
}

void validate_mesh(const epick::Mesh& mesh, const RegionMap* regions)
{
    namespace pmp = CGAL::Polygon_mesh_processing;
    for(auto&& face : CGAL::faces(mesh)) {
        const auto n = pmp::compute_face_normal(face, mesh);
        if(!is_walkable_normal(n)) {
            throw SimulationError("Face {} inclination exceeds 50deg.", face.idx());
        }
    }

    if(const auto intersection = first_self_intersection(mesh)) {
        if(regions == nullptr) {
            throw SimulationError("Mesh faces pass through each other.");
        }
        throw SimulationError(
            "Region {} and region {} pass through each other.",
            (*regions)[intersection->first],
            (*regions)[intersection->second]);
    }

    std::vector<size_t> component(mesh.number_of_faces(), 0);
    const auto component_of =
        boost::make_iterator_property_map(std::begin(component), get(CGAL::face_index, mesh));
    const auto count = pmp::connected_components(mesh, component_of);
    if(count > 1) {
        if(regions == nullptr) {
            throw SimulationError("Expected exactly 1 connected component, got: {}", count);
        }
        const auto first = *mesh.faces().begin();
        for(const auto face : mesh.faces()) {
            if(component[face] != component[first]) {
                throw SimulationError(
                    "Region {} is not connected to region {}.",
                    (*regions)[face],
                    (*regions)[first]);
            }
        }
    }
}

std::optional<epick::Point2D>
find_edge_clearance_violation(const epick::PolyWithHoles& polygon, double dist)
{
    // Each vertex gets a box around its point, each edge its axis-aligned bounding box (AABB)
    // grown by dist. CGAL's box_intersection_d finds the overlapping pairs: the vertices that may
    // be too close to an edge. For these, the exact distance decides.
    std::vector<epick::Point2D> points{};
    std::vector<std::array<size_t, 2>> edges{};
    const auto add_ring = [&points, &edges](const epick::Poly& ring) {
        const size_t first = points.size();
        for(size_t index = 0; index < ring.size(); ++index) {
            points.push_back(ring.vertex(index));
            edges.push_back({first + index, first + (index + 1) % ring.size()});
        }
    };
    add_ring(polygon.outer_boundary());
    for(const auto& hole : polygon.holes()) {
        add_ring(hole);
    }

    using Box = CGAL::Box_intersection_d::Box_with_info_d<double, 2, size_t>;
    std::vector<Box> vertex_boxes{};
    vertex_boxes.reserve(points.size());
    for(size_t index = 0; index < points.size(); ++index) {
        vertex_boxes.emplace_back(points[index].bbox(), index);
    }
    std::vector<Box> edge_boxes{};
    edge_boxes.reserve(edges.size());
    for(size_t index = 0; index < edges.size(); ++index) {
        const CGAL::Bbox_2 box = points[edges[index][0]].bbox() + points[edges[index][1]].bbox();
        edge_boxes.emplace_back(
            CGAL::Bbox_2(
                box.xmin() - dist, box.ymin() - dist, box.xmax() + dist, box.ymax() + dist),
            index);
    }

    const double dist_sq = dist * dist;
    std::optional<epick::Point2D> violation{};
    CGAL::box_intersection_d(
        vertex_boxes.begin(),
        vertex_boxes.end(),
        edge_boxes.begin(),
        edge_boxes.end(),
        [&](const Box& vertex_box, const Box& edge_box) {
            const size_t vertex = vertex_box.info();
            const auto [source, target] = edges[edge_box.info()];
            if(violation || vertex == source || vertex == target) {
                return;
            }
            const epick::Segment2D edge{points[source], points[target]};
            if(CGAL::squared_distance(points[vertex], edge) < dist_sq) {
                violation = points[vertex];
            }
        });
    return violation;
}
