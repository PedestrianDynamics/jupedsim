// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"

#include "SimulationError.hpp"

#include <CGAL/mark_domain_in_triangulation.h>

#include <array>
#include <cassert>
#include <cmath>
#include <iterator>
#include <map>
#include <utility>
#include <variant>
#include <vector>

namespace
{
/// The on-surface 3D point of face @p f at (x,y) @p q. @p q is assumed inside
/// f's (x,y) projection. z is the barycentric blend of the vertices' heights:
/// find q's weights in the projected triangle, then interpolate z.
/// Basically: (FaceId f, Point2D q) --> Point3D
Point3D point_on_face(const SurfaceMesh& mesh, SurfaceMesh::Face_index f, const Point2D& q)
{
    std::array<Point3D, 3> v{};
    int i = 0;
    for(const auto vh : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        v[i++] = mesh.point(vh);
    }
    const double e1x = v[1].x() - v[0].x();
    const double e1y = v[1].y() - v[0].y();
    const double e2x = v[2].x() - v[0].x();
    const double e2y = v[2].y() - v[0].y();
    const double qx = q.x() - v[0].x();
    const double qy = q.y() - v[0].y();
    const double denom = e1x * e2y - e2x * e1y; // 2*signed area; != 0 unless f is vertical
    assert(denom != 0.0 && "FATAL: vertical face in point_on_face");
    const double b1 = (qx * e2y - e2x * qy) / denom;
    const double b2 = (e1x * qy - qx * e1y) / denom;
    const double z = (1.0 - b1 - b2) * v[0].z() + b1 * v[1].z() + b2 * v[2].z();
    return Point3D{q.x(), q.y(), z};
}

/// True iff (x,y) point @p q lies in face @p f's (x,y) footprint (inside or on
/// its boundary).
bool face_covers(const SurfaceMesh& mesh, SurfaceMesh::Face_index f, const Point2D& q)
{
    std::array<Point2D, 3> p{};
    int i = 0;
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        const auto& s = mesh.point(v);
        p[i++] = Point2D{s.x(), s.y()};
    }
    return Triangle2D(p[0], p[1], p[2]).bounded_side(q) != CGAL::ON_UNBOUNDED_SIDE;
}
} // namespace

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

Geometry3D::FaceLocation Geometry3D::walk_on_surface(
    std::size_t from_region_id,
    const Point2D& from,
    const Point2D& to) const
{
    const auto start = locate_in_region(from_region_id, from).face;
    if(start == SurfaceMesh::null_face()) {
        throw SimulationError("walk_on_surface(): Invalid 'from'");
    }

    // A single agent step is assumed small relative to the triangle edge length,
    // so 'to' lies in the start face or one directly touching it (its vertex
    // 1-ring). Restricting to that surface neighbourhood -- rather than any
    // face whose (x,y) footprint contains 'to' -- is what keeps the agent on
    // the right sheet where regions overlap.
    if(face_covers(_mesh, start, to)) {
        return {start, point_on_face(_mesh, start, to)};
    }
    for(const auto v : CGAL::vertices_around_face(_mesh.halfedge(start), _mesh)) {
        for(const auto f : CGAL::faces_around_target(_mesh.halfedge(v), _mesh)) {
            if(f == SurfaceMesh::null_face() || f == start) {
                continue;
            }
            if(face_covers(_mesh, f, to)) {
                return {f, point_on_face(_mesh, f, to)};
            }
        }
    }
    // TODO: 'to' is neither in the start face nor a direct neighbour.
    //        This indicates a "manual" override to position that is larger than
    //        expected. We need to see how to deal with this. Right now it is an error.
    throw SimulationError("walk_on_surface(): 'to' is not in the start face or its neighbourhood");
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
