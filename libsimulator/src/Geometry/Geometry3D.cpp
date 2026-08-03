// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry3D.hpp"

#include "Geometry/RegionReach.hpp"
#include "Geometry/RegionSeams.hpp"
#include "Geometry/WallMerge.hpp"
#include "LineSegment.hpp"
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
    // Walls are fused across the whole mesh, so a wall running past a region boundary stays
    // one wall - and is then held by both regions it borders, to be findable from either.
    const auto walls = merge_border_walls(_mesh, _region, wall_merge_tolerance(_mesh));
    std::vector<std::vector<MergedWall>> walls_by_region(_regionCount);
    for(const auto& wall : walls) {
        for(const auto r : wall.regions) {
            walls_by_region[r].push_back(wall);
        }
    }

    auto seams_by_region =
        group_seams_by_region(extract_region_seams(_mesh, _region), _regionCount);

    _regionViews.clear();
    _regionViews.reserve(_regionCount);
    for(std::size_t r = 0; r < _regionCount; ++r) {
        _regionViews.emplace_back(
            r, this, std::move(walls_by_region[r]), std::move(seams_by_region[r]));
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

WallRange Geometry3D::line_segments_in_range(const Location& who, double distance) const
{
    if(_geometry2D != nullptr) {
        return WallRange{
            distance < 0.0 ? _geometry2D->LineSegmentsInApproxDistanceTo(who.xy()) :
                             _geometry2D->LineSegmentsInDistanceTo(distance, who.xy())};
    }

    // Without an explicit distance the grid answers by cell, so the reach that decides which
    // regions to consult is the one the cells were built for.
    const double reach = distance >= 0.0 ? distance : ApproximateWallReach;

    WallRange::Spans spans{};
    for(const auto& visit : regions_within_reach(
            [this](std::size_t r) -> const std::vector<RegionSeam>& {
                return region_view(r).seams();
            },
            who.region(),
            who.xy(),
            reach)) {
        const auto& view = region_view(visit.region);
        const auto walls = distance < 0.0 ? view.LineSegmentsInApproxDistanceTo(visit.from) :
                                            view.LineSegmentsInDistanceTo(visit.radius, visit.from);
        if(!walls.empty()) {
            spans.push_back(walls);
        }
    }
    return WallRange{std::move(spans)};
}

bool Geometry3D::no_geometry_between(const Location& who, Point direction) const
{
    return region_reached(who, direction).has_value();
}

bool Geometry3D::no_geometry_between(const Location& who, const Location& other) const
{
    // Rebuilding the direction rather than handing over other.xy() keeps the chord exactly
    // what the direction-taking callers have always passed.
    const auto arrival = region_reached(who, other.xy() - who.xy());
    return arrival == other.region();
}

std::optional<std::size_t> Geometry3D::region_reached(const Location& who, Point direction) const
{
    const LineSegment chord{who.xy(), who.xy() + direction};
    if(_geometry2D != nullptr) {
        return _geometry2D->IntersectsAny(chord) ? std::nullopt : std::optional{who.region()};
    }

    const auto& view = region_view(who.region());
    if(!view.crosses_seam(chord.p1, chord.p2)) {
        // Staying inside the region means never reaching another sheet, so only the walls of
        // this one can be in the way, and this is where the step ends up.
        return view.IntersectsAny(chord) ? std::nullopt : std::optional{who.region()};
    }
    // The chord leaves the region, so the flat test would be answering about the wrong
    // sheet. Walking it is the only thing that stays on the surface, and where the walk comes
    // out says which sheet it followed.
    const auto arrival = who.try_move_on_surface(direction);
    return arrival.has_value() ? std::optional{arrival->region()} : std::nullopt;
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
