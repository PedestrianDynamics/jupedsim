// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Mesh.hpp"
#include "Point.hpp"
#include "RoutingEngine.hpp"

#include <memory>
#include <string>
#include <vector>

class AStarRoutingEngine : public RoutingEngine
{
    CDT cdt{};
    std::unique_ptr<Mesh> mesh{};

public:
    AStarRoutingEngine() = default;
    ~AStarRoutingEngine() override = default;

    AStarRoutingEngine(const AStarRoutingEngine& other) = delete;
    AStarRoutingEngine& operator=(const AStarRoutingEngine& other) = delete;

    AStarRoutingEngine(AStarRoutingEngine&& other) = default;
    AStarRoutingEngine& operator=(AStarRoutingEngine&& other) = default;

    std::string name() const override { return "AStar"; }
    void set_geometry(const CollisionGeometry& geometry) override;
    std::vector<Point> compute_waypoints(Point from, Point destination) override;
    bool is_routable(Point p) const override;

    const Mesh* MeshData() const { return mesh.get(); }

private:
    CDT::Face_handle find_face(K::Point_2) const;
    std::vector<Point>
    straightenPath(Point from, Point to, const std::vector<CDT::Face_handle>& path);
};
