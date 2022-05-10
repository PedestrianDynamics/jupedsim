#include "NavMeshRoutingFactory.hpp"

#include "DTriangulation.hpp"
#include "Graph.hpp"
#include "Point.hpp"
#include "RoutingEngine.hpp"
#include "geometry/SubRoom.hpp"

#include <CGAL/number_utils.h>
#include <poly2tri/common/shapes.h>

#include <algorithm>
#include <stdexcept>

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

NavMeshRoutingEngine MakeFromBuilding(const Building& building)
{
    using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
    using Poly = CGAL::Polygon_2<Kernel>;
    using PolyWithHoles = CGAL::Polygon_with_holes_2<Kernel>;
    using PolyWithHolesList = std::list<PolyWithHoles>;
    using GraphType = NavMeshRoutingEngine::GraphType;

    PolyWithHolesList input_pl{};
    for(const auto& [_, room] : building.GetAllRooms()) {
        for(const auto& [_, sub_room] : room->GetAllSubRooms()) {
            Poly poly;
            for(const auto& p : sub_room->GetPolygon()) {
                poly.push_back({p.x, p.y});
            }
            PolyWithHoles poly_with_holes{poly};
            for(const auto& o : sub_room->GetAllObstacles()) {
                Poly hole;
                for(const auto& p : o->GetPolygon()) {
                    hole.push_back({p.x, p.y});
                }
                poly_with_holes.add_hole(hole);
            }
            input_pl.emplace_back(poly_with_holes);
        }
    }
    PolyWithHolesList union_pl{};
    CGAL::join(std::begin(input_pl), std::end(input_pl), std::back_inserter(union_pl));

    if(union_pl.size() != 1) {
        // TODO (kkratz): throw custom exception
        throw std::runtime_error("Walkable area needs to be a single polygon");
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
        union_pl.begin()->outer_boundary().vertices_begin(),
        union_pl.begin()->outer_boundary().vertices_end());

    std::vector<std::vector<Point>> holes{};
    holes.reserve(union_pl.begin()->number_of_holes());
    std::transform(
        union_pl.begin()->holes_begin(),
        union_pl.begin()->holes_end(),
        std::back_inserter(holes),
        [Convert](const auto& hole) {
            return Convert(hole.vertices_begin(), hole.vertices_end());
        });
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

    return NavMeshRoutingEngine{builder.Build()};
}
