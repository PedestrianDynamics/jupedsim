// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"

#include "Geometry/Geometry.hpp"
#include "Geometry/Validation.hpp"
#include "SimulationError.hpp"

#include <CGAL/mark_domain_in_triangulation.h>
#include <boost/range/iterator_range.hpp>

#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

//==================================================================================================
// WalkableSurface
//==================================================================================================
size_t WalkableSurface::AddRegion(Polygon polygon, double height)
{
    auto convert_ring = [this, height](const Ring& ring) {
        // Deal with first and last point being the same.
        const size_t count = ring.size() > 1 && (ring.back() - ring.front()).isZeroLength() ?
                                 ring.size() - 1 :
                                 ring.size();
        if(count < 3) {
            throw SimulationError("boundary/hole needs at least 3 different points");
        }
        std::vector<size_t> converted_ring;
        converted_ring.reserve(count);
        for(size_t index = 0; index < count; ++index) {
            converted_ring.push_back(_globalVertices.size());
            _globalVertices.push_back({ring[index].x, ring[index].y, height});
        }
        return converted_ring;
    };

    std::vector<std::vector<size_t>> polygons{convert_ring(polygon.boundary)};
    for(const Ring& ring : polygon.holes) {
        polygons.emplace_back(convert_ring(ring));
    }

    const Region region{.polygons = std::move(polygons), .connectable = true};
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
        if(!region.connectable) {
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
    if(!CGAL::coplanar(p1, p2, p3, p4)) {
        throw SimulationError("Connector not planar");
    };

    const K::Plane_3 plane(p1, p2, p3);
    if(plane.is_degenerate()) {
        throw SimulationError("Connector between {} and {} has no area.", from, to);
    }
    auto normal = plane.orthogonal_vector() / std::sqrt(plane.orthogonal_vector().squared_length());
    if(normal.z() < 0) {
        // IsWalkableNormal expects a certain orientation.
        normal = -normal;
    }
    if(!IsWalkableNormal(normal)) {
        throw SimulationError("Connector between {} and {} is too steep.", from, to);
    }

    Region connector{.polygons = {connector_polygon}, .connectable = false};
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
    const auto as_poly = [&as_point_2d](const std::vector<size_t>& ring) {
        Poly poly{};
        for(const size_t vertexID : ring) {
            poly.push_back(as_point_2d(vertexID));
        }
        return poly;
    };

    RegionGraph2D graph{};

    // PolyWithHoles needs boundary counterclockwise and holes clockwise.
    const auto oriented = [&as_poly](const std::vector<size_t>& ring, CGAL::Orientation wanted) {
        Poly poly = as_poly(ring);
        if(poly.orientation() != wanted) {
            poly.reverse_orientation();
        }
        return poly;
    };

    for(const auto regionID : boost::make_iterator_range(boost::vertices(_regionGraph))) {
        const auto& polygons = _regionGraph[regionID].polygons;
        std::vector<Poly> holes{};
        holes.reserve(polygons.size() - 1);
        for(size_t index = 1; index < polygons.size(); ++index) {
            holes.emplace_back(oriented(polygons[index], CGAL::CLOCKWISE));
        }
        boost::add_vertex(
            PolyWithHoles(
                oriented(polygons[0], CGAL::COUNTERCLOCKWISE), std::begin(holes), std::end(holes)),
            graph);
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
        const Region& region = _regionGraph[regionID];

        // Triangulate the region in 2D.
        CDT cdt{};
        std::unordered_map<CDT::Vertex_handle, size_t> cdtVertices{};
        for(const auto& polygon : region.polygons) {
            std::vector<CDT::Vertex_handle> handles{};
            handles.reserve(polygon.size());
            for(const size_t vertexID : polygon) {
                const Point3D& globalVertex = _globalVertices[vertexID];
                const auto handle = cdt.insert(Point2D(globalVertex[0], globalVertex[1]));
                // Remember CGAL-->globalVertices mapping
                cdtVertices.emplace(handle, vertexID);
                handles.emplace_back(handle);
            }
            for(size_t index = 1; index < handles.size(); ++index) {
                cdt.insert_constraint(handles[index - 1], handles[index]);
            }
            cdt.insert_constraint(handles.back(), handles.front()); // close the ring
        }
        CGAL::mark_domain_in_triangulation(cdt);

        auto global_vertex = [&cdtVertices, regionID](CDT::Vertex_handle handle) {
            const auto iter = cdtVertices.find(handle);
            if(iter == cdtVertices.end()) {
                // A vertex not in the map got added by triangulation ("Steiner point")
                throw SimulationError("Region {} has self-intersecting boundaries.", regionID);
            }
            return iter->second;
        };

        for(const auto& face : cdt.finite_face_handles()) {
            if(!face->get_in_domain()) { // Only add faces inside the polygon.
                continue;
            }
            // Note: CDT faces are oriented ccw in 2D. This means we do not need to check the
            //       order of points for connectors.
            const auto added = mesh.add_face(
                mesh_vertex(global_vertex(face->vertex(0))),
                mesh_vertex(global_vertex(face->vertex(1))),
                mesh_vertex(global_vertex(face->vertex(2))));
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

    NormaliseAndValidateMesh(mesh);

    return std::make_unique<Geometry>(std::move(mesh), std::move(region_split));
}
