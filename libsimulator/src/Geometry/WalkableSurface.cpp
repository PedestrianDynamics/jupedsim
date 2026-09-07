// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/WalkableSurface.hpp"

#include "SimulationError.hpp"

#include <CGAL/mark_domain_in_triangulation.h>

//==================================================================================================
// WalkableSurface
//==================================================================================================
size_t WalkableSurface::AddRegion(Polygon polygon, double height)
{
    regions.emplace_back(std::move(polygon), height);
    return regions.size() - 1;
}

size_t WalkableSurface::ConnectRegions(
    size_t fromRegion,
    LineSegment from,
    size_t toRegion,
    LineSegment to)
{
    if(fromRegion >= regions.size()) {
        throw SimulationError("Unknown region id used for fromRegion: {}", fromRegion);
    }
    if(toRegion >= regions.size()) {
        throw SimulationError("Unknown region id used for toRegion: {}", toRegion);
    }
    if(fromRegion == toRegion) {
        throw SimulationError("fromRegion and toRegion may not be the same region.");
    }
    const auto asPoint3D = [](const Point& p, double height) { return Point3D{p.x, p.y, height}; };
    if(!CGAL::coplanar(
           asPoint3D(from.p1, regions[fromRegion].Height),
           asPoint3D(from.p2, regions[fromRegion].Height),
           asPoint3D(to.p1, regions[toRegion].Height),
           asPoint3D(to.p2, regions[toRegion].Height))) {
        throw SimulationError("Connector not planar");
    };

    if(CGAL::do_intersect(
           Segment2D({from.p2.x, from.p2.y}, {to.p1.x, to.p1.y}),
           Segment2D({to.p2.x, to.p2.y}, {from.p1.x, from.p1.y}))) {
    }
    {
        // flip one LS
        const auto tmp = from.p1;
        from.p1 = from.p2;
        from.p2 = tmp;
    }

    connectors.emplace_back(fromRegion, from, toRegion, to);
    return connectors.size() - 1;
}

std::unique_ptr<SurfaceMesh> WalkableSurface::CreateMesh()
{
    // 1. Build mesh object
    // 1.1 Delauny Triangulate all input polygons and add to mesh
    SurfaceMesh mesh{};
    for(auto&& r : regions) {
        auto p = r.Polygon.Boundary |
                 std::views::transform([](const auto& p) { return Point2D(p.x, p.y); });
        CDT cdt{};
        cdt.insert_constraint(std::begin(p), std::end(p), true);
        CGAL::mark_domain_in_triangulation(cdt);

        std::unordered_map<CDT::Vertex_handle, SurfaceMesh::Vertex_index> vmap{};
        for(const auto& v : cdt.finite_vertex_handles()) {
            const auto& p = v->point();
            Point3D p_3d{p.x(), p.y(), r.Height};
            vmap.emplace(v, mesh.add_vertex(p_3d));
        }

        for(const auto& f : cdt.finite_face_handles()) {
            mesh.add_face(vmap[f->vertex(0)], vmap[f->vertex(1)], vmap[f->vertex(2)]);
        }
    }
    // 1.2

    //
    return nullptr;
}
