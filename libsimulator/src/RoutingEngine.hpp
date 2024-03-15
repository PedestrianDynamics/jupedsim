// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
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

class RoutingEngine : public Clonable<RoutingEngine>
{
public:
    RoutingEngine() = default;
    ~RoutingEngine() override = default;

    // TODO(kkratz): additional input parameters missing
    virtual Point ComputeWaypoint(Point currentPosition, Point destination) = 0;
    virtual std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination) = 0;

    /// Checks if supplied point is inside the routable space.
    /// @@param p Point to validate
    /// @return boolean IsRoutable?
    virtual bool IsRoutable(Point p) const = 0;

    // TODO(kkratz): input sources missing
    virtual void Update() = 0;

protected:
    RoutingEngine(const RoutingEngine&) = default;
    RoutingEngine& operator=(const RoutingEngine&) = default;

    RoutingEngine(RoutingEngine&&) = default;
    RoutingEngine& operator=(RoutingEngine&&) = default;
};

class NavMeshRoutingEngine : public RoutingEngine
{
    CDT cdt{};
    std::unique_ptr<Mesh> mesh{};

public:
    NavMeshRoutingEngine();
    explicit NavMeshRoutingEngine(const PolyWithHoles& poly);
    ~NavMeshRoutingEngine() override = default;

    NavMeshRoutingEngine(const NavMeshRoutingEngine& other) = delete;
    NavMeshRoutingEngine& operator=(const NavMeshRoutingEngine& other) = delete;

    NavMeshRoutingEngine(NavMeshRoutingEngine&& other) = default;
    NavMeshRoutingEngine& operator=(NavMeshRoutingEngine&& other) = default;

    std::unique_ptr<RoutingEngine> Clone() const override;
    Point ComputeWaypoint(Point currentPosition, Point destination) override;
    std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination) override;
    bool IsRoutable(Point p) const override;
    void Update() override;

    const Mesh* MeshData() const { return mesh.get(); };

private:
    CDT::Face_handle find_face(K::Point_2) const;
    std::vector<Point>
    straightenPath(Point from, Point to, const std::vector<CDT::Face_handle>& path);
};
