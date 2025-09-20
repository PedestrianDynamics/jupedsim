// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AABB.hpp"
#include "CfgCgal.hpp"
#include "Clonable.hpp"
#include "Graph.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Mesh.hpp"
#include "Point.hpp"

#include <memory>
#include <vector>

using LocationID = size_t;
using Location = std::variant<Point, LocationID>;

class RoutingEngine : public Clonable<RoutingEngine>
{
    CDT cdt{};
    std::unique_ptr<Mesh> mesh{};

public:
    RoutingEngine();
    explicit RoutingEngine(const PolyWithHoles& poly);
    ~RoutingEngine() override = default;

    RoutingEngine(const RoutingEngine& other) = delete;
    RoutingEngine& operator=(const RoutingEngine& other) = delete;

    RoutingEngine(RoutingEngine&& other) = default;
    RoutingEngine& operator=(RoutingEngine&& other) = default;

    std::unique_ptr<RoutingEngine> Clone() const override;
    Point ComputeWaypoint(Point currentPosition, Point destination, float path_bias = 0.0f);
  std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination, float path_bias);
    bool IsRoutable(Point p) const;
    void Update();

    const Mesh* MeshData() const { return mesh.get(); };

private:
    CDT::Face_handle find_face(K::Point_2) const;
    std::vector<Point>
    straightenPath(Point from, Point to, const std::vector<CDT::Face_handle>& path, float path_bias = 0.0f);
};
