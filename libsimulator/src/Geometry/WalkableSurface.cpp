// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"

#include "Geometry/Geometry.hpp"
#include "Geometry/Validation.hpp"
#include "SimulationError.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <set>
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
    std::unordered_map<CDT::Vertex_handle, size_t> cdtVertices{};
    for(const auto& ring : rings) {
        std::vector<CDT::Vertex_handle> handles{};
        handles.reserve(ring.size());
        for(const size_t vertexID : ring) {
            const Point3D& vertex = vertices[vertexID];
            const auto handle = cdt.insert(Point2D(vertex[0], vertex[1]));
            cdtVertices.emplace(handle, vertexID);
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
            const auto iter = cdtVertices.find(face->vertex(corner));
            if(iter == cdtVertices.end()) {
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
            for(const size_t vertexID : ring) {
                const Point3D& v = vertices[vertexID];
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

} // namespace

//==================================================================================================
// WalkableSurface
//==================================================================================================
size_t WalkableSurface::AddRegion(Polygon polygon, double height)
{
    // Build up locally first. If an error is thrown, the internal structures are still fine.
    std::vector<Point3D> vertices{};
    std::vector<std::vector<size_t>> polygons{};

    auto convert_ring = [&vertices, &polygons, height](const Ring& ring) {
        // Deal with first and last point being the same.
        const size_t count = ring.size() > 1 && (ring.back() - ring.front()).isZeroLength() ?
                                 ring.size() - 1 :
                                 ring.size();
        if(count < 3) {
            throw SimulationError("boundary/hole needs at least 3 different points");
        }
        Poly poly{};
        std::vector<size_t> converted_ring;
        converted_ring.reserve(count);
        for(size_t index = 0; index < count; ++index) {
            poly.push_back(Point2D(ring[index].x, ring[index].y));
            converted_ring.push_back(vertices.size());
            vertices.push_back({ring[index].x, ring[index].y, height});
        }
        if(!poly.is_simple()) {
            throw SimulationError("boundary/hole is not simple");
        }
        polygons.emplace_back(std::move(converted_ring));
    };

    convert_ring(polygon.boundary);
    for(const Ring& ring : polygon.holes) {
        convert_ring(ring);
    }

    const PolyWithHoles polyWithHoles = as_2d_poly_with_holes(polygons, vertices);
    // CGAL's is_valid_polygon_with_holes() allows that boundary and holes touch at vertices:
    // Check whether all vertices are unique.
    std::set<Point2D> points{};
    const bool pointsUnique = std::ranges::all_of(
        vertices, [&points](const Point3D& v) { return points.emplace(v.x(), v.y()).second; });
    if(!pointsUnique ||
       !CGAL::is_valid_polygon_with_holes(polyWithHoles, CGAL::Gps_segment_traits_2<K>{})) {
        throw SimulationError("Holes must lie strictly inside the boundary and may not overlap.");
    }

    ValidateFloorOverlap(polyWithHoles, height);

    // Fix vertex indices and insert them into the global map.
    const size_t base = _globalVertices.size();
    for(auto& ring : polygons) {
        for(auto& index : ring) {
            index += base;
        }
    }
    _globalVertices.insert(std::end(_globalVertices), std::begin(vertices), std::end(vertices));

    const Region region{
        .polygons = std::move(polygons), .height = height, .polyWithHoles = polyWithHoles};
    return boost::add_vertex(region, _regionGraph);
}

std::array<size_t, 2> WalkableSurface::FindEdge(size_t regionId, const LineSegment& edge) const
{
    // no need to compare z as regions by design cannot overlap in xy within a single region
    auto match_in_2d = [this](size_t vertexID, const Point& p) {
        const Point3D& globalVertex = _globalVertices[vertexID];
        return (Point(globalVertex[0], globalVertex[1]) - p).isZeroLength();
    };

    const Region& region = _regionGraph[regionId];
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
    throw SimulationError("{} is not an edge of region {}.", edge, regionId);
}

size_t WalkableSurface::ConnectRegions(
    size_t fromRegion,
    LineSegment from,
    size_t toRegion,
    LineSegment to)
{
    auto check_region = [this](size_t region_id, std::string_view name) {
        if(region_id >= boost::num_vertices(_regionGraph)) {
            throw SimulationError("Unknown region id used for {}: {}", name, region_id);
        }
        const Region& region = _regionGraph[region_id];
        if(!region.is_connectable()) {
            throw SimulationError("{} {} is not connectable", name, region_id);
        }
    };
    check_region(fromRegion, "fromRegion");
    check_region(toRegion, "toRegion");
    if(fromRegion == toRegion) {
        throw SimulationError("fromRegion and toRegion may not be the same region.");
    }

    if(CGAL::do_intersect(
           Segment2D({from.p2.x, from.p2.y}, {to.p1.x, to.p1.y}),
           Segment2D({to.p2.x, to.p2.y}, {from.p1.x, from.p1.y}))) {
        // flip one LS
        std::swap(from.p1, from.p2);
    }

    const auto fromEdge = FindEdge(fromRegion, from);
    const auto toEdge = FindEdge(toRegion, to);
    std::vector<size_t> connector_polygon{fromEdge[0], fromEdge[1], toEdge[0], toEdge[1]};
    const Point3D& p1 = _globalVertices[connector_polygon[0]];
    const Point3D& p2 = _globalVertices[connector_polygon[1]];
    const Point3D& p3 = _globalVertices[connector_polygon[2]];
    const Point3D& p4 = _globalVertices[connector_polygon[3]];
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
        // IsWalkableNormal expects a certain orientation.
        normal = -normal;
    }
    if(!IsWalkableNormal(normal)) {
        throw SimulationError("Connector is too steep.");
    }

    Region connector{
        .polygons = {connector_polygon},
        .height = std::nullopt,
        .polyWithHoles = as_2d_poly_with_holes({connector_polygon}, _globalVertices)};
    const auto connectorRegion = boost::add_vertex(connector, _regionGraph);
    boost::add_edge(fromRegion, connectorRegion, fromEdge, _regionGraph);
    boost::add_edge(connectorRegion, toRegion, toEdge, _regionGraph);
    return connectorRegion;
}

WalkableSurface::RegionGraph2D WalkableSurface::CreateRegionGraph2D() const
{
    const auto as_point_2d = [this](size_t vertexID) {
        const Point3D& v = _globalVertices[vertexID];
        return Point2D(v[0], v[1]);
    };

    RegionGraph2D graph{};
    // Add vertices.
    for(const auto regionID : boost::make_iterator_range(boost::vertices(_regionGraph))) {
        boost::add_vertex(_regionGraph[regionID].polyWithHoles, graph);
    }

    // Add seams.
    for(const auto& edge : boost::make_iterator_range(boost::edges(_regionGraph))) {
        const auto from = boost::source(edge, _regionGraph);
        const auto to = boost::target(edge, _regionGraph);
        const Seam& seam = _regionGraph[edge];
        boost::add_edge(from, to, Segment2D(as_point_2d(seam[0]), as_point_2d(seam[1])), graph);
        boost::add_edge(to, from, Segment2D(as_point_2d(seam[1]), as_point_2d(seam[0])), graph);
    }

    return graph;
}

void WalkableSurface::ValidateFloorOverlap(const PolyWithHoles& polyWithHoles, double height) const
{
    for(const auto regionID : boost::make_iterator_range(boost::vertices(_regionGraph))) {
        const Region& region = _regionGraph[regionID];
        if(region.height != height) {
            continue;
        }
        // Check overlap or even if regions touch each other.
        const auto side = CGAL::oriented_side(region.polyWithHoles, polyWithHoles);
        if(side != CGAL::ON_NEGATIVE_SIDE) {
            throw SimulationError(
                side == CGAL::ON_POSITIVE_SIDE ? "New region overlaps with region {}." :
                                                 "New region touches region {}.",
                regionID);
        }
    }
}

std::unique_ptr<Geometry> WalkableSurface::CreateGeometry()
{
    SurfaceMesh mesh{};
    RegionSplit region_split{{}, boost::num_vertices(_regionGraph)};

    // Map _globalVertices to CGAL mesh vertices.
    std::vector<SurfaceMesh::Vertex_index> meshVertices(
        _globalVertices.size(), SurfaceMesh::null_vertex());
    auto mesh_vertex = [this, &mesh, &meshVertices](size_t vertexID) {
        auto& meshVertex = meshVertices[vertexID];
        if(meshVertex == SurfaceMesh::null_vertex()) {
            meshVertex = mesh.add_vertex(_globalVertices[vertexID]);
        }
        return meshVertex;
    };

    for(const auto regionID : boost::make_iterator_range(boost::vertices(_regionGraph))) {
        const auto triangles = triangulate_rings(_regionGraph[regionID].polygons, _globalVertices);
        if(!triangles) {
            // Already checked within AddRegion and ConnectRegions - but kept for safety.
            throw SimulationError("Region {} has self-intersecting boundaries.", regionID);
        }

        for(const auto& triangle : *triangles) {
            const auto added = mesh.add_face(
                mesh_vertex(triangle[0]), mesh_vertex(triangle[1]), mesh_vertex(triangle[2]));
            if(added == SurfaceMesh::null_face()) {
                throw SimulationError(
                    "Region {} does not fit to a walkable surface, check its connectors.",
                    regionID);
            }
            if(region_split.region.size() != added.idx()) {
                throw SimulationError(
                    "Internal Error: Added face id {} does not match expected id {}",
                    added.idx(),
                    region_split.region.size());
            }
            region_split.region.push_back(regionID);
        }
    }

    NormaliseAndValidateMesh(mesh, &region_split.region);

    return std::make_unique<Geometry>(std::move(mesh), std::move(region_split));
}
