// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentView.hpp"
#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "Geometry/Geometry3D.hpp"
#include "GeometryBuilder.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModels/CollisionFreeSpeedModel/CollisionFreeSpeedModel.hpp"

#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/boost/graph/iterator.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <set>
#include <utility>
#include <vector>

/// The migration rests on one claim: on a geometry that is flat, the mesh path answers what
/// the polygon path answers. Everywhere else it is checked against expectations written by
/// hand; here it is checked against the 2D it is meant to replace.
namespace
{
using State = CollisionFreeSpeedModel::State;

/// A room with a pillar in it, so that both things a model asks about are present: walls to
/// be pushed away from, and something to lose sight of another agent behind.
///
/// Every side is one polygon edge, the way 2D has always been fed.
PolyWithHoles room_with_pillar()
{
    GeometryBuilder b{};
    b.AddAccessibleArea({{0, 0}, {20, 0}, {20, 20}, {0, 20}});
    b.ExcludeFromAccessibleArea({{9, 9}, {11, 9}, {11, 11}, {9, 11}});
    return b.Build().Polygon();
}

/// A copy carrying the geometry and nothing else. Building a Geometry3D leaves a region map
/// behind on its mesh, and a second one refuses to take a mesh that already has one.
SurfaceMesh bare_copy(const SurfaceMesh& mesh)
{
    SurfaceMesh out{};
    std::vector<SurfaceMesh::Vertex_index> moved(mesh.number_of_vertices());
    for(const auto v : mesh.vertices()) {
        moved[static_cast<std::size_t>(v)] = out.add_vertex(mesh.point(v));
    }
    for(const auto f : mesh.faces()) {
        std::vector<SurfaceMesh::Vertex_index> corners{};
        for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            corners.push_back(moved[static_cast<std::size_t>(v)]);
        }
        out.add_face(corners);
    }
    return out;
}

/// The same outline, cut more finely along its boundary.
///
/// This is the whole point of comparing at all. A polygon arrives with each wall drawn as one
/// long edge, because that is how 2D has always had to be fed -- it never fused anything, so
/// one edge was one wall. A mesh arrives with the boundary cut into however many edges the
/// triangulation felt like, and fusing them back is what lets it answer the same question.
/// Comparing against an equally coarse mesh would leave nothing to fuse and prove nothing.
SurfaceMesh with_a_finer_boundary(SurfaceMesh mesh, double target_edge_length)
{
    std::vector<SurfaceMesh::Edge_index> border{};
    for(const auto e : mesh.edges()) {
        if(mesh.is_border(mesh.halfedge(e)) || mesh.is_border(mesh.opposite(mesh.halfedge(e)))) {
            border.push_back(e);
        }
    }
    CGAL::Polygon_mesh_processing::split_long_edges(border, target_edge_length, mesh);
    return mesh;
}

/// The same geometry twice. Built from the polygon it keeps its 2D view and takes the flat
/// path; built from that polygon's own triangulation, refined, it has no 2D view and takes
/// the mesh path. Same outline either way, so any difference is the path, not the shape.
struct BothWays {
    std::unique_ptr<Geometry3D> poly;
    std::unique_ptr<Geometry3D> mesh;

    explicit BothWays(double boundary_edge_length = 0.25)
        : poly(std::make_unique<Geometry3D>(room_with_pillar()))
        , mesh(
              std::make_unique<Geometry3D>(
                  with_a_finer_boundary(bare_copy(poly->mesh()), boundary_edge_length)))
    {
    }
};

GenericAgent make_agent(Point position, Point target)
{
    GenericAgent agent(
        GenericAgent::ID::Invalid,
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        position,
        State{});
    agent.nextTarget = target;
    return agent;
}

/// Where the agents stand and where they are heading. Spread around the pillar, so some pairs
/// see each other and some do not, and some are close enough to a wall to feel it.
///
/// Those last ones stand off-centre on purpose. Square in front of a wall the pieces of a
/// chopped one push symmetrically, their sideways parts cancel, and the direction that comes
/// out of Normalized() is the same as a single fused wall would give -- the step would agree
/// however badly the walls were cut.
std::vector<std::pair<Point, Point>> crowd()
{
    return {
        {{10.0, 6.0}, {10.0, 18.0}}, // below the pillar, heading past it
        {{10.0, 14.0}, {10.0, 2.0}}, // above it, heading the other way -- out of sight
        {{6.0, 10.0}, {18.0, 10.0}}, // beside it
        {{0.3, 0.8}, {19.0, 19.0}}, // near the left wall, close to one of its ends
        {{8.6, 9.7}, {1.0, 10.0}}, // alongside the pillar, off its centre
    };
}

/// A room full of people, all heading for the same exit, with the pillar left clear. Five
/// agents in a hall barely touch each other; this is where a difference has somewhere to go.
std::vector<std::pair<Point, Point>> dense_crowd(double spacing)
{
    std::vector<std::pair<Point, Point>> who{};
    for(double x = 1.0; x < 19.0; x += spacing) {
        for(double y = 1.0; y < 19.0; y += spacing) {
            if(x > 8.4 && x < 11.6 && y > 8.4 && y < 11.6) {
                continue;
            }
            who.push_back({{x, y}, {19.5, 10.0}});
        }
    }
    return who;
}

AgentContainer<GenericAgent>
populate(const Geometry3D& geo, const std::vector<std::pair<Point, Point>>& who = crowd())
{
    AgentContainer<GenericAgent> agents{};
    for(const auto& [position, target] : who) {
        agents.push_back(make_agent(position, target));
    }
    for(auto& agent : agents) {
        agent.location = geo.get_location(agent.Position().x, agent.Position().y, 0.0);
    }
    return agents;
}

/// The walls one agent is offered, as a set of undirected segments.
///
/// Undirected because the two paths walk their boundaries in opposite directions and hand the
/// same wall over with its ends swapped. Which wall it is does not depend on that, and that
/// is what this compares.
///
/// The direction is nonetheless not free, which is why the mesh path goes to the trouble of
/// facing the same way: `ShortestPoint` projects from the segment's first end, so the same
/// wall given the other way round hands an agent a closest point that differs in the last
/// bit, and the steps below stop being identical.
std::set<std::array<double, 4>> walls_around(const Geometry3D& geo, std::size_t who)
{
    auto agents = populate(geo);
    NeighborhoodSearch<GenericAgent> search{5.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    std::set<std::array<double, 4>> walls{};
    for(const auto& wall : AgentView{query, agents[who]}.WallsNearby()) {
        const std::array<double, 4> forwards{
            wall.segment.p1.x, wall.segment.p1.y, wall.segment.p2.x, wall.segment.p2.y};
        const std::array<double, 4> backwards{
            wall.segment.p2.x, wall.segment.p2.y, wall.segment.p1.x, wall.segment.p1.y};
        walls.insert(std::min(forwards, backwards));
    }
    return walls;
}

/// One step of every agent, as the model computes it.
std::vector<Point> steps(const Geometry3D& geo)
{
    auto agents = populate(geo);
    NeighborhoodSearch<GenericAgent> search{5.0};
    search.Update(agents);
    const EnvironmentQuery query{geo, search};

    // A wall reach far wider than the default 0.02 m, so that many boundary pieces fall
    // inside it at once. At the default only the single nearest piece is ever in range and
    // the sum cannot tell a fused wall from a chopped one -- the step would agree for want
    // of looking.
    const CollisionFreeSpeedModel model{8.0, 0.1, 5.0, 0.5};
    std::vector<Point> deltas{};
    for(const auto& agent : agents) {
        OperationalModelState next = agent.state;
        deltas.push_back(model.ComputeNextState(agent.state, next, AgentStep{query, agent, 0.05}));
    }
    return deltas;
}

/// Where the crowd stands after @p how_many steps, walked one step at a time: move, put the
/// new position back on the surface, search the neighbourhood again. Empty if anyone walked
/// off the geometry, which would make the comparison meaningless rather than failing.
std::vector<Point> positions_after(
    const Geometry3D& geo,
    const std::vector<std::pair<Point, Point>>& who,
    std::size_t how_many)
{
    auto agents = populate(geo, who);
    const CollisionFreeSpeedModel model{8.0, 0.1, 5.0, 0.5};
    NeighborhoodSearch<GenericAgent> search{5.0};

    for(std::size_t s = 0; s < how_many; ++s) {
        search.Update(agents);
        const EnvironmentQuery query{geo, search};

        std::vector<Point> deltas{};
        std::vector<OperationalModelState> updated{};
        for(const auto& agent : agents) {
            OperationalModelState next = agent.state;
            deltas.push_back(
                model.ComputeNextState(agent.state, next, AgentStep{query, agent, 0.05}));
            updated.push_back(next);
        }
        for(std::size_t i = 0; i < agents.size(); ++i) {
            agents[i].MoveAlongSurface(deltas[i]);
            agents[i].state = updated[i];
            auto where = geo.get_location(agents[i].Position().x, agents[i].Position().y, 0.0);
            if(!where.has_value()) {
                return {};
            }
            agents[i].location = where;
        }
    }

    std::vector<Point> out{};
    for(const auto& agent : agents) {
        out.push_back(agent.Position());
    }
    return out;
}

} // namespace

TEST(FlatMeshParity, TheMeshOffersTheSameWalls)
{
    const BothWays geo{};

    for(std::size_t who = 0; who < crowd().size(); ++who) {
        EXPECT_EQ(walls_around(*geo.poly, who), walls_around(*geo.mesh, who)) << "agent " << who;
    }
}

TEST(FlatMeshParity, TheMeshDrawsTheSameSightLines)
{
    const BothWays geo{};

    // Across the pillar, past it on either side, and the length of the room. Sampled rather
    // than argued: the two paths test against different sets of segments, and a chord that
    // grazes a corner is where that could tell.
    for(double x = 0.5; x < 20.0; x += 0.75) {
        for(double y = 0.5; y < 20.0; y += 0.75) {
            const auto from_poly = geo.poly->get_location(x, y, 0.0);
            const auto from_mesh = geo.mesh->get_location(x, y, 0.0);
            ASSERT_EQ(from_poly.has_value(), from_mesh.has_value()) << "at " << x << "," << y;
            if(!from_poly.has_value()) {
                continue;
            }
            for(const auto& to : {Point{10.0, 10.0}, Point{1.0, 19.0}, Point{19.0, 1.0}}) {
                EXPECT_EQ(
                    geo.poly->no_geometry_between(*from_poly, to - from_poly->xy()),
                    geo.mesh->no_geometry_between(*from_mesh, to - from_mesh->xy()))
                    << "from " << x << "," << y << " to " << to.x << "," << to.y;
            }
        }
    }
}

TEST(FlatMeshParity, TheMeshProducesTheSameModelStep)
{
    const BothWays geo{};

    // The point of the exercise: whatever the two paths do internally, a model standing on
    // either of them moves by the same amount.
    //
    // To the last bit and not beyond it. The walls are now identical and given the same way
    // round, so what is left is that the two grids hand them over in a different order and a
    // model adds up repulsion per wall. Where the sideways pushes cancel exactly that leaves
    // the rounding on its own -- around 1e-21 against a step of 0.06, which is why the crowd
    // below still ends up in bit-identical positions.
    const auto on_polygon = steps(*geo.poly);
    const auto on_mesh = steps(*geo.mesh);

    ASSERT_EQ(on_polygon.size(), on_mesh.size());
    for(std::size_t i = 0; i < on_polygon.size(); ++i) {
        EXPECT_NEAR(on_polygon[i].x, on_mesh[i].x, 1e-16) << "agent " << i;
        EXPECT_NEAR(on_polygon[i].y, on_mesh[i].y, 1e-16) << "agent " << i;
    }
}

TEST(FlatMeshParity, ACrowdWalksTogetherOnBothForAsLongAsItCan)
{
    const BothWays geo{};
    const auto who = dense_crowd(1.0);
    ASSERT_GT(who.size(), 300u);

    // Ten steps of three hundred people, every one of them in the same place on both, to the
    // last bit.
    //
    // Ten and not five hundred, and the reason is the model rather than the mesh. What is
    // still not shared is the order the two grids hand walls over in -- the same walls, added
    // up the other way round -- and after some fifty steps that rounds differently somewhere.
    // From there the two separate fast: 9e-2 by a hundred steps, metres by five hundred. That
    // is not a defect but what this crowd does with any last-bit difference at all: adding one
    // ULP to every step of a polygon-path run, changing nothing else, takes it 5 m from its
    // own twin over the same horizon, while a single one-off nudge dies out completely. It is
    // contractive against one perturbation and not against a steady one, because sooner or
    // later a steady one flips which neighbour is nearest, and that is a finite decision.
    //
    // So M12 can expect an identical step but not an identical trajectory.
    const auto on_polygon = positions_after(*geo.poly, who, 10);
    const auto on_mesh = positions_after(*geo.mesh, who, 10);

    ASSERT_FALSE(on_polygon.empty()) << "the crowd left the geometry, so nothing was compared";
    EXPECT_EQ(on_polygon, on_mesh);
}
