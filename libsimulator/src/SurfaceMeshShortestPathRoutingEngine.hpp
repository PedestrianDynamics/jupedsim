// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine3D.hpp"

#include <CGAL/Surface_mesh_shortest_path.h>

#include <tuple>
#include <vector>

class SurfaceMeshShortestPathRoutingEngine : public RoutingEngine3D
{
public:
    /// @param mesh surface mesh defining the walkable geometry
    explicit SurfaceMeshShortestPathRoutingEngine(SurfaceMesh&& mesh);
    ~SurfaceMeshShortestPathRoutingEngine() override = default;

    bool IsValidLocation(const Location& loc) const override;

    std::tuple<std::vector<Point3D>, double>
    GetShortestPath(const Point3D& source, const Location& target) override;

    Point GetOrientation(const Point3D& source, const Location& target) override;

private:
    using Traits = CGAL::Surface_mesh_shortest_path_traits<SurfaceKernel, SurfaceMesh>;
    using ShortestPath = CGAL::Surface_mesh_shortest_path<Traits>;

    /// Result of projecting a query point onto the surface.
    struct FaceLocation {
        SurfaceMesh::Face_index face;
        SurfaceKernel::Point_3 point;
    };

    /// Find face and point on face projecting via -z onto mesh.
    /// `SurfaceMesh::null_face()` in `face` if no such point is found.
    FaceLocation face_below(const Point3D& p) const;

    SurfaceMesh _mesh;
    AABBTree _aabbTree;
};
