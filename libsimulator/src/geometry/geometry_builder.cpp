// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/geometry_builder.hpp"

#include "cfg_cgal.hpp"
#include "geometry/geometry.hpp"
#include "geometry/region_split.hpp"
#include "geometry/validation.hpp"
#include "into.hpp"
#include "kernel.hpp"
#include "line_segment.hpp"
#include "simulation_error.hpp"
#include "validation.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Box_intersection_d/Box_with_info_d.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/box_intersection_d.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <boost/range/iterator_range.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
/// Triangulate one region (boundary + holes)
/// Returns the triangles as triples of those same indices, or nothing when the rings intersect each
/// other.
std::optional<std::vector<std::array<size_t, 3>>> triangulate_rings(
    const std::vector<std::vector<size_t>>& rings,
    const std::vector<epick::Point3D>& vertices)
{
    epick::CDT cdt{};
    std::unordered_map<epick::CDT::Vertex_handle, size_t> cdt_vertices{};
    for(const auto& ring : rings) {
        std::vector<epick::CDT::Vertex_handle> handles{};
        handles.reserve(ring.size());
        for(const size_t vertex_id : ring) {
            const epick::Point3D& vertex = vertices[vertex_id];
            const auto handle = cdt.insert(epick::Point2D(vertex[0], vertex[1]));
            cdt_vertices.emplace(handle, vertex_id);
            handles.emplace_back(handle);
        }
        for(size_t index = 1; index < handles.size(); ++index) {
            cdt.insert_constraint(handles[index - 1], handles[index]);
        }
        cdt.insert_constraint(handles.back(), handles.front()); // close the ring
    }
    CGAL::mark_domain_in_triangulation(cdt);

    std::vector<std::array<size_t, 3>> triangles{};
    for(const auto& face : cdt.finite_face_handles()) {
        if(!face->get_in_domain()) { // Only faces inside the polygon.
            continue;
        }
        // Note: CDT faces are oriented ccw in 2D. This means we do not need to check the
        //       order of points for connectors.
        std::array<size_t, 3> triangle{};
        for(int corner = 0; corner < 3; ++corner) {
            const auto iter = cdt_vertices.find(face->vertex(corner));
            if(iter == cdt_vertices.end()) {
                // A vertex not in the map got added by triangulation ("Steiner point")
                return std::nullopt;
            }
            triangle[corner] = iter->second;
        }
        triangles.emplace_back(triangle);
    }
    return triangles;
}

epick::PolyWithHoles as_2d_poly_with_holes(
    const std::vector<std::vector<size_t>>& rings,
    const std::vector<epick::Point3D>& vertices)
{
    // lambda fct: PolyWithHoles needs boundary counterclockwise and holes clockwise.
    const auto oriented_poly =
        [&vertices](const std::vector<size_t>& ring, CGAL::Orientation wanted) {
            epick::Poly poly{};
            for(const size_t vertex_id : ring) {
                const epick::Point3D& v = vertices[vertex_id];
                poly.push_back(onto_xy(v));
            }
            if(poly.orientation() != wanted) {
                poly.reverse_orientation();
            }
            return poly;
        };

    std::vector<epick::Poly> holes{};
    holes.reserve(rings.size() - 1);
    for(size_t index = 1; index < rings.size(); ++index) {
        holes.emplace_back(oriented_poly(rings[index], CGAL::CLOCKWISE));
    }
    return epick::PolyWithHoles(
        oriented_poly(rings[0], CGAL::COUNTERCLOCKWISE), std::begin(holes), std::end(holes));
}

Geometry::SeamEdge
find_seam_edge(const epick::PolyWithHoles& poly, const epick::Point2D& a, const epick::Point2D& b)
{
    const auto find_in_ring = [&a, &b](const epick::Poly& ring) -> std::optional<size_t> {
        for(size_t index = 0; index < ring.size(); ++index) {
            const epick::Point2D& p = ring.vertex(index);
            const epick::Point2D& q = ring.vertex((index + 1) % ring.size());
            if((p == a && q == b) || (p == b && q == a)) {
                return index;
            }
        }
        return std::nullopt;
    };

    if(const auto index = find_in_ring(poly.outer_boundary())) {
        return {0, *index};
    }
    for(size_t hole = 0; hole < poly.number_of_holes(); ++hole) {
        if(const auto index = find_in_ring(poly.holes()[hole])) {
            return {hole + 1, *index};
        }
    }
    throw SimulationError("Internal Error: Seam is not an edge of the region.");
}

} // namespace

//==================================================================================================
// GeometryBuilder
//==================================================================================================
GeometryBuilder::GeometryBuilder()
{
    _global_vertices.reserve(1024);
}

size_t GeometryBuilder::add_region(Polygon polygon, double height)
{
    if(!std::isfinite(height) || std::abs(height) > max_coordinate) {
        throw SimulationError(
            "Region height must be finite and at most {} in magnitude", max_coordinate);
    }
    // Checked up front: CGAL's predicates are not guarded against non-finite or huge values, and
    // some of them crash instead of throwing.
    const auto is_valid_point = [](const Point& p) {
        return std::isfinite(p.x()) && std::isfinite(p.y()) && std::abs(p.x()) <= max_coordinate &&
               std::abs(p.y()) <= max_coordinate;
    };
    const auto is_valid_ring = [&is_valid_point](const Ring& ring) {
        return std::ranges::all_of(ring, is_valid_point);
    };
    if(!is_valid_ring(polygon.boundary) || !std::ranges::all_of(polygon.holes, is_valid_ring)) {
        throw SimulationError(
            "Region coordinates must be finite and at most {} in magnitude", max_coordinate);
    }

    bool rings_are_simple = false;
    bool holes_are_valid = false;
    std::optional<epeck::Point2D> shared_vertex{};
    std::optional<std::pair<epeck::Point2D, epeck::Point2D>> too_close{};
    epeck::PolyWithHoles exact_polygon;
    // The coordinates are checked above; this only turns any remaining CGAL error into a
    // SimulationError.
    try {
        exact_polygon = into<epeck::PolyWithHoles>(polygon);
        // Each ring on its own first: orientation, and with it fixup_winding, needs simple rings.
        const auto is_simple_ring = [](const epeck::Poly& ring) {
            return ring.size() >= 3 && ring.is_simple();
        };
        rings_are_simple = is_simple_ring(exact_polygon.outer_boundary()) &&
                           std::ranges::all_of(exact_polygon.holes(), is_simple_ring);
        if(rings_are_simple) {
            fixup_winding(exact_polygon);
            // With simple rings, only the holes' placement can make the polygon invalid.
            epeck::Traits traits;
            holes_are_valid = CGAL::is_valid_polygon_with_holes(exact_polygon, traits);
            // CGAL accepts rings touching at a shared vertex. Within a simple ring vertices are
            // unique, so a repeated vertex means two rings touch.
            std::vector<epeck::Point2D> corners{};
            corners.reserve(vertex_count(exact_polygon));
            std::ranges::copy(vertices(exact_polygon), std::back_inserter(corners));
            std::ranges::sort(corners, [](const auto& p, const auto& q) {
                return CGAL::compare_xy(p, q) == CGAL::SMALLER;
            });
            if(const auto repeated = std::ranges::adjacent_find(corners);
               repeated != std::ranges::end(corners)) {
                shared_vertex = *repeated;
            }
            too_close = find_clearance_violation(vertices(exact_polygon), min_clearance);
        }
    } catch(const std::runtime_error& e) {
        throw SimulationError("{}", e.what());
    }
    if(!rings_are_simple) {
        throw SimulationError(
            "Invalid polygon provided to add_region: every ring needs at least 3 vertices and "
            "must not intersect itself.");
    }
    if(!holes_are_valid || shared_vertex) {
        throw SimulationError(
            "Holes must lie strictly inside the boundary and must neither overlap nor touch each "
            "other{}.",
            shared_vertex ? fmt::format(" (two rings touch at {})", *shared_vertex) : "");
    }
    if(too_close) {
        throw SimulationError(
            "Vertices {} and {} are closer than {} m: all vertices of a region need a clearance "
            "of {} m.",
            too_close->first,
            too_close->second,
            min_clearance,
            min_clearance);
    }

    std::vector<std::vector<size_t>> indices{};
    indices.reserve(exact_polygon.number_of_holes() + 1);
    size_t offset = _global_vertices.size();
    std::vector<size_t> ring(exact_polygon.outer_boundary().size());
    std::ranges::copy(std::views::iota(offset, offset + ring.size()), ring.begin());
    indices.push_back(std::move(ring));
    offset += exact_polygon.outer_boundary().size();
    for(const auto& hole : exact_polygon.holes()) {
        std::vector<size_t> hole_indices(hole.size());
        std::ranges::copy(std::views::iota(offset, offset + hole.size()), hole_indices.begin());
        indices.push_back(std::move(hole_indices));
        offset += hole.size();
    }

    auto vert_3d =
        vertices(exact_polygon) | std::views::transform([height](const auto& p) {
            return epick::Point3D{CGAL::to_double(p.x()), CGAL::to_double(p.y()), height};
        });
    std::ranges::copy(vert_3d, std::back_inserter(_global_vertices));

    Region region{
        .polygons = std::move(indices),
        .height = height,
        .poly_with_holes = into<epick::PolyWithHoles>(exact_polygon)};
    // Boost has no add_vertex overload taking an rvalue property, so move it in afterwards.
    const auto id = boost::add_vertex(_region_graph);
    _region_graph[id] = std::move(region);
    return id;
}

std::array<size_t, 2> GeometryBuilder::find_edge(size_t region_id, const LineSegment& edge) const
{
    // no need to compare z as regions by design cannot overlap in xy within a single region
    auto match_in_2d = [this](size_t vertex_id, const Point& p) {
        const epick::Point3D& global_vertex = _global_vertices[vertex_id];
        return (Point(global_vertex[0], global_vertex[1]) - p).is_zero_length();
    };

    const Region& region = _region_graph[region_id];
    for(const auto& polygon : region.polygons) {
        for(size_t index = 0; index < polygon.size(); ++index) {
            const size_t current = polygon[index];
            const size_t next = polygon[(index + 1) % polygon.size()];
            // Ring order, whichever way round the edge was given: the region lies to its left.
            if((match_in_2d(current, edge.p1) && match_in_2d(next, edge.p2)) ||
               (match_in_2d(current, edge.p2) && match_in_2d(next, edge.p1))) {
                return {current, next};
            }
        }
    }
    throw SimulationError("{} is not an edge of region {}.", edge, region_id);
}

size_t GeometryBuilder::connect_regions(
    size_t from_region,
    LineSegment from,
    size_t to_region,
    LineSegment to)
{
    auto check_region = [this](size_t region_id, std::string_view name) {
        if(region_id >= boost::num_vertices(_region_graph)) {
            throw SimulationError("Unknown region id used for {}: {}", name, region_id);
        }
        const Region& region = _region_graph[region_id];
        if(!region.is_connectable()) {
            throw SimulationError("{} {} is not connectable", name, region_id);
        }
    };
    check_region(from_region, "fromRegion");
    check_region(to_region, "toRegion");
    if(from_region == to_region) {
        throw SimulationError("fromRegion and toRegion may not be the same region.");
    }

    // Seams in ring order, i.e. with their region to the left. The connector lies to the right
    // of both, so a valid connector is counterclockwise as {b, a, d, c}.
    const auto [a, b] = find_edge(from_region, from);
    const auto [c, d] = find_edge(to_region, to);
    // A seam carries one connector at most: a second one would make the mesh non-manifold.
    const auto is_same_seam = [](const Seam& seam, size_t u, size_t v) {
        return (seam[0] == u && seam[1] == v) || (seam[0] == v && seam[1] == u);
    };
    for(const auto edge : boost::make_iterator_range(boost::edges(_region_graph))) {
        const Seam& seam = _region_graph[edge];
        if(is_same_seam(seam, a, b)) {
            throw SimulationError("{} of region {} is already connected.", from, from_region);
        }
        if(is_same_seam(seam, c, d)) {
            throw SimulationError("{} of region {} is already connected.", to, to_region);
        }
    }
    const std::vector<size_t> connector_polygon{b, a, d, c};

    std::array<epick::Point3D, 4> corners{};
    std::array<epick::Point2D, 4> corners_2d{};
    for(size_t index = 0; index < corners.size(); ++index) {
        corners[index] = _global_vertices[connector_polygon[index]];
        corners_2d[index] = onto_xy(corners[index]);
    }
    if(!epick::Poly(std::begin(corners_2d), std::end(corners_2d)).is_simple()) {
        throw SimulationError("Connector is no simple polygon in 2D.");
    }

    // Both seams are horizontal. On one height (up to height_tolerance) the connector is flat;
    // between two heights it is planar only if the seams are parallel, which is checked up to a
    // small angle.
    if(std::abs(corners[0].z() - corners[2].z()) > height_tolerance) {
        const epick::Vector2D from_direction = corners_2d[1] - corners_2d[0];
        const epick::Vector2D to_direction = corners_2d[3] - corners_2d[2];
        const double sine =
            std::abs(CGAL::determinant(from_direction, to_direction)) /
            std::sqrt(from_direction.squared_length() * to_direction.squared_length());
        if(sine > max_seam_angle_sine) {
            throw SimulationError("Connector is not planar: the seams are not parallel.");
        }
    }

    // Newell's normal over all corners: needs no special case for three collinear corners, which a
    // flat connector may have. Its z component is twice the signed area in 2D, so it is negative
    // exactly when the connector lies on the inner side of both seams.
    epick::Vector3D normal{0, 0, 0};
    for(size_t index = 0; index < corners.size(); ++index) {
        const epick::Point3D& p = corners[index];
        const epick::Point3D& q = corners[(index + 1) % corners.size()];
        normal = normal + epick::Vector3D{
                              (p.y() - q.y()) * (p.z() + q.z()),
                              (p.z() - q.z()) * (p.x() + q.x()),
                              (p.x() - q.x()) * (p.y() + q.y())};
    }
    if(normal.z() < 0) {
        throw SimulationError("Connector does not lie outside of both regions.");
    }
    normal = normal / std::sqrt(normal.squared_length());
    if(!is_walkable_normal(normal)) {
        throw SimulationError("Connector is too steep.");
    }

    Region connector{
        .polygons = {connector_polygon},
        .height = std::nullopt,
        .poly_with_holes = as_2d_poly_with_holes({connector_polygon}, _global_vertices)};
    const auto connector_region = boost::add_vertex(connector, _region_graph);
    boost::add_edge(from_region, connector_region, Seam{a, b}, _region_graph);
    boost::add_edge(connector_region, to_region, Seam{c, d}, _region_graph);
    return connector_region;
}

std::unique_ptr<GeometryBuilder::RegionGraph2D> GeometryBuilder::create_region_graph_2d() const
{
    const auto as_point_2d = [this](size_t vertex_id) {
        const epick::Point3D& v = _global_vertices[vertex_id];
        return onto_xy(v);
    };

    auto graph = std::make_unique<RegionGraph2D>();
    // Add vertices.
    for(const auto region_id : boost::make_iterator_range(boost::vertices(_region_graph))) {
        boost::add_vertex(_region_graph[region_id].poly_with_holes, *graph);
    }

    // Add seams.
    for(const auto& edge : boost::make_iterator_range(boost::edges(_region_graph))) {
        const auto from = boost::source(edge, _region_graph);
        const auto to = boost::target(edge, _region_graph);
        const Seam& seam = _region_graph[edge];
        const epick::Point2D a = as_point_2d(seam[0]);
        const epick::Point2D b = as_point_2d(seam[1]);
        boost::add_edge(from, to, find_seam_edge((*graph)[from], a, b), *graph);
        boost::add_edge(to, from, find_seam_edge((*graph)[to], a, b), *graph);
    }

    return graph;
}

std::unique_ptr<Geometry> GeometryBuilder::build() const
{
    if(boost::num_vertices(_region_graph) == 0) {
        throw SimulationError("No Geometry defined.");
    }

    // Regions on one height (up to height_tolerance) must neither overlap nor touch. Before the
    // clearance check, which would report regions touching at a vertex less specifically.
    using Box = CGAL::Box_intersection_d::Box_with_info_d<double, 2, size_t>;
    std::vector<Box> floors{};
    for(const auto region_id : boost::make_iterator_range(boost::vertices(_region_graph))) {
        const Region& region = _region_graph[region_id];
        if(region.height) {
            floors.emplace_back(region.poly_with_holes.outer_boundary().bbox(), region_id);
        }
    }
    CGAL::box_self_intersection_d(floors.begin(), floors.end(), [this](const Box& a, const Box& b) {
        const size_t first = std::min(a.info(), b.info());
        const size_t second = std::max(a.info(), b.info());
        const Region& first_region = _region_graph[first];
        const Region& second_region = _region_graph[second];
        if(std::abs(*first_region.height - *second_region.height) > height_tolerance) {
            return;
        }
        // Exact: the epick Boolean operations crash or misclassify nearly degenerate pairs. The
        // conversion is exact, as the coordinates are doubles.
        const auto side = CGAL::oriented_side(
            into<epeck::PolyWithHoles>(first_region.poly_with_holes),
            into<epeck::PolyWithHoles>(second_region.poly_with_holes));
        if(side == CGAL::ON_POSITIVE_SIDE) {
            throw SimulationError("Region {} overlaps with region {}.", second, first);
        }
        if(side == CGAL::ON_ORIENTED_BOUNDARY) {
            throw SimulationError("Region {} touches region {}.", second, first);
        }
    });

    if(const auto too_close = find_clearance_violation(_global_vertices, min_clearance)) {
        throw SimulationError(
            "Vertices {} and {} are closer than {} m: all vertices of the geometry need a "
            "clearance of {} m.",
            too_close->first,
            too_close->second,
            min_clearance,
            min_clearance);
    }

    epick::Mesh mesh{};
    RegionSplit region_split{{}, boost::num_vertices(_region_graph)};

    // Map _global_vertices to CGAL mesh vertices.
    std::vector<epick::Mesh::Vertex_index> mesh_vertices(
        _global_vertices.size(), epick::Mesh::null_vertex());
    auto mesh_vertex = [this, &mesh, &mesh_vertices](size_t vertex_id) {
        auto& mesh_vertex = mesh_vertices[vertex_id];
        if(mesh_vertex == epick::Mesh::null_vertex()) {
            mesh_vertex = mesh.add_vertex(_global_vertices[vertex_id]);
        }
        return mesh_vertex;
    };

    for(const auto region_id : boost::make_iterator_range(boost::vertices(_region_graph))) {
        const auto triangles =
            triangulate_rings(_region_graph[region_id].polygons, _global_vertices);
        if(!triangles) {
            // Already checked within add_region and connect_regions - but kept for safety.
            throw SimulationError("Region {} has self-intersecting boundaries.", region_id);
        }

        for(const auto& triangle : *triangles) {
            const auto added = mesh.add_face(
                mesh_vertex(triangle[0]), mesh_vertex(triangle[1]), mesh_vertex(triangle[2]));
            if(added == epick::Mesh::null_face()) {
                throw SimulationError(
                    "Region {} does not fit to a walkable surface, check its connectors.",
                    region_id);
            }
            if(region_split.region.size() != added.idx()) {
                throw SimulationError(
                    "Internal Error: Added face id {} does not match expected id {}",
                    added.idx(),
                    region_split.region.size());
            }
            region_split.region.push_back(region_id);
        }
    }

    // Regions and connectors are counterclockwise, so every face points upwards in exact
    // arithmetic. A face whose double normal does not is degenerate: one of its corners lies
    // within rounding distance of the opposite edge, which the vertex clearance does not catch.
    // Reported here, as validate_mesh would only call it too steep.
    for(const auto face : mesh.faces()) {
        if(CGAL::Polygon_mesh_processing::compute_face_normal(face, mesh).z() <= 0) {
            throw SimulationError(
                "Region {} has a vertex (almost) on one of its edges; move it at least {} m away.",
                region_split.region[face.idx()],
                min_clearance);
        }
    }
    validate_mesh(mesh, &region_split.region);

    return std::make_unique<Geometry>(
        std::move(mesh), std::move(region_split), create_region_graph_2d());
}
