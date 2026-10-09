// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/walkable_surface.hpp"

#include "geometry/geometry.hpp"
#include "geometry/validation.hpp"
#include "simulation_error.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <boost/range/iterator_range.hpp>

#include <cmath>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace
{
/// Triangulate one region (boundary + holes)
/// Returns the triangles as triples of those same indices, or nothing when the rings intersect each
/// other.
std::optional<std::vector<std::array<size_t, 3>>> triangulate_rings(
    const std::vector<std::vector<size_t>>& rings,
    const std::vector<Point3D>& vertices)
{
    CDT cdt{};
    std::unordered_map<CDT::Vertex_handle, size_t> cdt_vertices{};
    for(const auto& ring : rings) {
        std::vector<CDT::Vertex_handle> handles{};
        handles.reserve(ring.size());
        for(const size_t vertex_id : ring) {
            const Point3D& vertex = vertices[vertex_id];
            const auto handle = cdt.insert(Point2D(vertex[0], vertex[1]));
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

PolyWithHoles as_2d_poly_with_holes(
    const std::vector<std::vector<size_t>>& rings,
    const std::vector<Point3D>& vertices)
{
    // lambda fct: PolyWithHoles needs boundary counterclockwise and holes clockwise.
    const auto oriented_poly =
        [&vertices](const std::vector<size_t>& ring, CGAL::Orientation wanted) {
            Poly poly{};
            for(const size_t vertex_id : ring) {
                const Point3D& v = vertices[vertex_id];
                poly.push_back(Point2D(v[0], v[1]));
            }
            if(poly.orientation() != wanted) {
                poly.reverse_orientation();
            }
            return poly;
        };

    std::vector<Poly> holes{};
    holes.reserve(rings.size() - 1);
    for(size_t index = 1; index < rings.size(); ++index) {
        holes.emplace_back(oriented_poly(rings[index], CGAL::CLOCKWISE));
    }
    return PolyWithHoles(
        oriented_poly(rings[0], CGAL::COUNTERCLOCKWISE), std::begin(holes), std::end(holes));
}

Geometry::SeamEdge find_seam_edge(const PolyWithHoles& poly, const Point2D& a, const Point2D& b)
{
    const auto find_in_ring = [&a, &b](const Poly& ring) -> std::optional<size_t> {
        for(size_t index = 0; index < ring.size(); ++index) {
            const Point2D& p = ring.vertex(index);
            const Point2D& q = ring.vertex((index + 1) % ring.size());
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
// WalkableSurface
//==================================================================================================
size_t WalkableSurface::add_region(Polygon polygon, double height)
{
    // Build up locally first. If an error is thrown, the internal structures are still fine.
    std::vector<Point3D> vertices{};
    std::vector<std::vector<size_t>> polygons{};

    auto convert_ring = [&vertices, &polygons, height](const Ring& ring) {
        // Deal with first and last point being the same.
        const size_t count = ring.size() > 1 && (ring.back() - ring.front()).is_zero_length() ?
                                 ring.size() - 1 :
                                 ring.size();
        if(count < 3) {
            throw SimulationError("boundary/hole needs at least 3 different points");
        }
        std::vector<size_t> converted_ring;
        converted_ring.reserve(count);
        for(size_t index = 0; index < count; ++index) {
            converted_ring.push_back(vertices.size());
            vertices.push_back({ring[index].x, ring[index].y, height});
        }
        polygons.emplace_back(std::move(converted_ring));
    };

    convert_ring(polygon.boundary);
    for(const Ring& ring : polygon.holes) {
        convert_ring(ring);
    }

    return insert_region(std::move(polygons), vertices, height);
}

size_t WalkableSurface::add_region(const PolyWithHoles& polygon, double height)
{
    std::vector<Point3D> vertices{};
    std::vector<std::vector<size_t>> polygons{};

    auto convert_ring = [&vertices, &polygons, height](const Poly& ring) {
        std::vector<size_t> converted_ring{};
        converted_ring.reserve(ring.size());
        for(const Point2D& p : ring.container()) {
            converted_ring.push_back(vertices.size());
            vertices.push_back({p.x(), p.y(), height});
        }
        polygons.emplace_back(std::move(converted_ring));
    };

    convert_ring(polygon.outer_boundary());
    for(const Poly& hole : polygon.holes()) {
        convert_ring(hole);
    }

    return insert_region(std::move(polygons), vertices, height);
}

size_t WalkableSurface::insert_region(
    std::vector<std::vector<size_t>> polygons,
    const std::vector<Point3D>& vertices,
    double height)
{
    // Before orienting the rings: CGAL's orientation() requires simple polygons.
    for(const auto& ring : polygons) {
        Poly poly{};
        for(const size_t vertex_id : ring) {
            poly.push_back(Point2D(vertices[vertex_id][0], vertices[vertex_id][1]));
        }
        if(!poly.is_simple()) {
            throw SimulationError("boundary/hole is not simple");
        }
    }
    const PolyWithHoles poly_with_holes = as_2d_poly_with_holes(polygons, vertices);
    validate_floor_overlap(poly_with_holes, height);

    // Fix vertex indices and insert them into the global map.
    const size_t base = _global_vertices.size();
    for(auto& ring : polygons) {
        for(auto& index : ring) {
            index += base;
        }
    }
    _global_vertices.insert(std::end(_global_vertices), std::begin(vertices), std::end(vertices));

    const Region region{
        .polygons = std::move(polygons), .height = height, .poly_with_holes = poly_with_holes};
    return boost::add_vertex(region, _region_graph);
}

std::array<size_t, 2> WalkableSurface::find_edge(size_t region_id, const LineSegment& edge) const
{
    // no need to compare z as regions by design cannot overlap in xy within a single region
    auto match_in_2d = [this](size_t vertex_id, const Point& p) {
        const Point3D& global_vertex = _global_vertices[vertex_id];
        return (Point(global_vertex[0], global_vertex[1]) - p).is_zero_length();
    };

    const Region& region = _region_graph[region_id];
    for(const auto& polygon : region.polygons) {
        for(size_t index = 0; index < polygon.size(); ++index) {
            const size_t current = polygon[index];
            const size_t next = polygon[(index + 1) % polygon.size()];
            if(match_in_2d(current, edge.p1) && match_in_2d(next, edge.p2)) {
                return {current, next};
            }
            if(match_in_2d(current, edge.p2) && match_in_2d(next, edge.p1)) {
                return {next, current};
            }
        }
    }
    throw SimulationError("{} is not an edge of region {}.", edge, region_id);
}

size_t WalkableSurface::connect_regions(
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

    if(CGAL::do_intersect(
           Segment2D({from.p2.x, from.p2.y}, {to.p1.x, to.p1.y}),
           Segment2D({to.p2.x, to.p2.y}, {from.p1.x, from.p1.y}))) {
        // flip one LS
        std::swap(from.p1, from.p2);
    }

    const auto from_edge = find_edge(from_region, from);
    const auto to_edge = find_edge(to_region, to);
    std::vector<size_t> connector_polygon{from_edge[0], from_edge[1], to_edge[0], to_edge[1]};
    const Point3D& p1 = _global_vertices[connector_polygon[0]];
    const Point3D& p2 = _global_vertices[connector_polygon[1]];
    const Point3D& p3 = _global_vertices[connector_polygon[2]];
    const Point3D& p4 = _global_vertices[connector_polygon[3]];
    const std::array<Point2D, 4> corners_2d{
        Point2D(p1.x(), p1.y()),
        Point2D(p2.x(), p2.y()),
        Point2D(p3.x(), p3.y()),
        Point2D(p4.x(), p4.y())};
    if(!Poly(std::begin(corners_2d), std::end(corners_2d)).is_simple()) {
        throw SimulationError("Connector is no simple polygon in 2D.");
    }
    if(!CGAL::coplanar(p1, p2, p3, p4)) {
        throw SimulationError("Connector is not planar");
    };

    // Accept the case where P1, P2, P3, P4 form a triangle in 2D (on same height).
    const K::Plane_3 plane =
        CGAL::collinear(p1, p2, p3) ? K::Plane_3(p1, p2, p4) : K::Plane_3(p1, p2, p3);
    auto normal = plane.orthogonal_vector() / std::sqrt(plane.orthogonal_vector().squared_length());
    if(normal.z() < 0) {
        // is_walkable_normal expects a certain orientation.
        normal = -normal;
    }
    if(!is_walkable_normal(normal)) {
        throw SimulationError("Connector is too steep.");
    }

    Region connector{
        .polygons = {connector_polygon},
        .height = std::nullopt,
        .poly_with_holes = as_2d_poly_with_holes({connector_polygon}, _global_vertices)};
    const auto connector_region = boost::add_vertex(connector, _region_graph);
    boost::add_edge(from_region, connector_region, from_edge, _region_graph);
    boost::add_edge(connector_region, to_region, to_edge, _region_graph);
    return connector_region;
}

std::unique_ptr<WalkableSurface::RegionGraph2D> WalkableSurface::create_region_graph_2d() const
{
    const auto as_point_2d = [this](size_t vertex_id) {
        const Point3D& v = _global_vertices[vertex_id];
        return Point2D(v[0], v[1]);
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
        const Point2D a = as_point_2d(seam[0]);
        const Point2D b = as_point_2d(seam[1]);
        boost::add_edge(from, to, find_seam_edge((*graph)[from], a, b), *graph);
        boost::add_edge(to, from, find_seam_edge((*graph)[to], a, b), *graph);
    }

    return graph;
}

void WalkableSurface::validate_floor_overlap(const PolyWithHoles& poly_with_holes, double height)
    const
{
    for(const auto region_id : boost::make_iterator_range(boost::vertices(_region_graph))) {
        const Region& region = _region_graph[region_id];
        if(region.height != height) {
            continue;
        }
        // Check overlap or even if regions touch each other.
        const auto side = CGAL::oriented_side(region.poly_with_holes, poly_with_holes);
        if(side != CGAL::ON_NEGATIVE_SIDE) {
            throw SimulationError(
                side == CGAL::ON_POSITIVE_SIDE ? "New region overlaps with region {}." :
                                                 "New region touches region {}.",
                region_id);
        }
    }
}

std::unique_ptr<Geometry> WalkableSurface::create_geometry()
{
    SurfaceMesh mesh{};
    RegionSplit region_split{{}, boost::num_vertices(_region_graph)};

    // Map _global_vertices to CGAL mesh vertices.
    std::vector<SurfaceMesh::Vertex_index> mesh_vertices(
        _global_vertices.size(), SurfaceMesh::null_vertex());
    auto mesh_vertex = [this, &mesh, &mesh_vertices](size_t vertex_id) {
        auto& mesh_vertex = mesh_vertices[vertex_id];
        if(mesh_vertex == SurfaceMesh::null_vertex()) {
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
            if(added == SurfaceMesh::null_face()) {
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

    normalise_and_validate_mesh(mesh, &region_split.region);

    return std::make_unique<Geometry>(
        std::move(mesh), std::move(region_split), create_region_graph_2d());
}
