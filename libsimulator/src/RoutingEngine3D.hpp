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
    virtual ~RoutingEngine3D() = default;

    /// Load the walkable surface from a Wavefront OBJ file. The mesh is
    /// triangulated on load (OBJ faces may be arbitrary polygons).
    void set_geometry(SurfaceMesh&& mesh)
    {
        _mesh = std::move(mesh);
        _aabbTree = std::make_unique<AABBTree>(faces(_mesh).begin(), faces(_mesh).end(), _mesh);
    }

    /// Checks whether the provided location (3D-point or polygon)
    /// is on walkable surface taking wall clearance into account.
    /// @param loc location (Point or Polygon) to check
    /// @return path between from and to, includes from as
    ///         first element and to as last element
    virtual bool is_valid_location(const Location& loc) const;

    /// Compute shortest path on walkable surface.
    /// @param from where to route (Point)
    /// @param to route to (Location: Point or Polygon)
    /// @return tuple of (path, cost): the path includes from as first element
    ///         and to as last element; cost is typically geodesic distance along it,
    ///         but not necessarily (e.g. floor fields with slowness field).
    virtual std::tuple<std::vector<Point3D>, double>
    get_shortest_path(const Point3D& from, const Location& to) = 0;

    /// Get orientation to next point of shortest path on walkable surface projectted to x/y.
    /// @param from where to route
    /// @param to route to (Location: Point or Polygon)
    /// @return 2D orientation to next point of shortest path between from and to
    virtual Point get_orientation(const Point3D& from, const Location& to);

protected:
    /// Result of projecting a query point onto the surface.
    struct FaceLocation {
        SurfaceMesh::Face_index face;
        SurfaceKernel::Point_3 point;
    };

    /// Find face and point on face projecting via -z onto mesh.
    /// `SurfaceMesh::null_face()` in `face` if no such point is found.
    FaceLocation face_below(const Point3D& p) const;

    SurfaceMesh _mesh{};
    std::unique_ptr<AABBTree> _aabbTree{};
};
