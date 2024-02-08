// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "routing_engine.hpp"
#include "mesh.hpp"
#include <limits>
#include <queue>

using PQueue = std::priority_queue<SearchNode*, std::vector<SearchNode*>, EvaluationFunction>;

constexpr bool EvaluationFunction::operator()(const SearchNode* lhs, const SearchNode* rhs) const
{
    return true;
}

RoutingEngine::RoutingEngine(std::unique_ptr<Mesh>&& mesh_) : mesh(std::move(mesh_))
{
    search_nodes.reserve(4096);
}

void push_successors(PQueue& pq)
{
}

void create_initial_search_nodes(PQueue& pq)
{
}

std::vector<glm::vec2> RoutingEngine::ComputeRoute(glm::vec2 start, glm::vec2 goal)
{
    // 1. Ensure start and goal are routable
    const auto start_polygon_index = mesh->FindContainingPolygon(start);
    if(start_polygon_index == Mesh::Polygon::InvalidIndex) {
        return {};
    }
    const auto goal_polygon_index = mesh->FindContainingPolygon(goal);
    if(goal_polygon_index == Mesh::Polygon::InvalidIndex) {
        return {};
    }

    // start and goal as inside the same convex polygon, hence there are no intermediate path points
    // required
    if(start_polygon_index == goal_polygon_index) {
        return {start, goal};
    }

    // 2. Create initial search node(s)
    //   -> if the start location is inside a polygon OR
    //      on the boundary OR
    //      unambigous corner OR
    //      ambigous corner WITH valid polygon:
    //      create 1 Searchnode
    //          <root=NONE,I[start,start]>
    //          with target polygon the polygon containing the start location
    //  -> if the start location is on an edge between polygons
    //      create 2 Searchnodes
    //          <root=NONE,I[start,start]>
    //          with target polygon being the polygon on one side of the edge
    //          <root=NONE,I[start,start]>
    //          with target polygon being the polygon on the other side of the edge
    //  -> if the start location is on an internal vertex create a serchnode for each polygon
    //  this
    //     vertex is part of
    //          <root=NONE,I[start,start]>
    //          with target polygon each of the polygons this vertex is part of
    search_nodes.clear();
    PQueue pq{};
    create_initial_search_nodes(pq);
    std::vector<double> costs(mesh->CountVertices(), std::numeric_limits<double>::max());

    // 3. Loop over priority queue
    while(!pq.empty()) {
        auto node = pq.top();
        pq.pop();
        if(node->next_polygon_index == goal_polygon_index) {
            // END CONDITION:
            // A path has been found if the target polygon equals the polygon containing the goal.
            //  -> Create a "final" SearchNode with either the preceding root or left or
            //     right vertex, depending on if goal is visible from preceding root
            size_t root_vertex_index = node->root_vertex_index;
            const glm::vec2 root_vertex = root_vertex_index == Mesh::Polygon::InvalidIndex ?
                                              start :
                                              mesh->Vertex(root_vertex_index);
            const glm::vec2 root_to_goal = goal - root_vertex;
            if(glm::dot(root_to_goal, node->left - root_vertex) < 0) {
                root_vertex_index = node->left_vertex_index;
            }
            if(glm::dot(node->right - root_vertex, root_vertex) < 0) {
                root_vertex_index = node->right_vertex_index;
            }
            search_nodes.emplace_back(
                node,
                root_vertex_index,
                goal,
                goal,
                Mesh::Polygon::InvalidIndex,
                Mesh::Polygon::InvalidIndex,
                goal_polygon_index,
                node->f,
                node->g);
            // TODO: build path and return it
            return {};
        }
        const size_t root_vertex_index = node->root_vertex_index;
        if(root_vertex_index != Mesh::Polygon::InvalidIndex) {
            if(costs[root_vertex_index] < node->g) {
                // There exists already a short path to the same root
                continue;
            }
        }
    }

    return {};
}
