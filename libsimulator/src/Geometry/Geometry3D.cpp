// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"

#include "SimulationError.hpp"

#include <CGAL/mark_domain_in_triangulation.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <variant>
#include <vector>

namespace
{
/// Lift a 2D walkable area to a flat surface mesh at z=0, using the same CDT
/// the 2D RoutingEngine builds (same constraint insertion order): keeping the
/// triangulations identical is what makes 2D/3D results directly comparable.
SurfaceMesh mesh_from_polygon(const PolyWithHoles& poly)
{
    CDT cdt{};
    cdt.insert_constraint(
        poly.outer_boundary().vertices_begin(), poly.outer_boundary().vertices_end(), true);
    for(const auto& hole : poly.holes()) {
        cdt.insert_constraint(hole.vertices_begin(), hole.vertices_end(), true);
    }
    CGAL::mark_domain_in_triangulation(cdt);

    SurfaceMesh mesh{};
    std::map<CDT::Vertex_handle, SurfaceMesh::Vertex_index> idx{};
    const auto vertex_of = [&](CDT::Vertex_handle v) {
        const auto it = idx.find(v);
        if(it != idx.end()) {
            return it->second;
        }
        const auto& p = v->point();
        return idx[v] = mesh.add_vertex({p.x(), p.y(), 0.0});
    };
    for(auto f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); ++f) {
        if(f->get_in_domain()) {
            mesh.add_face(
                vertex_of(f->vertex(0)), vertex_of(f->vertex(1)), vertex_of(f->vertex(2)));
        }
    }
    return mesh;
}

/// Calculate an epsilon to be used for the merge of segments. The max error
/// in float operation depends on the magnitude of numbers - therefore take
/// those into account + an additiona safety factor.
/// WARNING: this assumes the intermediate vertices were produced in double; an
/// externally generated mesh authored in float would need a coarser epsilon.
double collinear_merge_tolerance(const std::vector<LineSegment>& segments)
{
    double scale = 0.0;
    for(const auto& s : segments) {
        for(const auto& p : {s.p1, s.p2}) {
            scale = std::max({scale, std::abs(p.x), std::abs(p.y)});
        }
    }
    scale *= 64.0; // additional safety
    return scale * std::numeric_limits<double>::epsilon();
}
} // namespace

Geometry3D::Geometry3D(SurfaceMesh mesh) : _mesh(std::move(mesh))
{
    build();
}

Geometry3D::Geometry3D(PolyWithHoles poly) : Geometry3D(mesh_from_polygon(poly))
{
    _geometry2D = std::make_unique<Geometry2D>(std::move(poly));
}

void Geometry3D::build()
{
    // Compact vertex/face indices so vertices()/triangles()/region_id_per_face() are
    // contiguous and 1:1 with each other (triangulate_faces may leave removed
    // faces behind).
    _mesh.collect_garbage();
    _aabbTree = std::make_unique<AABBTree>(_mesh.faces().begin(), _mesh.faces().end(), _mesh);
    const auto split = split_into_regions(_mesh);
    _region = split.region;
    _regionCount = split.count;
    build_region_views();
}

void Geometry3D::build_region_views()
{
    const auto xy = [&](SurfaceMesh::Vertex_index v) {
        const auto& p = _mesh.point(v);
        return Point{p.x(), p.y()};
    };

    // Classify every region-boundary halfedge: opposite is a mesh border -> the
    // edge is a WALL; opposite belongs to another region -> the edge is a SEAM
    // (and the neighbour region is recorded). Interior edges (same region on
    // both sides) are ignored. Seams never enter the wall grid.
    std::vector<std::vector<LineSegment>> walls(_regionCount);
    std::vector<std::vector<LineSegment>> seams(_regionCount);
    std::vector<std::set<std::size_t>> neighbors(_regionCount);
    for(const auto f : _mesh.faces()) {
        const auto r = _region[f];
        for(const auto h : CGAL::halfedges_around_face(_mesh.halfedge(f), _mesh)) {
            const LineSegment seg{xy(_mesh.source(h)), xy(_mesh.target(h))};
            const auto opp = _mesh.opposite(h);
            if(_mesh.is_border(opp)) {
                walls[r].push_back(seg);
                continue;
            }
            const auto r2 = _region[_mesh.face(opp)];
            if(r2 != r) {
                seams[r].push_back(seg);
                neighbors[r].insert(r2);
            }
        }
    }

    _regionViews.clear();
    _regionViews.reserve(_regionCount);
    for(std::size_t r = 0; r < _regionCount; ++r) {
        _regionViews.emplace_back(
            r,
            this,
            merge_collinear(walls[r], collinear_merge_tolerance(walls[r])),
            merge_collinear(seams[r], collinear_merge_tolerance(seams[r])),
            std::vector<std::size_t>(neighbors[r].begin(), neighbors[r].end()));
    }
}

const AABBTree& Geometry3D::aabb_tree() const
{
    if(!_aabbTree) {
        throw SimulationError("Geometry3D has no geometry loaded.");
    }
    return *_aabbTree;
}

Geometry3D::FaceLocation Geometry3D::face_below(const Point3D& p) const
{
    // first_intersection along -z returns the hit nearest to the ray source,
    // i.e. the face directly below the query point. The ray starts a hair
    // above p: a query point sitting exactly on the surface may round minimally
    // below its face's plane, and the strictly-downward ray would miss it.
    constexpr double onSurfaceTolerance = 1e-9;
    const Ray3D ray(Point3D{p.x(), p.y(), p.z() + onSurfaceTolerance}, Direction3D(0, 0, -1));
    const auto hit = aabb_tree().first_intersection(ray);
    if(!hit) {
        return {SurfaceMesh::null_face(), K::Point_3{}};
    }
    const auto* projected = std::get_if<K::Point_3>(&hit->first);
    // Assert against vertical faces.
    assert(projected && "FATAL: vertical face hit by the face_below line");
    return {hit->second, *projected};
}

bool Geometry3D::is_valid_location(const Point3D& p) const
{
    return face_below(p).face != SurfaceMesh::null_face();
}

Geometry3D::FaceLocation
Geometry3D::locate_in_region(std::size_t region_id, const Point2D& xy) const
{
    // All intersections along z. Search for the one with the region_id.
    const Line3D vertical(Point3D{xy.x(), xy.y(), 0}, Direction3D(0, 0, 1));
    std::vector<AABBTree::Intersection_and_primitive_id<Line3D>::Type> hits{};
    aabb_tree().all_intersections(vertical, std::back_inserter(hits));

    for(const auto& [where, face] : hits) {
        if(_region[face] != region_id) {
            continue;
        }
        const auto* point = std::get_if<Point3D>(&where);
        // Assert against vertical faces.
        assert(point && "FATAL: vertical face hit by the locate line");
        return {face, *point};
    }
    return {SurfaceMesh::null_face(), Point3D{}};
}

std::optional<Location>
Geometry3D::get_location(double x, double y, double z_hint, double tol) const
{
    const auto face_location = locate_near_z(Point2D{x, y}, z_hint, tol);
    if(face_location.face == SurfaceMesh::null_face()) {
        return std::nullopt;
    }
    return Location{
        this,
        Point{x, y},
        region_of(face_location.face),
        face_location.face,
        face_location.point.z()};
}

Geometry3D::FaceLocation
Geometry3D::locate_near_z(const Point2D& xy, double z, double tolerance) const
{
    const Line3D vertical(Point3D{xy.x(), xy.y(), 0}, Direction3D(0, 0, 1));
    std::vector<AABBTree::Intersection_and_primitive_id<Line3D>::Type> hits{};
    aabb_tree().all_intersections(vertical, std::back_inserter(hits));

    FaceLocation best{SurfaceMesh::null_face(), Point3D{}};
    auto bestDeviation = tolerance;
    for(const auto& [where, face] : hits) {
        const auto* point = std::get_if<Point3D>(&where);
        // Assert against vertical faces.
        assert(point && "FATAL: vertical face hit by the locate line");
        if(const auto deviation = std::abs(point->z() - z); deviation <= bestDeviation) {
            bestDeviation = deviation;
            best = {face, *point};
        }
    }
    return best;
}

std::vector<std::size_t> Geometry3D::region_id_per_face() const
{
    std::vector<std::size_t> ids{};
    ids.reserve(_mesh.number_of_faces());
    for(const auto f : _mesh.faces()) {
        ids.push_back(_region[f]);
    }
    return ids;
}

std::vector<std::array<double, 3>> Geometry3D::vertices() const
{
    std::vector<std::array<double, 3>> out{};
    out.reserve(_mesh.number_of_vertices());
    for(const auto v : _mesh.vertices()) {
        const auto& p = _mesh.point(v);
        out.push_back({p.x(), p.y(), p.z()});
    }
    return out;
}

std::vector<std::array<std::size_t, 3>> Geometry3D::triangles() const
{
    std::vector<std::array<std::size_t, 3>> out{};
    out.reserve(_mesh.number_of_faces());
    for(const auto f : _mesh.faces()) {
        std::array<std::size_t, 3> tri{};
        int i = 0;
        for(const auto v : CGAL::vertices_around_face(_mesh.halfedge(f), _mesh)) {
            tri[i++] = static_cast<std::size_t>(v);
        }
        out.push_back(tri);
    }
    return out;
}
