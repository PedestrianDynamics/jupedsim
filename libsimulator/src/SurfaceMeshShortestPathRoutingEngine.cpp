// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SurfaceMeshShortestPathRoutingEngine.hpp"

#include "SimulationError.hpp"

#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// SurfaceMeshShortestPathRoutingEngine
////////////////////////////////////////////////////////////////////////////////
SurfaceMeshShortestPathRoutingEngine::SurfaceMeshShortestPathRoutingEngine(
    const Geometry& geometry,
    double wallClearance)
    : RoutingEngine(wallClearance), _geometry(geometry)
{
}

bool SurfaceMeshShortestPathRoutingEngine::IsValidLocation(const RoutingTarget& loc) const
{
    return _geometry.face_below(loc).face != SurfaceMesh::null_face();
}

Geometry::FaceLocation
SurfaceMeshShortestPathRoutingEngine::on_surface(const Point3D& p, const char* what) const
{
    const auto below = _geometry.face_below(p);
    if(below.face == SurfaceMesh::null_face()) {
        throw SimulationError(
            "GetShortestPath(): {} does not project onto the walkable surface.", what);
    }
    return below;
}

SurfaceMeshShortestPathRoutingEngine::ShortestPath&
SurfaceMeshShortestPathRoutingEngine::tree_for(const RoutingTarget& target)
{
    auto it = _cache.find(target);
    if(it == _cache.end()) {
        const auto below = on_surface(target, "target");
        auto shortest_path = std::make_unique<ShortestPath>(_geometry.mesh());
        const auto to_loc = shortest_path->locate(below.point, _geometry.aabb_tree());
        shortest_path->add_source_point(to_loc);
        shortest_path->build_sequence_tree();
        it = _cache.emplace(target, std::move(shortest_path)).first;
    }
    return *it->second;
}

SurfaceMeshShortestPathRoutingEngine::Way
SurfaceMeshShortestPathRoutingEngine::trace_way(const Point3D& source, const RoutingTarget& target)
{
    const auto from_below = on_surface(source, "source");
    auto& tree = tree_for(target);
    const auto from_loc = tree.locate(from_below.point, _geometry.aabb_tree());

    // The very points `shortest_path_points_to_source_points` gives -- that call is this walk
    // with the kind of simplex thrown away. Keeping it is what tells a corner from a crossing.
    struct Collector {
        ShortestPath& tree;
        const SurfaceMesh& mesh;
        Way& way;

        void add(const Point3D& p, Point open)
        {
            way.points.push_back(p);
            way.intoTheOpen.push_back(open);
        }

        /// Where the open lies at a corner: opposite the two walls that meet there.
        ///
        /// Both walls end at the corner, so a point on their bisector is that far from either.
        /// Which of the two directions is the open one needs no test: a geodesic only turns
        /// around a corner the walkable area bends away from, so the free side is always the
        /// wider one. Judged in plan, like everything a model is told.
        Point out_of_the_corner(SurfaceMesh::Vertex_index v) const
        {
            const auto border = CGAL::is_border(v, mesh);
            if(!border) {
                // An interior vertex: the way bends over a fold, there is no wall to avoid.
                return {0.0, 0.0};
            }
            const auto xy = [this](SurfaceMesh::Vertex_index w) {
                const auto& p = mesh.point(w);
                return Point{p.x(), p.y()};
            };
            const Point here = xy(v);
            const Point along = (xy(mesh.source(*border)) - here).Normalized();
            const Point onwards = (xy(mesh.target(mesh.next(*border))) - here).Normalized();
            // A straight wall running through has no corner to step around.
            return (along + onwards).Normalized() * -1.0;
        }

        void operator()(SurfaceMesh::Halfedge_index e, K::FT t)
        {
            add(tree.point(e, t), Point{0.0, 0.0});
        }
        void operator()(SurfaceMesh::Vertex_index v) { add(tree.point(v), out_of_the_corner(v)); }
        void operator()(SurfaceMesh::Face_index f, const ShortestPath::Barycentric_coordinates& bc)
        {
            add(tree.point(f, bc), Point{0.0, 0.0});
        }
    };

    Way way{};
    Collector collector{tree, _geometry.mesh(), way};
    tree.shortest_path_sequence_to_source_points(from_loc.first, from_loc.second, collector);
    return way;
}

Point3D SurfaceMeshShortestPathRoutingEngine::held_off_the_wall(
    const Point3D& corner,
    Point into_the_open) const
{
    const Point moved = Point{corner.x(), corner.y()} + into_the_open * WallClearance();
    // Back onto the surface: beside a corner the floor may climb, and the route has to stay on
    // the storey the corner belongs to. Half a metre is far more than the clearance can climb
    // and far less than one storey.
    constexpr double sameStorey = 0.5;
    const auto located = _geometry.get_location(moved.x, moved.y, corner.z(), sameStorey);
    return located ? located->position_3d() : Point3D{moved.x, moved.y, corner.z()};
}

std::vector<Point3D> SurfaceMeshShortestPathRoutingEngine::GetShortestPath(
    const Point3D& source,
    const RoutingTarget& target)
{
    const auto way = trace_way(source, target);

    std::vector<Point3D> path{};
    path.reserve(way.points.size());
    for(std::size_t i = 0; i < way.points.size(); ++i) {
        path.push_back(
            way.intoTheOpen[i].isZeroLength() ?
                way.points[i] :
                held_off_the_wall(way.points[i], way.intoTheOpen[i]));
    }
    return path;
}

Point SurfaceMeshShortestPathRoutingEngine::next_waypoint(
    const Point3D& source,
    const RoutingTarget& target)
{
    const Point here{source.x(), source.y()};
    // CGAL sets a point wherever the way crosses a triangle edge.
    for(const auto& p : GetShortestPath(source, target)) {
        const Point xy{p.x(), p.y()};
        if(!(xy - here).isZeroLength()) {
            // Return first point "far enough" from source
            return xy;
        }
    }
    return here;
}

Point SurfaceMeshShortestPathRoutingEngine::ComputeWaypoint(
    const Location& from,
    const Location& to)
{
    const Point next = next_waypoint(from.position_3d(), to.position_3d());
    return next == from.xy() ? to.xy() : next;
}

Point SurfaceMeshShortestPathRoutingEngine::GetOrientation(
    const Point3D& source,
    const RoutingTarget& target)
{
    const Point here{source.x(), source.y()};
    // Zero when the way heads for where it already is: nowhere left to go.
    return (next_waypoint(source, target) - here).Normalized();
}
