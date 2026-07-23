// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Location.hpp"

#include "Geometry/Geometry3D.hpp"
#include "SimulationError.hpp"

#include <array>
#include <cassert>
#include <cstddef>

namespace
{
/// The on-surface 3D point of face @p f at (x,y) @p q. @p q is assumed inside
/// f's (x,y) projection. z is the barycentric blend of the vertices' heights:
/// find q's weights in the projected triangle, then interpolate z.
/// Basically: (FaceId f, Point2D q) --> Point3D
Point3D point_on_face(const SurfaceMesh& mesh, SurfaceMesh::Face_index f, const Point2D& q)
{
    std::array<Point3D, 3> v{};
    for(int i = 0; const auto vh : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        v[i++] = mesh.point(vh);
    }
    const double e1x = v[1].x() - v[0].x();
    const double e1y = v[1].y() - v[0].y();
    const double e2x = v[2].x() - v[0].x();
    const double e2y = v[2].y() - v[0].y();
    const double qx = q.x() - v[0].x();
    const double qy = q.y() - v[0].y();
    const double denom = e1x * e2y - e2x * e1y; // 2*signed area; != 0 unless f is vertical
    assert(denom != 0.0 && "FATAL: vertical face in point_on_face()");
    const double b1 = (qx * e2y - e2x * qy) / denom;
    const double b2 = (e1x * qy - qx * e1y) / denom;
    const double z = (1.0 - b1 - b2) * v[0].z() + b1 * v[1].z() + b2 * v[2].z();
    return Point3D{q.x(), q.y(), z};
}

/// (x,y) projection of vertex @p v.
Point2D xy_of(const SurfaceMesh& mesh, SurfaceMesh::Vertex_index v)
{
    const auto& p = mesh.point(v);
    return Point2D{p.x(), p.y()};
}

/// The face incident to @p v whose xy angular sector contains @p direction.
/// This is needed when walking on the surface crosses a vertex. Ties (direction
/// exactly along an edge) resolve to the first match. Returns `null_face()` when
/// no incident face's sector contains @p direction, i.e. the walkable area is
/// left crossing the vertex @p v.
SurfaceMesh::Face_index
fan_face(const SurfaceMesh& mesh, SurfaceMesh::Vertex_index v, const K::Vector_2& direction)
{
    const auto vp = xy_of(mesh, v);
    const auto x = vp + direction; // a point along the direction starting at v
    for(const auto f : CGAL::faces_around_target(mesh.halfedge(v), mesh)) {
        if(f == SurfaceMesh::null_face()) {
            continue;
        }
        // For CCW triangle: The face is the one we need if `x` is left (or equal) to
        // the edges touching vertex v.
        SurfaceMesh::Halfedge_index into_v{};
        for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(f), mesh)) {
            if(mesh.target(h) == v) {
                into_v = h;
                break;
            }
        }
        const auto p_prev = xy_of(mesh, mesh.source(into_v));
        const auto n_next = xy_of(mesh, mesh.target(mesh.next(into_v)));
        if(CGAL::orientation(vp, n_next, x) != CGAL::RIGHT_TURN &&
           CGAL::orientation(p_prev, vp, x) != CGAL::RIGHT_TURN) {
            return f;
        }
    }
    return SurfaceMesh::null_face();
}

/// Exact straight walk from @p from along @p direction across @p mesh to the
/// target `from + direction` starting in @p start_face. Advances face by face
/// through the xy-projected triangulation. Steps may be arbitrarily long (cost
/// O(crossed faces)). Throws if the the walkable area is left. Assumes faces are
/// CCW in the xy projection.
Geometry3D::FaceLocation straight_walk(
    const SurfaceMesh& mesh,
    SurfaceMesh::Face_index start_face,
    const Point2D& from,
    const K::Vector_2& direction)
{
    assert(start_face != SurfaceMesh::null_face());

    const Point2D to = from + direction; // the fixed target, formed once

    // 'to' is covered by face f (interior or on its boundary) iff it is not
    // strictly to the RIGHT of any of f's CCW directed edges.
    const auto covers = [&](SurfaceMesh::Face_index f) {
        for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(f), mesh)) {
            if(CGAL::orientation(xy_of(mesh, mesh.source(h)), xy_of(mesh, mesh.target(h)), to) ==
               CGAL::RIGHT_TURN) {
                return false;
            }
        }
        return true;
    };

    SurfaceMesh::Face_index f = start_face;

    // A straight segment visits each face at most once, plus one possible
    // non-advancing iteration when starting on a vertex (fanning back to the
    // start face); exceeding that means a predicate/topology inconsistency.
    // This is just a safeguard and should never happen.
    const auto maxSteps = mesh.number_of_faces() + 2;
    // Store last vertex if face is entered by vertex. This vertex has to be
    // excluded from the next "hits vertex" test to avoid a potential infinite loop.
    // Note: We never need to reset this as the vertex is bound to the current triangle
    //       so that we never skip a "vertex of interest".
    SurfaceMesh::Vertex_index lastCrossedVertex = SurfaceMesh::null_vertex();
    for(std::size_t step = 0; step < maxSteps; ++step) {
        if(covers(f)) {
            // Done: target is inside the triangle
            return {f, point_on_face(mesh, f, to)};
        }

        // 'to' lies outside f -> find the forward exit across f's boundary.
        // (1) Hits Vertex: the ray passes exactly through current triangle's vertex.
        //     `>0` means that the vertex is "in front" of us, not behind.
        //     With `>= 0` it also catches a start exactly on a vertex.
        bool hitsVertex = false;
        for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            if(v == lastCrossedVertex) {
                continue;
            }
            const auto vp = xy_of(mesh, v);
            if(CGAL::orientation(from, to, vp) == CGAL::COLLINEAR && (vp - from) * direction >= 0) {
                const auto g = fan_face(mesh, v, direction);
                if(g == SurfaceMesh::null_face()) {
                    throw SimulationError(
                        "straight_walk(): path leaves the walkable area at a vertex");
                }
                lastCrossedVertex = v;
                f = g;
                hitsVertex = true;
                break;
            }
        }
        if(hitsVertex) {
            continue;
        }

        // (2) Edge crossing: the forward exit edge has its source strictly RIGHT
        //     and its target strictly LEFT of the directed line from->to.
        SurfaceMesh::Halfedge_index exit = SurfaceMesh::null_halfedge();
        for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(f), mesh)) {
            const auto s = CGAL::orientation(from, to, xy_of(mesh, mesh.source(h)));
            const auto t = CGAL::orientation(from, to, xy_of(mesh, mesh.target(h)));
            if(s == CGAL::RIGHT_TURN && t == CGAL::LEFT_TURN) {
                exit = h;
                break;
            }
        }
        if(exit == SurfaceMesh::null_halfedge()) {
            // [RL] Purely defensive safeguard - should not be possible by mesh
            // construction/validation.
            throw SimulationError(
                "straight_walk(): no forward exit found (mesh/predicate inconsistency)");
        }
        const auto neighbor = mesh.face(mesh.opposite(exit));
        if(neighbor == SurfaceMesh::null_face()) {
            throw SimulationError("straight_walk(): path leaves the walkable area");
        }
        f = neighbor;
    }
    throw SimulationError("straight_walk(): did not converge (mesh/predicate inconsistency)");
}
} // namespace

void Location::move_on_surface(Point xy_direction)
{
    // Walk from current Location along provided xy-direction.
    const auto hit = straight_walk(
        _geometry->mesh(),
        _face,
        Point2D{_xy.x, _xy.y},
        K::Vector_2{xy_direction.x, xy_direction.y});
    // Though not strictly required, we only update if previous function did not throw
    // (e.g. by leaving the walkable area).
    _xy = _xy + xy_direction;
    _face = hit.face;
    _z = hit.point.z();
    _regionId = _geometry->region_of(hit.face);
}
