// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry.hpp"

#include "GeometricFunctions.hpp"
#include "Geometry/BoundaryIndex.hpp"
#include "LineSegment.hpp"
#include "Polygon.hpp"
#include "SimulationError.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <boost/iterator/function_output_iterator.hpp>
#include <boost/range/iterator_range.hpp>

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

Geometry::Geometry(SurfaceMesh mesh) : _mesh(std::move(mesh))
{
    // Compact vertex/face indices so vertices()/triangles()/region_id_per_face() are
    // contiguous.
    _mesh.collect_garbage();
    _regionSplit = split_into_regions(_mesh);
    build();
}

Geometry::Geometry(
    SurfaceMesh&& mesh,
    RegionSplit&& regionSplit,
    std::unique_ptr<RegionGraph2D> regionGraph2d)
    : _mesh(std::move(mesh))
    , _regionGraph2D(std::move(regionGraph2d))
    , _regionSplit(std::move(regionSplit))
{
    // safety only: hand-crafted mesh/region-split/regionGraph2d combo should not run into this
    assert(!_mesh.has_garbage());
    build();
}

void Geometry::build()
{
    _aabbTree = std::make_unique<AABBTree>(_mesh.faces().begin(), _mesh.faces().end(), _mesh);
    _region = _regionSplit.region;
    _boundaryIndex = MakePortalBoundaryIndex(_mesh, _regionSplit);
    _regionGraph = CreateRegionGraph(_mesh, _regionSplit);
}

PolyWithHoles Geometry::polygon(size_t region_id) const
{
    if(!_regionGraph2D) {
        throw SimulationError("Geometry is built from mesh and has no 2D polygons");
    }
    if(region_id >= region_count()) {
        throw SimulationError("Unknown region ID {}", region_id);
    }
    return (*_regionGraph2D)[region_id];
}

Geometry::FaceLocation Geometry::face_below(const Point3D& p) const
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

bool Geometry::is_valid_location(const Point3D& p) const
{
    return face_below(p).face != SurfaceMesh::null_face();
}

std::vector<LineSegment>
Geometry::line_segments_in_range(const Location& who, double distance) const
{
    return _boundaryIndex->Query(who, distance);
}

bool Geometry::no_geometry_between(const Location& who, Point direction) const
{
    return region_reached(who, direction).has_value();
}

bool Geometry::no_geometry_between(const Location& who, const Location& other) const
{
    const auto arrival = region_reached(who, other.xy() - who.xy());
    return arrival == other.region();
}

std::optional<std::size_t> Geometry::region_reached(const Location& who, Point direction) const
{
    const LineSegment chord{who.xy(), who.xy() + direction};
    const auto& graph = *_regionGraph;
    const auto crosses_seam = [&] {
        for(const auto e : boost::make_iterator_range(boost::out_edges(who.region(), graph))) {
            if(intersects(chord, graph[e])) {
                return true;
            }
        }
        return false;
    };
    if(!crosses_seam()) {
        // Direct line stays within this region: the region's own walls settle it.
        return graph[who.region()]->IntersectsAny(chord) ? std::nullopt :
                                                           std::optional{who.region()};
    }
    // Crosses regions: Doing the expensive "move on surface".
    const auto arrival = who.try_move_on_surface(direction);
    return arrival.has_value() ? std::optional{arrival->region()} : std::nullopt;
}

std::vector<Geometry::FaceLocation> Geometry::faces_at(const Point2D& xy) const
{
    const Line3D vertical(Point3D{xy.x(), xy.y(), 0}, Direction3D(0, 0, 1));
    std::vector<AABBTree::Intersection_and_primitive_id<Line3D>::Type> hits{};
    aabb_tree().all_intersections(vertical, std::back_inserter(hits));

    std::vector<FaceLocation> faces{};
    faces.reserve(hits.size());
    for(const auto& [where, face] : hits) {
        const auto* point = std::get_if<Point3D>(&where);
        // Assert against vertical faces - purely defensive.
        assert(point && "FATAL: vertical face hit by the locate line");
        faces.push_back({face, *point});
    }
    // The tree's traversal order is not specified; make it deterministic.
    std::sort(faces.begin(), faces.end(), [this](const auto& a, const auto& b) {
        return std::pair{region_of(a.face), a.face} < std::pair{region_of(b.face), b.face};
    });
    return faces;
}

Location Geometry::location_at(Point xy, const FaceLocation& where) const
{
    return Location{this, xy, region_of(where.face), where.face, where.point.z()};
}

Geometry::FaceLocation Geometry::locate_in_region(std::size_t region_id, const Point2D& xy) const
{
    for(const auto& where : faces_at(xy)) {
        if(region_of(where.face) == region_id) {
            return where;
        }
    }
    return {SurfaceMesh::null_face(), Point3D{}};
}

Location Geometry::get_location(double x, double y, std::size_t region_id) const
{
    if(region_id >= region_count()) {
        throw SimulationError("Unknown region ID {}", region_id);
    }
    const auto face_location = locate_in_region(region_id, {x, y});
    if(face_location.face == SurfaceMesh::null_face()) {
        throw SimulationError("Point {} is not in region {}", Point{x, y}, region_id);
    }
    return location_at(Point{x, y}, face_location);
}

std::optional<Location>
Geometry::get_location_near_z(double x, double y, double z, double tol) const
{
    const auto where = locate_near_z(Point2D{x, y}, z, tol);
    if(where.face == SurfaceMesh::null_face()) {
        return std::nullopt;
    }
    return location_at(Point{x, y}, where);
}

Geometry::FaceLocation Geometry::locate_near_z(const Point2D& xy, double z, double tolerance) const
{
    FaceLocation best{SurfaceMesh::null_face(), Point3D{}};
    auto bestDeviation = tolerance;
    for(const auto& face_location : faces_at(xy)) {
        const auto deviation = std::abs(face_location.point.z() - z);
        if(deviation < bestDeviation) {
            bestDeviation = deviation;
            best = face_location;
        }
    }
    return best;
}

std::vector<std::size_t> Geometry::region_id_per_face() const
{
    std::vector<std::size_t> ids{};
    ids.reserve(_mesh.number_of_faces());
    for(const auto f : _mesh.faces()) {
        ids.push_back(_region[f]);
    }
    return ids;
}

std::vector<std::array<double, 3>> Geometry::vertices() const
{
    std::vector<std::array<double, 3>> out{};
    out.reserve(_mesh.number_of_vertices());
    for(const auto v : _mesh.vertices()) {
        const auto& p = _mesh.point(v);
        out.push_back({p.x(), p.y(), p.z()});
    }
    return out;
}

std::vector<std::array<std::size_t, 3>> Geometry::triangles() const
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

namespace
{
/// The seam @p e as a segment of its source region's footprint.
Segment2D seam_segment(const Geometry::RegionGraph2D& g, Geometry::RegionGraph2D::edge_descriptor e)
{
    const auto& footprint = g[boost::source(e, g)];
    const auto& seam = g[e];
    const Poly& ring =
        seam.ring == 0 ? footprint.outer_boundary() : footprint.holes()[seam.ring - 1];
    return ring.edge(seam.index);
}

/// Whether @p s lies inside @p area or crosses or touches its boundary.
bool meets(const Poly& area, const Segment2D& s)
{
    if(area.bounded_side(s.source()) != CGAL::ON_UNBOUNDED_SIDE) {
        return true;
    }
    return std::any_of(area.edges_begin(), area.edges_end(), [&s](const Segment2D& edge) {
        return CGAL::do_intersect(edge, s);
    });
}

/// Exact arithmetic for cutting areas. Boolean set operations construct the points where
/// edges cross and run predicates on them again; with inexact constructions the two can
/// disagree on (nearly) degenerate input the result is a crash or a wrong cut, not a small error.
/// With EPECK the one rounding step is the conversion of the finished pieces back to doubles.
using ExactKernel = CGAL::Exact_predicates_exact_constructions_kernel;
using ExactPoly = CGAL::Polygon_2<ExactKernel>;
using ExactPolyWithHoles = CGAL::Polygon_with_holes_2<ExactKernel>;

ExactPoly to_exact(const Poly& ring)
{
    const CGAL::Cartesian_converter<K, ExactKernel> convert{};
    ExactPoly exact{};
    for(auto v = ring.vertices_begin(); v != ring.vertices_end(); ++v) {
        exact.push_back(convert(*v));
    }
    return exact;
}

ExactPolyWithHoles to_exact(const PolyWithHoles& area)
{
    std::vector<ExactPoly> holes{};
    holes.reserve(area.number_of_holes());
    for(const auto& hole : area.holes()) {
        holes.push_back(to_exact(hole));
    }
    return ExactPolyWithHoles(to_exact(area.outer_boundary()), holes.begin(), holes.end());
}

/// Rounds @p ring to doubles, dropping vertices that round onto their predecessor.
Poly to_inexact(const ExactPoly& ring)
{
    const CGAL::Cartesian_converter<ExactKernel, K> convert{};
    Poly inexact{};
    for(auto v = ring.vertices_begin(); v != ring.vertices_end(); ++v) {
        const auto p = convert(*v);
        if(inexact.is_empty() || p != inexact.vertex(inexact.size() - 1)) {
            inexact.push_back(p);
        }
    }
    if(inexact.size() > 1 && inexact.vertex(0) == inexact.vertex(inexact.size() - 1)) {
        inexact.erase(std::prev(inexact.vertices_end()));
    }
    return inexact;
}

} // namespace

/// Split @p p into pieces that lie in a single region each. The pieces are clipped to the
/// region's footprint. A vector of AreaPiece is returned, each with the piece's polygon and the
/// region id it lies in.
std::vector<AreaPiece> Geometry::split_into_region_pieces(const Poly& p, size_t region_id) const
{

    const auto* g = _regionGraph2D.get();
    if(!g) {
        throw SimulationError("Geometry is built from mesh and has no 2D polygons");
    }
    // convert the polygon to exact arithmetic for cutting, and intersection test
    const auto seed_poly = to_exact((*g)[region_id]);
    const auto exact_area = to_exact(p);
    if(!CGAL::do_intersect(seed_poly, exact_area)) {
        throw SimulationError("Area does not intersect region {}", region_id);
    }
    std::vector<bool> seen(boost::num_vertices(*g), false);

    std::vector<AreaPiece> pieces{};

    // define lambda function to add pieces of a region to the pieces vector
    const auto add_pieces_of = [&pieces, &exact_area, &g, &seen](std::size_t r) {
        // CGAL::intersection returns a range of polygons with holes, however for the routing
        // we only need the outer boundary of each piece, since agents never walk on holes.
        seen[r] = true;
        CGAL::intersection(
            to_exact((*g)[r]),
            exact_area,
            boost::make_function_output_iterator([&pieces, r](const ExactPolyWithHoles& piece) {
                pieces.push_back(AreaPiece{to_inexact(piece.outer_boundary()), r});
            }));
    };

    // we start a graph traversal from the seed region, and add pieces of each region
    // where a seam segment intersects the polygon p.
    // We keep track of seen regions to avoid cycles.
    std::set<std::size_t> todo;
    todo.insert(region_id);
    while(!todo.empty()) {
        const auto r = *todo.begin();
        todo.erase(todo.begin());
        add_pieces_of(r);
        for(auto e : boost::make_iterator_range(boost::out_edges(r, *g))) {
            const auto n = boost::target(e, *g);
            if(!seen[n] && meets(p, seam_segment(*g, e))) {
                todo.insert(n);
            }
        }
    }
    return pieces;
}
