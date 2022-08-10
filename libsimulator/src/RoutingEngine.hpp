#pragma once

#include "AABB.hpp"
#include "CGALPolygon.hpp"
#include "Graph.hpp"
#include "Line.hpp"
#include "Point.hpp"
#include "Triangle.hpp"

class RoutingEngine
{
public:
    RoutingEngine() = default;
    virtual ~RoutingEngine() = default;

    // TODO(kkratz): additional input parameters missing
    virtual std::vector<Point> ComputeWaypoint(Point currentPosition, Point destination) = 0;

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
    Line edge;
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
    std::vector<Point> ComputeWaypoint(Point currentPosition, Point destination) override;
    void Update() override;

private:
    GraphType::VertexId findVertex(Point p) const;
};
