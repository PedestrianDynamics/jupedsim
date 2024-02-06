// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine.hpp"

#include "GeometricFunctions.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "SimulationError.hpp"
#include "Triangle.hpp"

#include <vector>

NavMeshRoutingEngine::NavMeshRoutingEngine(GraphType&& graph) : _graph(graph)
{
}

std::unique_ptr<RoutingEngine> NavMeshRoutingEngine::Clone() const
{
    return std::make_unique<NavMeshRoutingEngine>(*this);
}

Point NavMeshRoutingEngine::ComputeWaypoint(Point currentPosition, Point destination)
{
    GraphType::VertexId from = findVertex(currentPosition);
    GraphType::VertexId to = findVertex(destination);
    if(from == to) {
        return destination;
    }

    const auto path = _graph.Path(from, to);
    assert(!path.empty());

    const size_t portalCount = path.size();

    // This is the actual simple stupid funnel algorithm
    auto apex = currentPosition;
    auto portal_left = currentPosition;
    auto portal_right = currentPosition;

    for(size_t index_portal = 1; index_portal <= portalCount; ++index_portal) {
        // TODO(kkratz): Edge lookup is O(n) where n is the number of edges in the graph
        // This needs to be replaced with a graph that allows for faster edge lookup
        const auto portal = index_portal < portalCount ?
                                _graph.Edge(path[index_portal - 1], path[index_portal]).edge :
                                LineSegment(destination, destination);
        const auto& candidate_left = portal.p2;
        const auto& candidate_right = portal.p1;

        if(triarea2d(apex, portal_right, candidate_right) <= 0.0) {
            if(apex == portal_right || triarea2d(apex, portal_left, candidate_right) > 0.0) {
                portal_right = candidate_right;
            } else {
                return portal_left;
            }
        }
        if(triarea2d(apex, portal_left, candidate_left) >= 0.0) {
            if(apex == portal_left || triarea2d(apex, portal_right, candidate_left) < 0.0) {
                portal_left = candidate_left;
            } else {
                return portal_right;
            }
        }
    }
    return destination;
}

std::vector<Point>
NavMeshRoutingEngine::ComputeAllWaypoints(Point currentPosition, Point destination)
{
    GraphType::VertexId from = findVertex(currentPosition);
    GraphType::VertexId to = findVertex(destination);
    if(from == to) {
        return std::vector<Point>{currentPosition, destination};
    }

    std::vector<Point> waypoints{currentPosition};
    const auto path = _graph.Path(from, to);
    assert(!path.empty());

    const size_t portalCount = path.size();

    // This is the actual simple stupid funnel algorithm
    auto apex = currentPosition;
    auto portal_left = currentPosition;
    auto portal_right = currentPosition;

    size_t index_apex{0};
    size_t index_left{0};
    size_t index_right{0};

    for(size_t index_portal = 1; index_portal <= portalCount; ++index_portal) {
        const auto portal = index_portal < portalCount ?
                                _graph.Edge(path[index_portal - 1], path[index_portal]).edge :
                                LineSegment(destination, destination);
        const auto line_segment_left = portal.p2;
        const auto line_segment_right = portal.p1;
        const auto line_segment_direction = (line_segment_right - line_segment_left).Normalized();
        const auto candidate_left = line_segment_left + (line_segment_direction * 0.2);
        const auto candidate_right = line_segment_right - (line_segment_direction * 0.2);

        if(triarea2d(apex, portal_right, candidate_right) <= 0.0) {
            if(apex == portal_right || triarea2d(apex, portal_left, candidate_right) > 0.0) {
                portal_right = candidate_right;
                index_right = index_portal;
            } else {
                waypoints.emplace_back(portal_left);
                apex = portal_left;
                index_apex = index_left;
                portal_left = apex;
                portal_right = apex;
                index_left = index_apex;
                index_right = index_apex;
                index_portal = index_apex;
                continue;
            }
        }
        if(triarea2d(apex, portal_left, candidate_left) >= 0.0) {
            if(apex == portal_left || triarea2d(apex, portal_right, candidate_left) < 0.0) {
                portal_left = candidate_left;
                index_left = index_portal;
            } else {
                waypoints.emplace_back(portal_right);
                apex = portal_right;
                index_apex = index_right;
                portal_left = apex;
                portal_right = apex;
                index_left = index_apex;
                index_right = index_apex;
                index_portal = index_apex;
                continue;
            }
        }
    }
    waypoints.emplace_back(destination);
    return waypoints;
}

bool NavMeshRoutingEngine::IsRoutable(Point p) const
{
    for(const auto& id : _graph.Vertices()) {
        const auto& v = _graph.Vertex(id);
        if(v.aabb.Inside(p) && v.triangle.Inside(p)) {
            return true;
        }
    }
    return false;
}

void NavMeshRoutingEngine::Update()
{
}

std::vector<Triangle> NavMeshRoutingEngine::Mesh() const
{
    std::vector<Triangle> mesh{};
    for(const auto& id : _graph.Vertices()) {
        const auto& v = _graph.Vertex(id);
        mesh.emplace_back(v.triangle);
    }
    return mesh;
}

std::vector<EdgeData> NavMeshRoutingEngine::EdgesFor(GraphType::VertexId id) const
{
    return _graph.EdgesFor(id);
}

NavMeshRoutingEngine::GraphType::VertexId NavMeshRoutingEngine::findVertex(Point p) const
{
    for(const auto& id : _graph.Vertices()) {
        const auto& v = _graph.Vertex(id);
        if(v.aabb.Inside(p) && v.triangle.Inside(p)) {
            return id;
        }
    }
    throw SimulationError("Point {} outside accessible area", p);
}
