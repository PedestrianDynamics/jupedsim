// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AABB.hpp"
#include "Graph.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "Triangle.hpp"

#include <vector>

#include <CGAL/Boolean_set_operations_2.h>

class RoutingEngine
{
public:
    RoutingEngine() = default;
    virtual ~RoutingEngine() = default;

    // TODO(kkratz): additional input parameters missing
    virtual Point ComputeWaypoint(Point currentPosition, Point destination) = 0;
    virtual std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination) = 0;

    /// Checks if supplied point is inside the routable space.
    /// @@param p Point to validate
    /// @return boolean IsRoutable?
    virtual bool IsRoutable(Point p) const = 0;

    // TODO(kkratz): input sources missing
    virtual void Update() = 0;

    virtual std::unique_ptr<RoutingEngine> Clone() const = 0;

protected:
    RoutingEngine(const RoutingEngine&) = default;
    RoutingEngine& operator=(const RoutingEngine&) = default;

    RoutingEngine(RoutingEngine&&) = default;
    RoutingEngine& operator=(RoutingEngine&&) = default;
};

struct VertexData {
    AABB aabb{};
    Triangle triangle{};
};

struct EdgeData {
    double weight;
    LineSegment edge;
};

class NavMeshRoutingEngine : public RoutingEngine
{
public:
    using GraphType = Graph<VertexData, EdgeData>;

private:
    GraphType _graph{};

public:
    explicit NavMeshRoutingEngine(GraphType&& graph);
    ~NavMeshRoutingEngine() override = default;

    NavMeshRoutingEngine(const NavMeshRoutingEngine& other) = default;
    NavMeshRoutingEngine& operator=(const NavMeshRoutingEngine& other) = default;

    NavMeshRoutingEngine(NavMeshRoutingEngine&& other) = default;
    NavMeshRoutingEngine& operator=(NavMeshRoutingEngine&& other) = default;

    std::unique_ptr<RoutingEngine> Clone() const override;
    Point ComputeWaypoint(Point currentPosition, Point destination) override;
    std::vector<Point> ComputeAllWaypoints(Point currentPosition, Point destination) override;
    bool IsRoutable(Point p) const override;
    void Update() override;

    /// This is designed for debugging purposes.
    /// @return a copy of all triangles that make up the accessible area.
    std::vector<Triangle> Mesh() const;

    /// This is designed for debugging purposes.
    /// @return vector of edgedata
    std::vector<EdgeData> EdgesFor(GraphType::VertexId id) const;

private:
    GraphType::VertexId findVertex(Point p) const;
};
