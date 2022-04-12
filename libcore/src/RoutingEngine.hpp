#pragma once

#include "AABB.hpp"
#include "CGALPolygon.hpp"
#include "Destination.hpp"
#include "Graph.hpp"
#include "Triangle.hpp"
#include "geometry/Building.hpp"
#include "geometry/Point.hpp"

#include <poly2tri/common/shapes.h>

class RoutingEngine
{
public:
    RoutingEngine() = default;
    virtual ~RoutingEngine() = default;

    // TODO(kkratz): additional input parameters missing
    virtual std::vector<Point> ComputeWaypoint(Point currentPosition, Point destination) = 0;

    // TODO(kkratz): input sources missing
    virtual void Update() = 0;

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
    Line edge;
};

class NavMeshRoutingEngine : public RoutingEngine
{
private:
    using GraphType = Graph<VertexData, EdgeData>;
    GraphType _graph{};

public:
    static NavMeshRoutingEngine MakeFromBuilding(const Building& building);

    explicit NavMeshRoutingEngine(GraphType&& graph);
    ~NavMeshRoutingEngine() override = default;

    NavMeshRoutingEngine(const NavMeshRoutingEngine& other) = delete;
    NavMeshRoutingEngine& operator=(const NavMeshRoutingEngine& other) = delete;

    NavMeshRoutingEngine(NavMeshRoutingEngine&& other) = default;
    NavMeshRoutingEngine& operator=(NavMeshRoutingEngine&& other) = default;

    std::vector<Point> ComputeWaypoint(Point currentPosition, Point destination) override;
    void Update() override;

private:
    GraphType::VertexId findVertex(Point p) const;
};
