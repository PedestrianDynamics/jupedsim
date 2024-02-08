// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "glm/ext/vector_float2.hpp"
#include "mesh.hpp"

/// Defined in the paper as tuple of intervall 'I' = [a,b] and root 'r' where each point in 'I' is
/// visible from 'r'.
struct SearchNode {
    /// Non-owning pointer.
    SearchNode* parent{};
    size_t root_vertex_index{Mesh::Polygon::InvalidIndex};
    /// Left side of the intervall 'I'
    glm::vec2 left{};
    /// Right side of the intervall 'I'
    glm::vec2 right{};
    size_t left_vertex_index{};
    size_t right_vertex_index{};
    size_t next_polygon_index{};
    double f{};
    double g{};

    /// CTor to allow emplace_back into vector, fuck you clang
    SearchNode(
        SearchNode* parent_,
        size_t root_vertex_index_,
        glm::vec2 left_,
        glm::vec2 right_,
        size_t left_vertex_index_,
        size_t right_vertex_index_,
        size_t next_polygon_index_,
        double f_,
        double g_)
        : parent(parent_)
        , root_vertex_index(root_vertex_index_)
        , left(left_)
        , right(right_)
        , left_vertex_index(left_vertex_index_)
        , right_vertex_index(right_vertex_index_)
        , next_polygon_index(next_polygon_index_)
        , f(f_)
        , g(g_)
    {
    }
};

struct EvaluationFunction {
    constexpr bool operator()(const SearchNode* lhs, const SearchNode* rhs) const;
};

class RoutingEngine
{
    /// Must form a single simple polygon with holes
    std::unique_ptr<Mesh> mesh;
    /// SearchNode storage
    std::vector<SearchNode> search_nodes{};

public:
    explicit RoutingEngine(std::unique_ptr<Mesh>&& mesh_);
    ~RoutingEngine() = default;

    /// Tests if 'p' is inside the routable area
    bool IsRoutable(glm::vec2 p) const;

    /// Computes the path from start to goal with intermediate turning points. Returned points
    /// contain [start, turning_point_0, ..., turning_point_n, goal]
    std::vector<glm::vec2> ComputeRoute(glm::vec2 start, glm::vec2 goal);
};
