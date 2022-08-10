#include "RoutingEngine.hpp"

#include "GeometricFunctions.hpp"
#include "Line.hpp"

#include <vector>

NavMeshRoutingEngine::NavMeshRoutingEngine(GraphType&& graph) : _graph(graph)
{
}

std::unique_ptr<RoutingEngine> NavMeshRoutingEngine::Clone() const
{
    return std::make_unique<NavMeshRoutingEngine>(*this);
}

std::vector<Point> NavMeshRoutingEngine::ComputeWaypoint(Point currentPosition, Point destination)
{
    GraphType::VertexId from = findVertex(currentPosition);
    GraphType::VertexId to = findVertex(destination);
    if(from == to) {
        return std::vector<Point>{currentPosition, destination};
    }

    std::vector<Point> waypoints{currentPosition};
    const auto path = _graph.Path(from, to);
    assert(!path.empty());

    // TODO(kkratz): Edge lookup is O(n) where n is the number of edges in the graph
    // This needs to be replaced with a graph that allows for faster edge lookup
    std::vector<Line> portals{};
    portals.reserve(path.size() - 1);
    for(size_t index = 1; index < path.size(); ++index) {
        portals.push_back(_graph.Edge(path[index - 1], path[index]).edge);
    }

    // This is the actual simple stupid funnel algorithm
    auto apex = currentPosition;
    auto portal_left = currentPosition;
    auto portal_right = currentPosition;

    size_t index_apex{0};
    size_t index_left{0};
    size_t index_right{0};

    for(size_t index_portal = 0; index_portal < portals.size(); ++index_portal) {
        const auto candidate_left = portals[index_portal].GetPoint2();
        const auto candidate_right = portals[index_portal].GetPoint1();
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

void NavMeshRoutingEngine::Update()
{
}

NavMeshRoutingEngine::GraphType::VertexId NavMeshRoutingEngine::findVertex(Point p) const
{
    for(const auto& id : _graph.Vertices()) {
        const auto& v = _graph.Vertex(id);
        if(v.aabb.Inside(p) && v.triangle.Inside(p)) {
            return id;
        }
    }
    // TODO(kratz): Use custom exception
    throw std::runtime_error("Point outside accessible area");
}
