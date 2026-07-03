// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Point.hpp"

#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/AABB_traits_3.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

#include <memory>
#include <string_view>
#include <tuple>
#include <vector>

// Surface_mesh_shortest_path is designed for EPICK
using SurfaceKernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using SurfaceMesh = CGAL::Surface_mesh<SurfaceKernel::Point_3>;
using Point3D = SurfaceKernel::Point_3;
using Location = Point3D; // [RL] TODO: Support more than just points

using AABBPrimitive = CGAL::AABB_face_graph_triangle_primitive<SurfaceMesh>;
using AABBTraits = CGAL::AABB_traits_3<SurfaceKernel, AABBPrimitive>;
using AABBTree = CGAL::AABB_tree<AABBTraits>;

/// Base for routing engines.
class RoutingEngine3D
{
public:
    RoutingEngine3D() = default;
    virtual ~RoutingEngine3D() = default;

    // Non-copyable and non-movable
    RoutingEngine3D(const RoutingEngine3D&) = delete;
    RoutingEngine3D& operator=(const RoutingEngine3D&) = delete;
    RoutingEngine3D(RoutingEngine3D&&) = delete;
    RoutingEngine3D& operator=(RoutingEngine3D&&) = delete;

    /// Set geometry.
    /// @param mesh surface mesh to set geometry to
    void set_geometry(SurfaceMesh&& mesh);

    /// Checks whether the provided location (3D-point or polygon)
    /// is on walkable surface taking wall clearance into account.
    /// @param loc location (Point or Polygon) to check
    /// @return true if the location projects onto the walkable surface
    virtual bool is_valid_location(const Location& loc) const;

    /// Set the routing target and run any per-target precomputation. This allows e.g.
    // to  run a floor field pre-computation and re-use this in get_shortest_path()
    /// @param target the destination all subsequent get_shortest_path() route to
    virtual void set_target(const Location& target) = 0;

    /// Compute the shortest path from @p source to the target set via set_target().
    /// @param source where to route from
    /// @return tuple of (path, cost): the path includes source as first element
    ///         and the target as last element; cost is typically geodesic distance
    ///         along it, but not necessarily (e.g. floor fields with slowness field).
    virtual std::tuple<std::vector<Point3D>, double> get_shortest_path(const Point3D& source) = 0;

    /// Get orientation to next point of the shortest path from @p source to the
    /// current target, projected to x/y.
    /// @param source where to route from
    /// @return 2D orientation to the next waypoint
    virtual Point get_orientation(const Point3D& source);

protected:
    /// Result of projecting a query point onto the surface.
    struct FaceLocation {
        SurfaceMesh::Face_index face;
        SurfaceKernel::Point_3 point;
    };

    /// Find face and point on face projecting via -z onto mesh.
    /// `SurfaceMesh::null_face()` in `face` if no such point is found.
    FaceLocation face_below(const Point3D& p) const;

    /// Drop any per-target precomputation, e.g. when the geometry changes.
    /// Engines with target-related cache have to override it, engines like
    /// polyanya/TA* will not as they typically do not run any pre-computation.
    virtual void invalidate_target() {}

    SurfaceMesh _mesh{};
    std::unique_ptr<AABBTree> _aabbTree{};
};
