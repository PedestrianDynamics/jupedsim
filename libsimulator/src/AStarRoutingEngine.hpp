// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Clonable.hpp"
#include "Mesh.hpp"
#include "Point.hpp"

#include <cstddef>
#include <memory>
#include <variant>
#include <vector>

using LocationID = size_t;
using Location = std::variant<Point, LocationID>;

class AStarRoutingEngine : public Clonable<AStarRoutingEngine>
{
    CDT cdt{};
    std::unique_ptr<Mesh> mesh{};

public:
    AStarRoutingEngine();
    explicit AStarRoutingEngine(const PolyWithHoles& poly);
    ~AStarRoutingEngine() override = default;

    AStarRoutingEngine(const AStarRoutingEngine& other) = delete;
    AStarRoutingEngine& operator=(const AStarRoutingEngine& other) = delete;

    AStarRoutingEngine(AStarRoutingEngine&& other) = default;
    AStarRoutingEngine& operator=(AStarRoutingEngine&& other) = default;

    std::unique_ptr<AStarRoutingEngine> Clone() const override;
    Point ComputeWaypoint(Point currentPosition, Point destination);
    std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination);
    bool IsRoutable(Point p) const;
    void Update();

    const Mesh* MeshData() const { return mesh.get(); };

private:
    CDT::Face_handle find_face(K::Point_2) const;
    std::vector<Point>
    straightenPath(Point from, Point to, const std::vector<CDT::Face_handle>& path);
};
