/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometryBuilder.hpp"

#include "CollisionGeometry.hpp"
#include "DTriangulation.hpp"
#include "Geometry.hpp"
#include "Graph.hpp"
#include "Line.hpp"
#include "RoutingEngine.hpp"

#include <CGAL/Boolean_set_operations_2/difference.h>
#include <CGAL/number_utils.h>
#include <iterator>
#include <memory>
#include <poly2tri/common/shapes.h>

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

static Line edgeIndex(p2t::Triangle* from, p2t::Triangle* to)
{
    const Line dist(centroid(from), centroid(to));
    for(int index = 0; index < 3; ++index) {
        const auto a = from->GetPoint(index);
        const auto b = from->GetPoint((index + 1) % 3);
        const Line edge({a->x, a->y}, {b->x, b->y});
        if(dist.IntersectionWith(edge)) {
            return edge;
        }
    }
    throw std::logic_error("Triangles have to share an edge");
}

enum class Ordering { CW, CCW };

static Poly intoCGALPolygon(const std::vector<Point>& ring, Ordering ordering)
{
    Poly poly{};
    for(const auto& p : ring) {
        poly.push_back(Poly::Point_2{p.x, p.y});
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
    std::for_each(
        std::begin(accessibleAreaInput), std::end(accessibleAreaInput), [](const auto& p) {
            if(!p.is_simple()) {
                throw std::runtime_error("Supplied non simple accessible area.");
            }
        });
    PolyWithHolesList accessibleList{};
    CGAL::join(
        std::begin(accessibleAreaInput),
        std::end(accessibleAreaInput),
        std::back_inserter(accessibleList));

    if(accessibleList.size() != 1) {
        throw std::runtime_error("accesisble area not connected");
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
            throw std::runtime_error("Exclusion splits accessibleArea");
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
    std::stack<std::tuple<p2t::Triangle*, GraphType::VertexId>> toVisit{};
    std::set<p2t::Triangle*> known{};
    GraphType::Builder builder{};
    auto firstTriangle = triangulation.GetTriangles().front();
    toVisit.push({firstTriangle, builder.AddVertex(toVertexData(firstTriangle))});
    known.insert(std::get<0>(toVisit.top()));

    while(!toVisit.empty()) {
        auto top_of_stack = toVisit.top();
        auto [t, id] = top_of_stack;
        toVisit.pop();
        const auto pos_from = centroid(t);
        for(int index = 0; index < 3; ++index) {
            auto neighbor = t->GetNeighbor(index);
            if(neighbor == nullptr || known.count(neighbor) > 0 || !neighbor->IsInterior()) {
                continue;
            }
            const auto pos_to = centroid(neighbor);
            const auto neighbor_id = builder.AddVertex(toVertexData(neighbor));
            const auto weight = (pos_to - pos_from).Norm();
            builder.AddEdge(id, neighbor_id, {weight, edgeIndex(t, neighbor)});
            builder.AddEdge(neighbor_id, id, {weight, edgeIndex(neighbor, t)});
            known.insert(neighbor);
            toVisit.push({neighbor, neighbor_id});
        }
    }
    return {
        std::make_unique<CollisionGeometry>(collisionGeometryBuilder.Build()),
        std::make_unique<NavMeshRoutingEngine>(builder.Build())};
}
