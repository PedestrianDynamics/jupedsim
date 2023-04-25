/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometryBuilder.hpp"

#include "CollisionGeometry.hpp"
#include "DTriangulation.hpp"
#include "Geometry.hpp"
#include "Graph.hpp"
#include "Line.hpp"
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

enum class Ordering { CW, CCW };

static Poly intoCGALPolygon(const std::vector<Point>& ring, Ordering ordering)
{
    Poly poly{};
    for(const auto& p : ring) {
        poly.push_back(Poly::Point_2{p.x, p.y});
    }

    if(!poly.is_simple()) {
        throw SimulationError("Polygon is not simple: {}", ring);
    }
    const bool needsReversal = (ordering == Ordering::CW && poly.is_counterclockwise_oriented()) ||
                               (ordering == Ordering::CCW && poly.is_clockwise_oriented());
    if(needsReversal) {
        poly.reverse_orientation();
    }
    return poly;
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

    std::vector<Poly> accessibleAreaInput{};
    accessibleAreaInput.reserve(_accessibleAreas.size());
    std::transform(
        std::begin(_accessibleAreas),
        std::end(_accessibleAreas),
        std::back_inserter(accessibleAreaInput),
        [](const auto& p) { return intoCGALPolygon(p, Ordering::CCW); });
    PolyWithHolesList accessibleList{};
    CGAL::join(
        std::begin(accessibleAreaInput),
        std::end(accessibleAreaInput),
        std::back_inserter(accessibleList));

    if(accessibleList.size() != 1) {
        throw SimulationError("accesisble area not connected");
    }

    auto accessibleArea = *accessibleList.begin();

    std::vector<Poly> exclusionAreaInput{};
    exclusionAreaInput.reserve(_exclusions.size());
    std::transform(
        std::begin(_exclusions),
        std::end(_exclusions),
        std::back_inserter(exclusionAreaInput),
        [](const auto& p) { return intoCGALPolygon(p, Ordering::CCW); });

    for(const auto& ex : exclusionAreaInput) {
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
    CollisionGeometryBuilder collisionGeometryBuilder{};
    for(size_t index = 1; index < boundary.size(); ++index) {
        collisionGeometryBuilder.AddLineSegment(
            boundary[index - 1].x, boundary[index - 1].y, boundary[index].x, boundary[index].y);
    }
    collisionGeometryBuilder.AddLineSegment(
        boundary.back().x, boundary.back().y, boundary.front().x, boundary.front().y);
    for(const auto& hole : holes) {
        for(size_t index = 1; index < hole.size(); ++index) {
            collisionGeometryBuilder.AddLineSegment(
                hole[index - 1].x, hole[index - 1].y, hole[index].x, hole[index].y);
        }
        collisionGeometryBuilder.AddLineSegment(
            hole.back().x, hole.back().y, hole.front().x, hole.front().y);
    }

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
            builder.AddEdge(
                id,
                neighbor_id,
                {weight, {{edge_from->x, edge_from->y}, {edge_to->x, edge_to->y}}});
            toVisit.push(neighbor);
        }
    }
    return {
        std::make_unique<CollisionGeometry>(collisionGeometryBuilder.Build()),
        std::make_unique<NavMeshRoutingEngine>(builder.Build())};
}
