// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometryBuilder.hpp"

#include "CollisionGeometry.hpp"
#include "DTriangulation.hpp"
#include "Geometry.hpp"
#include "Graph.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "RoutingEngine.hpp"
#include "SimulationError.hpp"

#include <CGAL/Boolean_set_operations_2/difference.h>
#include <CGAL/number_utils.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <poly2tri/common/shapes.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Poly = CGAL::Polygon_2<Kernel>;
using PolyWithHoles = CGAL::Polygon_with_holes_2<Kernel>;
using PolyWithHolesList = std::list<PolyWithHoles>;
using PolyList = std::list<Poly>;

static Point centroid(p2t::Triangle* t)
{
    Point res{};
    for(int index = 0; index < 3; ++index) {
        const auto p = t->GetPoint(index);
        res.x += p->x;
        res.y += p->y;
    }
    return res / 3.0;
};

static VertexData toVertexData(p2t::Triangle* triangle)
{
    Point points[3];
    for(int index = 0; index < 3; ++index) {
        const auto p2t_p = triangle->GetPoint(index);
        points[index] = Point{p2t_p->x, p2t_p->y};
    }
    return VertexData{AABB(points), Triangle{points[0], points[1], points[2]}};
}

GeometryBuilder& GeometryBuilder::AddAccessibleArea(const std::vector<Point>& lineLoop)
{
    _accessibleAreas.emplace_back(lineLoop);
    return *this;
}

GeometryBuilder& GeometryBuilder::ExcludeFromAccessibleArea(const std::vector<Point>& lineLoop)
{
    _exclusions.emplace_back(lineLoop);
    return *this;
}

Geometry GeometryBuilder::Build()
{
    using GraphType = NavMeshRoutingEngine::GraphType;

    const std::vector<Poly> accessibleListInput{
        std::begin(_accessibleAreas), std::end(_accessibleAreas)};
    PolyWithHolesList accessibleList{};

    CGAL::join(
        std::begin(accessibleListInput),
        std::end(accessibleListInput),
        std::back_inserter(accessibleList));

    if(accessibleList.size() != 1) {
        throw SimulationError("accessible area not connected");
    }

    auto accessibleArea = *accessibleList.begin();

    const std::vector<Poly> exclusionsListInput{std::begin(_exclusions), std::end(_exclusions)};
    PolyWithHolesList exclusionsList{};
    CGAL::join(
        std::begin(exclusionsListInput),
        std::end(exclusionsListInput),
        std::back_inserter(exclusionsList));

    for(const auto& ex : exclusionsList) {
        PolyWithHolesList res{};
        CGAL::difference(accessibleArea, ex, std::back_inserter(res));
        if(res.size() != 1) {
            throw SimulationError("Exclusion splits accessibleArea");
        }
        accessibleArea = *res.begin();
    }

    auto Convert = [](const auto& begin, const auto& end) {
        std::vector<Point> result{};
        result.reserve(end - begin);
        std::transform(begin, end, std::back_inserter(result), [](const auto& p) {
            return Point(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
        });
        return result;
    };
    const auto boundary = Convert(
        accessibleArea.outer_boundary().vertices_begin(),
        accessibleArea.outer_boundary().vertices_end());
    std::vector<std::vector<Point>> holes{};
    holes.reserve(accessibleArea.number_of_holes());
    std::transform(
        accessibleArea.holes_begin(),
        accessibleArea.holes_end(),
        std::back_inserter(holes),
        [Convert](const auto& hole) {
            return Convert(hole.vertices_begin(), hole.vertices_end());
        });

    std::unique_ptr<CollisionGeometry> collisionGeometry =
        std::make_unique<CollisionGeometry>(accessibleArea);

    DTriangulation triangulation(boundary, holes);
    std::stack<p2t::Triangle*> toVisit{};
    std::set<p2t::Triangle*> visited{};
    std::map<p2t::Triangle*, GraphType::VertexId> addedTriangles{};
    GraphType::Builder builder{};
    auto triangles = triangulation.GetTriangles();
    auto firstTriangle = *std::find_if(
        std::begin(triangles), std::end(triangles), [](const auto t) { return t->IsInterior(); });
    toVisit.push(firstTriangle);

    auto find_or_insert_id = [&builder, &addedTriangles](p2t::Triangle* t) -> GraphType::VertexId {
        auto iter = addedTriangles.find(t);
        if(iter != addedTriangles.end()) {
            return iter->second;
        }
        const auto id = builder.AddVertex(toVertexData(t));
        addedTriangles.emplace(std::make_pair(t, id));
        return id;
    };

    while(!toVisit.empty()) {
        auto top_of_stack = toVisit.top();
        auto t = top_of_stack;
        toVisit.pop();
        if(visited.count(t) > 0) {
            continue;
        } else {
            visited.insert(t);
        }
        const auto id = find_or_insert_id(t);

        const auto pos_from = centroid(t);
        for(int index = 0; index < 3; ++index) {
            // Triangle neighbors have the same index as the point oposing the shared edge. I.e.
            // neighbor[0] is the neighboring triangle that shares an edge created by the points 1
            // and 2.
            auto neighbor = t->GetNeighbor(index);
            if(neighbor == nullptr || !neighbor->IsInterior()) {
                continue;
            }
            const auto pos_to = centroid(neighbor);
            const auto neighbor_id = find_or_insert_id(neighbor);
            const auto weight = (pos_to - pos_from).Norm();
            const auto idx_edge_from = (index + 1) % 3;
            const auto edge_from = t->GetPoint(idx_edge_from);
            const auto idx_edge_to = (index + 2) % 3;
            const auto edge_to = t->GetPoint(idx_edge_to);

            auto from = Point(edge_from->x, edge_from->y);
            auto to = Point(edge_to->x, edge_to->y);
            const auto dir = (to - from).Normalized();
            from = from + dir * 0.2;
            to = to - dir * 0.2;

            builder.AddEdge(id, neighbor_id, {weight, {from, to}});
            toVisit.push(neighbor);
        }
    }
    return {std::move(collisionGeometry), std::make_unique<NavMeshRoutingEngine>(builder.Build())};
}
