// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"

#include "SimulationError.hpp"

#include <CGAL/mark_domain_in_triangulation.h>

//==================================================================================================
// WalkableSurface
//==================================================================================================
size_t WalkableSurface::AddRegion(Polygon polygon, double height)
{
    return boost::add_vertex(Region{std::move(polygon), height}, _regionGraph);
}

size_t WalkableSurface::ConnectRegions(
    size_t fromRegion,
    LineSegment from,
    size_t toRegion,
    LineSegment to)
{
    if(fromRegion >= boost::num_vertices(_regionGraph)) {
        throw SimulationError("Unknown region id used for fromRegion: {}", fromRegion);
    }
    if(toRegion >= boost::num_vertices(_regionGraph)) {
        throw SimulationError("Unknown region id used for toRegion: {}", toRegion);
    }
    if(fromRegion == toRegion) {
        throw SimulationError("fromRegion and toRegion may not be the same region.");
    }
    const auto asPoint3D = [](const Point& p, double height) { return Point3D{p.x, p.y, height}; };
    const auto fromHeight = _regionGraph[fromRegion].height;
    const auto toHeight = _regionGraph[toRegion].height;
    if(!CGAL::coplanar(
           asPoint3D(from.p1, fromHeight),
           asPoint3D(from.p2, fromHeight),
           asPoint3D(to.p1, toHeight),
           asPoint3D(to.p2, toHeight))) {
        throw SimulationError("Connector not planar");
    };

    if(CGAL::do_intersect(
           Segment2D({from.p2.x, from.p2.y}, {to.p1.x, to.p1.y}),
           Segment2D({to.p2.x, to.p2.y}, {from.p1.x, from.p1.y}))) {
        // flip one LS
        std::swap(from.p1, from.p2);
    }

    boost::add_edge(fromRegion, toRegion, Connector{from, to}, _regionGraph);
    return boost::num_edges(_regionGraph) - 1;
}

std::unique_ptr<SurfaceMesh> WalkableSurface::CreateMesh()
{
    // 1. Build mesh object
    // 1.1 Delauny Triangulate all input polygons and add to mesh
    SurfaceMesh mesh{};
    for(const auto& region : boost::make_iterator_range(boost::vertices(_regionGraph))) {
        const auto& r = _regionGraph[region];
        const auto as_point_2d =
            std::views::transform([](const Point& p) { return Point2D(p.x, p.y); });
        CDT cdt{};
        // 1.1.1 Mark outer and interior boundaries in 2D
        auto boundary = r.polygon.boundary | as_point_2d;
        cdt.insert_constraint(std::begin(boundary), std::end(boundary), true);
        for(const auto& hole : r.polygon.holes) {
            auto ring = hole | as_point_2d;
            cdt.insert_constraint(std::begin(ring), std::end(ring), true);
        }
        CGAL::mark_domain_in_triangulation(cdt);

        // 1.1.2 While add vertices, add the height
        std::unordered_map<CDT::Vertex_handle, SurfaceMesh::Vertex_index> vmap{};
        for(const auto& v : cdt.finite_vertex_handles()) {
            const auto& p = v->point();
            Point3D p_3d{p.x(), p.y(), r.height};
            vmap.emplace(v, mesh.add_vertex(p_3d));
        }

        // 1.1.3 Add faces
        for(const auto& f : cdt.finite_face_handles()) {
            if(!f->get_in_domain()) { // Only add faces inside the polygon.
                continue;
            }
            mesh.add_face(vmap[f->vertex(0)], vmap[f->vertex(1)], vmap[f->vertex(2)]);
        }
    }
    // 1.2

    //
    return std::make_unique<SurfaceMesh>(std::move(mesh));
}
