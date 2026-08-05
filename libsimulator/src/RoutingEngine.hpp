// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Mesh.hpp"
#include "Point.hpp"
#include "RoutingEngine3D.hpp"

#include <cstddef>
#include <memory>
#include <tuple>
#include <vector>

/// 2D TA* + funnel routing engine wrapped to 3D routing engine interface.
class RoutingEngine : public RoutingEngine3D
{
    CDT cdt{};
    std::unique_ptr<Mesh> mesh{};

public:
    explicit RoutingEngine(const PolyWithHoles& poly, double wallClearance = 0.2);
    ~RoutingEngine() override = default;

    // Nte: Copy and move are both deleted by the base class RoutingEngine3D.

    Point ComputeWaypoint(Point currentPosition, Point destination);
    std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination);
    bool IsRoutable(Point p) const;
    void Update();

    // RoutingEngine3D interface
    Point ComputeWaypoint(const Location& from, const Location& to) override;
    bool IsValidLocation(const RoutingTarget& loc) const override;
    std::vector<Point3D>
    GetShortestPath(const Point3D& source, const RoutingTarget& target) override;
    Point GetOrientation(const Point3D& source, const RoutingTarget& target) override;

    const Mesh* MeshData() const { return mesh.get(); };

private:
    CDT::Face_handle find_face(K::Point_2) const;
    std::vector<Point>
    straightenPath(Point from, Point to, const std::vector<CDT::Face_handle>& path);
};
