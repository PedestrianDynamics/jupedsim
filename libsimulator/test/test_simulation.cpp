// SPDX-License-Identifier: LGPL-3.0-or-later
#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "geometry_fixtures.hpp"
#include "kernel.hpp"
#include "mesh_fixtures.hpp"
#include "operational_models/collision_free_speed_model/collision_free_speed_model.hpp"
#include "polygon.hpp"
#include "route.hpp"
#include "simulation.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "test_common.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace
{
using State = CollisionFreeSpeedModel::State;

std::unique_ptr<CollisionFreeSpeedModel> model()
{
    return std::make_unique<CollisionFreeSpeedModel>(8.0, 0.1, 5.0, 0.02);
}

/// The switchback stair straight from a surface mesh, the way an OBJ file arrives: a geometry
/// with no authored regions, which a simulation does not take.
std::unique_ptr<Simulation> on_a_mesh_built_switchback_stair()
{
    return std::make_unique<Simulation>(
        model(),
        std::make_unique<Geometry>(fixtures::switchback_stair()),
        0.01,
        FloorfieldOptions{});
}

/// A simulation on a flat 20 x 20 room, and the id of that room's region.
struct FlatRoom {
    std::unique_ptr<Simulation> sim;
    std::size_t room_id;
};

FlatRoom on_a_flat_room()
{
    auto room = test_geometries::rectangle({0, 0}, {20, 20});
    return {
        std::make_unique<Simulation>(model(), std::move(room.geometry), 0.01, FloorfieldOptions{}),
        room.room_id};
}

/// Both floors of the U-stair carry this (x, y).
const Point stacked_point{2, 10};

/// A route of one waypoint, so that agents have somewhere to be routed to.
std::pair<Route::ID, BaseStage::ID> route_to(Simulation& sim, Point position, std::size_t region_id)
{
    const Vector half{0.5, 0.5};
    const Polygon area{test_geometries::rectangle_points(position - half, position + half)};
    const auto stage = sim.add_stage(WaypointDescription{{{area, region_id}}});
    const auto route = sim.add_route({{stage, NoRuleDescription{}}});
    return {route, stage};
}

/// A 1 x 1 m square around @p centre.
Polygon square_at(Point centre)
{
    const Vector half{0.5, 0.5};
    return Polygon{test_geometries::rectangle_points(centre - half, centre + half)};
}

/// A flat room with a waypoint at (14, 10) and exits at (3, 10) and (17, 10).
struct ClosureRoom {
    std::unique_ptr<Simulation> sim;
    std::size_t room;
    BaseStage::ID wp;
    BaseStage::ID exit1;
    BaseStage::ID exit2;
};

ClosureRoom a_closure_room()
{
    auto [sim, room] = on_a_flat_room();
    const auto wp = sim->add_stage(WaypointDescription{{{square_at({14, 10}), room}}});
    const auto exit1 = sim->add_stage(ExitDescription{{{square_at({3, 10}), room}}});
    const auto exit2 = sim->add_stage(ExitDescription{{{square_at({17, 10}), room}}});
    return {std::move(sim), room, wp, exit1, exit2};
}

/// At the waypoint, a round robin sends agents to exit1 and exit2 in turn.
Route::ID branch(ClosureRoom& r)
{
    return r.sim->add_route(
        {{r.wp, RoundRobinDescription({{r.exit1, 1}, {r.exit2, 1}})},
         {r.exit1, NoRuleDescription{}},
         {r.exit2, NoRuleDescription{}}});
}

/// From the waypoint, a fixed transition leads to exit2.
Route::ID corridor(ClosureRoom& r)
{
    return r.sim->add_route(
        {{r.wp, FixedRuleDescription(r.exit2)}, {r.exit2, NoRuleDescription{}}});
}

/// An agent standing in the waypoint, which decides on its next stage as it is added.
GenericAgent::ID add_at_waypoint(ClosureRoom& r, Route::ID route, double y = 10)
{
    return r.sim->add_agent(route, r.wp, Point{14, y}, State{}, r.room);
}

/// Iterates until @p done holds; fails after 5000 iterations.
template <typename Done>
void iterate_until(Simulation& sim, Done done)
{
    while(!done()) {
        sim.iterate();
        ASSERT_LT(sim.iteration(), 5000u) << "condition never met";
    }
}

/// Whether @p where lies in the area of stage @p stage.
bool in_area(const ClosureRoom& r, BaseStage::ID stage, const Location& where)
{
    return r.sim->stage(stage)->destination()->contains(where);
}

/// Whether agent @p id is routed to the area of stage @p stage, told by its centre.
bool heads_for(const ClosureRoom& r, GenericAgent::ID id, Point centre)
{
    const auto* target = std::get_if<Destination>(&r.sim->agent(id).final_target);
    return target != nullptr &&
           target->contains(r.sim->get_location(centre.x(), centre.y(), r.room));
}

/// Where a stranded agent stands still.
const Location& stranded_at(const ClosureRoom& r, GenericAgent::ID id)
{
    return std::get<Location>(r.sim->agent(id).final_target);
}

const Point exit1_centre{3, 10};
const Point exit2_centre{17, 10};

} // namespace

TEST(MultiStoreySimulation, RunsOnAMultiStoreySurface)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [route, stage] = route_to(sim, stacked_point, stair.room_2);

    const auto id = sim.add_agent(route, stage, Point{2, 2}, State{}, stair.room_1);
    ASSERT_EQ(sim.agent_count(), 1u);
    EXPECT_NO_THROW(sim.iterate());

    // Heading for the stair, which is the only way up: over there in plan, and still on the
    // ground floor.
    const auto& agent = sim.agent(id);
    EXPECT_GT(agent.route_orientation.x(), 0.0);
    EXPECT_EQ(agent.location.z(), 0.0);
}

TEST(MultiStoreySimulation, AnAgentIsPutInTheRegionItNames)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [route, stage] = route_to(sim, stacked_point, stair.room_2);

    const auto downstairs = sim.add_agent(route, stage, stacked_point, State{}, stair.room_1);
    const auto upstairs = sim.add_agent(route, stage, stacked_point, State{}, stair.room_2);

    EXPECT_EQ(sim.agent(downstairs).location.region(), stair.room_1);
    EXPECT_EQ(sim.agent(upstairs).location.region(), stair.room_2);
    EXPECT_NEAR(sim.agent(upstairs).location.z(), 3.0, 1e-9);
}

TEST(MultiStoreySimulation, AStageIsPutInTheRegionItNames)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [up_route, up_stage] = route_to(sim, stacked_point, stair.room_2);
    const auto [down_route, down_stage] = route_to(sim, stacked_point, stair.room_1);

    // Upstairs, the waypoint is only reached over the stair to the east.
    const auto id = sim.add_agent(up_route, up_stage, Point{2, 2}, State{}, stair.room_1);
    sim.iterate();
    EXPECT_GT(sim.agent(id).route_orientation.x(), 0.5);

    // Downstairs, it is in plain sight. Floor field directions follow a gradient on a grid, so
    // straight ahead holds only up to the grid.
    sim.switch_agent_route(id, down_route, down_stage);
    sim.iterate();
    const Vector straight = (stacked_point - sim.agent(id).location.xy()).normalized();
    EXPECT_NEAR(sim.agent(id).route_orientation.x(), straight.x(), 1e-2);
    EXPECT_NEAR(sim.agent(id).route_orientation.y(), straight.y(), 1e-2);
}

TEST(MultiStoreySimulation, ATargetWrittenFromOutsideLandsOnTheAgentsOwnStorey)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [route, stage] = route_to(sim, stacked_point, stair.room_2);
    const auto upstairs = sim.add_agent(route, stage, stacked_point, State{}, stair.room_2);

    // Only (x, y) is given, and two storeys carry it. It has to mean the one the agent is on --
    // anything else routes it through the wrong floor.
    sim.set_agent_target(upstairs, Point{2, 6});
    EXPECT_EQ(std::get<Location>(sim.agent(upstairs).final_target).region(), stair.room_2);

    const auto downstairs = sim.add_agent(route, stage, stacked_point, State{}, stair.room_1);
    sim.set_agent_target(downstairs, Point{2, 6});
    EXPECT_EQ(std::get<Location>(sim.agent(downstairs).final_target).region(), stair.room_1);
}

TEST(Simulation, RemovalRecordHoldsExitAndTime)
{
    auto [sim, room] = on_a_flat_room();
    const Polygon area{test_geometries::rectangle_points({9.5, 9.5}, {10.5, 10.5})};
    const auto exit = sim->add_stage(ExitDescription{{{area, room}}});
    const auto route = sim->add_route({{exit, NoRuleDescription{}}});
    const auto id = sim->add_agent(route, exit, Point{7, 10}, State{}, room);

    // The record appears after the iteration in which the agent reached the exit, stamped with
    // the elapsed time at which it did, while the agent is still in the simulation.
    double reached_at = 0;
    while(sim->removed_agents().empty()) {
        reached_at = sim->elapsed_time();
        sim->iterate();
        ASSERT_LT(sim->iteration(), 5000u) << "never reached the exit";
    }
    ASSERT_EQ(sim->removed_agents().size(), 1u);
    const auto& record = sim->removed_agents().front();
    EXPECT_EQ(record.id, id);
    EXPECT_EQ(record.exit, exit);
    EXPECT_DOUBLE_EQ(record.time, reached_at);
    EXPECT_EQ(sim->agent_count(), 1u);

    // The next iteration removes the agent and replaces the records.
    sim->iterate();
    EXPECT_EQ(sim->agent_count(), 0u);
    EXPECT_TRUE(sim->removed_agents().empty());
}

TEST(Simulation, MarkingAnAgentForRemovalRecordsNoExit)
{
    auto [sim, room] = on_a_flat_room();
    const auto [route, stage] = route_to(*sim, {10, 10}, room);
    const auto id = sim->add_agent(route, stage, Point{5, 5}, State{}, room);
    sim->iterate();

    sim->mark_agent_for_removal(id);

    ASSERT_EQ(sim->removed_agents().size(), 1u);
    EXPECT_EQ(sim->removed_agents().front().id, id);
    EXPECT_FALSE(sim->removed_agents().front().exit.has_value());
    EXPECT_DOUBLE_EQ(sim->removed_agents().front().time, sim->elapsed_time());
}

TEST(Simulation, AnAgentHasOneRemovalRecord)
{
    auto [sim, room] = on_a_flat_room();
    const Polygon area{test_geometries::rectangle_points({9.5, 9.5}, {10.5, 10.5})};
    const auto exit = sim->add_stage(ExitDescription{{{area, room}}});
    const auto route = sim->add_route({{exit, NoRuleDescription{}}});
    const auto id = sim->add_agent(route, exit, Point{7, 10}, State{}, room);
    const auto other = sim->add_agent(route, exit, Point{7, 12}, State{}, room);
    sim->mark_agent_for_removal(other);
    sim->mark_agent_for_removal(other);
    ASSERT_EQ(sim->removed_agents().size(), 1u);
    EXPECT_FALSE(sim->removed_agents().front().exit.has_value());

    sim->iterate();
    while(sim->removed_agents().empty()) {
        sim->iterate();
        ASSERT_LT(sim->iteration(), 5000u) << "never reached the exit";
    }
    sim->mark_agent_for_removal(id);

    // The exit record wins over the later removal.
    ASSERT_EQ(sim->removed_agents().size(), 1u);
    EXPECT_EQ(sim->removed_agents().front().id, id);
    EXPECT_EQ(sim->removed_agents().front().exit, exit);
}

TEST(Simulation, TwoAgentsReachTheirOwnExits)
{
    auto [sim, room] = on_a_flat_room();
    const Polygon left{test_geometries::rectangle_points({4.5, 9.5}, {5.5, 10.5})};
    const Polygon right{test_geometries::rectangle_points({14.5, 9.5}, {15.5, 10.5})};
    const auto exit_left = sim->add_stage(ExitDescription{{{left, room}}});
    const auto exit_right = sim->add_stage(ExitDescription{{{right, room}}});
    const auto route_left = sim->add_route({{exit_left, NoRuleDescription{}}});
    const auto route_right = sim->add_route({{exit_right, NoRuleDescription{}}});
    const auto a = sim->add_agent(route_left, exit_left, Point{3, 10}, State{}, room);
    const auto b = sim->add_agent(route_right, exit_right, Point{17, 10}, State{}, room);

    while(sim->removed_agents().empty()) {
        sim->iterate();
        ASSERT_LT(sim->iteration(), 5000u) << "never reached an exit";
    }
    ASSERT_EQ(sim->removed_agents().size(), 2u);
    for(const auto& record : sim->removed_agents()) {
        EXPECT_EQ(record.exit, record.id == a ? exit_left : exit_right);
        EXPECT_TRUE(record.id == a || record.id == b);
    }
}

TEST(Simulation, AnExitMayBeConcave)
{
    const auto flat = on_a_flat_room();
    const Polygon l_shape{std::vector<Point>{{2, 2}, {6, 2}, {6, 4}, {4, 4}, {4, 6}, {2, 6}}};
    EXPECT_NO_THROW(flat.sim->add_stage(ExitDescription{{{l_shape, flat.room_id}}}));
}

TEST(Simulation, AStageInAnUnknownRegionIsRejected)
{
    const auto flat = on_a_flat_room();
    const Polygon square{std::vector<Point>{{2, 2}, {4, 2}, {4, 4}, {2, 4}}};
    EXPECT_THROW(flat.sim->add_stage(ExitDescription{{{square, 99}}}), SimulationError);
}

/// Row-major index of the cell of @p info that @p xy lies in.
std::size_t cell_of(const GridInfo& info, Point xy)
{
    const auto col = static_cast<std::size_t>((xy.x() - info.origin.x()) / info.cell_size);
    const auto row = static_cast<std::size_t>((xy.y() - info.origin.y()) / info.cell_size);
    return row * info.width + col;
}

/// Travel time of the cell of @p grid that @p xy lies in.
double time_at(const TravelTimeGrid& grid, Point xy)
{
    return grid.values.at(cell_of(grid.info, xy));
}

TEST(Simulation, ReportsTheTravelTimesToAStage)
{
    const auto flat = on_a_flat_room();
    const Polygon strip{std::vector<Point>{{0, 0}, {1, 0}, {1, 20}, {0, 20}}};
    const auto exit = flat.sim->add_stage(ExitDescription{{{strip, flat.room_id}}});

    const auto grids = flat.sim->travel_times(exit);
    ASSERT_EQ(grids.size(), 1u);
    const auto& grid = grids.at(flat.room_id);

    EXPECT_EQ(grid.values.size(), grid.info.cell_count());
    // The grid covers the whole room.
    EXPECT_LE(grid.info.origin.x(), 0.0);
    EXPECT_LE(grid.info.origin.y(), 0.0);
    EXPECT_GE(grid.info.origin.x() + grid.info.width * grid.info.cell_size, 20.0);
    EXPECT_GE(grid.info.origin.y() + grid.info.height * grid.info.cell_size, 20.0);
    EXPECT_EQ(time_at(grid, {0.5, 10}), 0.0) << "inside the exit";
    EXPECT_GT(time_at(grid, {19.5, 10}), time_at(grid, {10, 10}));
    EXPECT_GT(time_at(grid, {10, 10}), 0.0);

    ASSERT_EQ(grid.directions.size(), grid.values.size());
    const auto direction_at = [&grid](Point xy) {
        return grid.directions.at(cell_of(grid.info, xy));
    };
    EXPECT_LT(direction_at({10, 10}).x(), -0.99) << "towards the exit on the left";
    EXPECT_EQ(direction_at({0.5, 10}), (Vector{0, 0})) << "inside the exit";
}

TEST(Simulation, ListsItsStagesInAscendingOrder)
{
    const auto flat = on_a_flat_room();
    const Polygon square{std::vector<Point>{{2, 2}, {4, 2}, {4, 4}, {2, 4}}};
    const auto exit = flat.sim->add_stage(ExitDescription{{{square, flat.room_id}}});
    const auto steering = flat.sim->add_stage(DirectSteeringDescription{});
    const auto waypoint = flat.sim->add_stage(WaypointDescription{{{square, flat.room_id}}});
    EXPECT_EQ(flat.sim->stage_ids(), (std::vector<BaseStage::ID>{exit, steering, waypoint}));
}

TEST(Simulation, ReportsTheTravelTimesOfEveryRegion)
{
    auto stair = test_geometries::u_stair();
    const auto region_count = stair.geometry->region_count();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const Polygon square{std::vector<Point>{{1, 9}, {3, 9}, {3, 11}, {1, 11}}};
    const auto exit = sim.add_stage(ExitDescription{{{square, stair.room_2}}});

    const auto grids = sim.travel_times(exit);

    ASSERT_EQ(grids.size(), region_count);
    for(const auto& [region_id, grid] : grids) {
        EXPECT_EQ(grid.values.size(), grid.info.cell_count()) << "region " << region_id;
        const auto shortest = *std::min_element(grid.values.begin(), grid.values.end());
        // Only the exit's own region holds cells inside it.
        if(region_id == stair.room_2) {
            EXPECT_EQ(shortest, 0.0);
        } else {
            EXPECT_GT(shortest, 0.0) << "region " << region_id;
        }
    }
}

TEST(Simulation, AStageWithoutAnAreaHasNoTravelTimes)
{
    const auto flat = on_a_flat_room();
    const auto steering = flat.sim->add_stage(DirectSteeringDescription{});
    EXPECT_TRUE(flat.sim->travel_times(steering).empty());
}

/// @p grid's values, each multiplied by @p factor.
std::vector<double> scaled(const SpeedGrid& grid, double factor)
{
    std::vector<double> values = grid.values;
    for(auto& v : values) {
        v *= factor;
    }
    return values;
}

TEST(Simulation, ReportsTheDefaultSpeedFieldOfEveryRegion)
{
    auto stair = test_geometries::u_stair();
    const auto region_count = stair.geometry->region_count();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const Polygon square{std::vector<Point>{{1, 9}, {3, 9}, {3, 11}, {1, 11}}};
    const auto exit = sim.add_stage(ExitDescription{{{square, stair.room_2}}});

    const auto defaults = sim.default_speed_fields();
    const auto travel_times = sim.travel_times(exit);
    ASSERT_EQ(defaults.size(), region_count);
    for(const auto& [region_id, grid] : defaults) {
        const auto& times = travel_times.at(region_id);
        EXPECT_EQ(grid.info, times.info) << "region " << region_id;
        EXPECT_EQ(sim.grid_info(region_id), grid.info) << "region " << region_id;
        ASSERT_EQ(grid.values.size(), grid.info.cell_count());
        // Walkable cells, and only those, have a speed and a finite travel time.
        for(std::size_t cell = 0; cell < grid.values.size(); ++cell) {
            EXPECT_EQ(grid.values[cell] > 0.0, std::isfinite(times.values[cell]))
                << "region " << region_id << ", cell " << cell;
        }
    }
    const auto custom = sim.custom_speed_fields(exit);
    ASSERT_EQ(custom.size(), region_count);
    for(const auto& [region_id, grid] : custom) {
        EXPECT_FALSE(grid.has_value()) << "no override in region " << region_id;
    }
}

// How speeds change travel times is the floor field's to test (libfloorfield); here only which
// stage is routed with which speed field.
TEST(Simulation, SpeedFieldsReachTheirStages)
{
    const auto flat = on_a_flat_room();
    const Polygon left{std::vector<Point>{{0, 0}, {1, 0}, {1, 20}, {0, 20}}};
    const Polygon right{std::vector<Point>{{19, 0}, {20, 0}, {20, 20}, {19, 20}}};
    const auto custom = flat.sim->add_stage(ExitDescription{{{left, flat.room_id}}});
    const auto other = flat.sim->add_stage(ExitDescription{{{right, flat.room_id}}});
    const auto time = [&flat](BaseStage::ID stage) {
        return time_at(flat.sim->travel_times(stage).at(flat.room_id), {10, 10});
    };
    const auto defaults = flat.sim->default_speed_fields().at(flat.room_id);
    const auto custom_initial = time(custom);
    const auto other_initial = time(other);

    // A custom speed field reaches its own stage only.
    flat.sim->set_custom_speed_field(custom, flat.room_id, scaled(defaults, 2.0));
    const auto custom_field = flat.sim->custom_speed_fields(custom).at(flat.room_id);
    ASSERT_TRUE(custom_field.has_value());
    EXPECT_EQ(custom_field->values, scaled(defaults, 2.0));
    EXPECT_FALSE(flat.sim->custom_speed_fields(other).at(flat.room_id).has_value());
    const auto custom_faster = time(custom);
    EXPECT_LT(custom_faster, custom_initial);
    EXPECT_EQ(time(other), other_initial);

    // The default reaches every stage but the one with a custom speed field.
    flat.sim->set_default_speed_field(flat.room_id, scaled(defaults, 4.0));
    EXPECT_EQ(flat.sim->default_speed_fields().at(flat.room_id).values, scaled(defaults, 4.0));
    EXPECT_LT(time(other), other_initial);
    EXPECT_EQ(time(custom), custom_faster);

    // Without its custom speed field, the stage follows the default again.
    flat.sim->clear_custom_speed_field(custom, flat.room_id);
    EXPECT_FALSE(flat.sim->custom_speed_fields(custom).at(flat.room_id).has_value());
    EXPECT_LT(time(custom), custom_faster);
}

TEST(Simulation, InvalidSpeedFieldsAreRejected)
{
    const auto flat = on_a_flat_room();
    const Polygon strip{std::vector<Point>{{0, 0}, {1, 0}, {1, 20}, {0, 20}}};
    const auto exit = flat.sim->add_stage(ExitDescription{{{strip, flat.room_id}}});
    const auto steering = flat.sim->add_stage(DirectSteeringDescription{});
    const auto defaults = flat.sim->default_speed_fields().at(flat.room_id);
    const auto walkable = std::distance(
        defaults.values.begin(),
        std::find_if(
            defaults.values.begin(), defaults.values.end(), [](double v) { return v > 0.0; }));

    auto too_short = defaults.values;
    too_short.pop_back();
    EXPECT_THROW(flat.sim->set_default_speed_field(flat.room_id, too_short), SimulationError);
    EXPECT_THROW(flat.sim->set_custom_speed_field(exit, flat.room_id, too_short), SimulationError);
    // Which values are allowed is the floor field's to test; one is enough to see its error
    // come through.
    auto zero = defaults.values;
    zero[walkable] = 0.0;
    EXPECT_THROW(flat.sim->set_default_speed_field(flat.room_id, zero), SimulationError);
    EXPECT_THROW(flat.sim->set_custom_speed_field(exit, flat.room_id, zero), SimulationError);
    EXPECT_EQ(flat.sim->default_speed_fields().at(flat.room_id).values, defaults.values);
    EXPECT_FALSE(flat.sim->custom_speed_fields(exit).at(flat.room_id).has_value());

    EXPECT_THROW(flat.sim->grid_info(99), SimulationError);
    EXPECT_THROW(flat.sim->set_default_speed_field(99, defaults.values), SimulationError);
    EXPECT_THROW(flat.sim->set_custom_speed_field(exit, 99, defaults.values), SimulationError);

    // A stage without an area has nothing to route to.
    EXPECT_TRUE(flat.sim->custom_speed_fields(steering).empty());
    EXPECT_THROW(
        flat.sim->set_custom_speed_field(steering, flat.room_id, defaults.values), SimulationError);
    EXPECT_THROW(flat.sim->clear_custom_speed_field(steering, flat.room_id), SimulationError);
}

TEST(MeshBuiltSimulation, IsRejected)
{
    EXPECT_THROW(on_a_mesh_built_switchback_stair(), SimulationError);
    EXPECT_NO_THROW(on_a_flat_room());
}

namespace
{
struct Climb {
    std::vector<double> heights{};
    std::vector<Point> positions{};
};

/// Heights and positions of agent @p id, one per step, until @p done or 6000 steps have passed.
template <typename Done>
Climb climb(Simulation& sim, GenericAgent::ID id, Done done)
{
    Climb climb{};
    for(int step = 0; step < 6000 && !done(); ++step) {
        const auto& agent = sim.agent(id);
        climb.heights.push_back(agent.location.z());
        climb.positions.push_back(agent.location.xy());
        sim.iterate();
    }
    return climb;
}

/// From the ground floor up to the upper one at z = 3, never back down, and without stalling: a
/// phantom wall over or under the agent would show as one that stops making headway.
void expect_climbed_without_stalling(const Climb& climb)
{
    ASSERT_FALSE(climb.heights.empty());
    EXPECT_EQ(climb.heights.front(), 0.0);
    for(std::size_t i = 1; i < climb.heights.size(); ++i) {
        EXPECT_GE(climb.heights[i], climb.heights[i - 1] - 1e-9)
            << "dropped from " << climb.heights[i - 1] << " to " << climb.heights[i] << " at step "
            << i;
    }
    EXPECT_GE(climb.heights.back(), 3.0 - 1e-9);
    for(std::size_t i = 100; i < climb.positions.size(); i += 100) {
        const auto& p = climb.positions[i];
        EXPECT_GT((p - climb.positions[i - 100]).norm(), 0.05)
            << "stalled around " << p.x() << ", " << p.y();
    }
}
} // namespace

TEST(MultiStoreySimulation, WalkingUpTheUStairToTheExitAbove)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};

    // The exit lies on the upper floor, and the agent starts on the ground floor directly below
    // it -- inside its outline in plan. The only way there: east to the stairwell, up both
    // flights, and back west on the upper floor.
    const Polygon outline{{{0, 4}, {3, 4}, {3, 8}, {0, 8}}};
    const auto exit = sim.add_stage(ExitDescription{{{outline, stair.room_2}}});
    const auto route = sim.add_route({{exit, NoRuleDescription{}}});
    const auto id = sim.add_agent(route, exit, Point{2, 6}, State{}, stair.room_1);

    const auto walked = climb(sim, id, [&sim] { return sim.agent_count() == 0; });

    EXPECT_EQ(sim.agent_count(), 0u) << "never made it to the exit";
    // Standing under the exit is not standing in it.
    EXPECT_GT(walked.heights.size(), 1u) << "left through the floor above, on the first step";
    expect_climbed_without_stalling(walked);
}

TEST(MultiStoreySimulation, SteeredUpTheUStairToTheFloorAbove)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};

    // The same way as to the exit above, steered towards a single place.
    const auto steering = sim.add_stage(DirectSteeringDescription{});
    const auto route = sim.add_route({{steering, NoRuleDescription{}}});
    const auto id = sim.add_agent(route, steering, Point{2, 6}, State{}, stair.room_1);
    const auto target = sim.get_location(2, 6, stair.room_2);
    sim.set_agent_target(id, target);

    const auto arrived = [&] {
        const auto& at = sim.agent(id).location;
        return at.region() == stair.room_2 && (at.xy() - target.xy()).norm() < 0.2;
    };
    const auto walked = climb(sim, id, arrived);

    EXPECT_TRUE(arrived()) << "never made it to the target";
    expect_climbed_without_stalling(walked);
}

TEST(SimulationClose, RejectsFurtherUse)
{
    using testing::HasSubstr;
    using testing::ThrowsMessage;
    const auto flat = on_a_flat_room();
    auto& sim = *flat.sim;
    const auto [route, stage] = route_to(sim, Point{15, 15}, flat.room_id);
    const auto id = sim.add_agent(route, stage, Point{5, 5}, State{}, flat.room_id);
    sim.iterate();

    sim.close();

    const auto closed = ThrowsMessage<SimulationError>(HasSubstr("in state Closed"));
    EXPECT_THAT([&] { sim.iterate(); }, closed);
    EXPECT_THAT([&] { sim.agent(id); }, closed);
    EXPECT_THAT([&] { sim.add_agent(route, stage, Point{5, 5}, State{}, flat.room_id); }, closed);
    EXPECT_THAT([&] { route_to(sim, Point{10, 10}, flat.room_id); }, closed);
    // The stage outlives close, its destination's floor field does not.
    EXPECT_THAT([&] { sim.travel_times(stage); }, closed);
    EXPECT_THAT([&] { sim.custom_speed_fields(stage); }, closed);
    EXPECT_THAT([&] { sim.set_custom_speed_field(stage, flat.room_id, {}); }, closed);
    EXPECT_THAT([&] { sim.clear_custom_speed_field(stage, flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.default_speed_fields(); }, closed);
    EXPECT_THAT([&] { sim.set_default_speed_field(flat.room_id, {}); }, closed);
    EXPECT_THAT([&] { sim.grid_info(flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.get_location(5, 5, flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.geo(); }, closed);
}

TEST(Simulation, CloseInformedReassignsRuleAgents)
{
    auto r = a_closure_room();
    const auto route = branch(r);
    const auto first = add_at_waypoint(r, route, 9.7);
    const auto second = add_at_waypoint(r, route, 10.3);
    ASSERT_EQ(r.sim->agent(first).stage_id, r.exit1);
    ASSERT_EQ(r.sim->agent(second).stage_id, r.exit2);
    EXPECT_EQ(r.sim->agent(second).decision_stage, r.wp);

    EXPECT_TRUE(r.sim->close_exit(r.exit2, true).empty());

    const auto& agent = r.sim->agent(second);
    EXPECT_EQ(agent.stage_id, r.exit1);
    EXPECT_EQ(agent.decision_stage, r.wp);
    EXPECT_FALSE(agent.stranded);
    EXPECT_TRUE(std::holds_alternative<Destination>(agent.final_target));
    EXPECT_EQ(r.sim->stage(r.exit2)->count_targeting(), 0u);
    EXPECT_EQ(r.sim->stage(r.exit1)->count_targeting(), 2u);
}

TEST(Simulation, CloseUninformedReassignsAtExit)
{
    auto r = a_closure_room();
    const auto route = branch(r);
    add_at_waypoint(r, route, 9.7);
    const auto id = add_at_waypoint(r, route, 10.3);
    ASSERT_EQ(r.sim->agent(id).stage_id, r.exit2);

    EXPECT_TRUE(r.sim->close_exit(r.exit2, false).empty());
    EXPECT_EQ(r.sim->agent(id).stage_id, r.exit2);

    // Where the agent stood when it was reassigned: the decision comes before the step.
    auto decided_at = r.sim->agent(id).location;
    iterate_until(*r.sim, [&] {
        if(r.sim->agent(id).stage_id != r.exit2) {
            return true;
        }
        decided_at = r.sim->agent(id).location;
        return false;
    });
    const auto& agent = r.sim->agent(id);
    EXPECT_EQ(agent.stage_id, r.exit1);
    EXPECT_FALSE(agent.stranded);
    EXPECT_TRUE(in_area(r, r.exit2, decided_at)) << "reassigned before reaching the exit";
    EXPECT_TRUE(heads_for(r, id, exit1_centre));
    EXPECT_TRUE(r.sim->removed_agents().empty());
}

TEST(Simulation, FixedTransitionStrandedInformed)
{
    auto r = a_closure_room();
    const auto id = add_at_waypoint(r, corridor(r));
    ASSERT_EQ(r.sim->agent(id).stage_id, r.exit2);
    for(int i = 0; i < 20; ++i) {
        r.sim->iterate();
    }
    const auto where = r.sim->agent(id).location.xy();

    EXPECT_EQ(r.sim->close_exit(r.exit2, true), std::vector<GenericAgent::ID>{id});

    const auto& agent = r.sim->agent(id);
    EXPECT_TRUE(agent.stranded);
    EXPECT_EQ(agent.stage_id, r.exit2);
    ASSERT_TRUE(std::holds_alternative<Location>(agent.final_target));
    EXPECT_EQ(std::get<Location>(agent.final_target).xy(), where);
    for(int i = 0; i < 100; ++i) {
        r.sim->iterate();
    }
    EXPECT_NEAR((r.sim->agent(id).location.xy() - where).norm(), 0.0, 1e-2);
}

TEST(Simulation, FixedTransitionStrandedUninformed)
{
    auto r = a_closure_room();
    const auto id = add_at_waypoint(r, corridor(r));

    EXPECT_TRUE(r.sim->close_exit(r.exit2, false).empty());
    EXPECT_FALSE(r.sim->agent(id).stranded);

    iterate_until(*r.sim, [&] { return r.sim->agent(id).stranded; });
    const auto& agent = r.sim->agent(id);
    EXPECT_EQ(agent.stage_id, r.exit2);
    EXPECT_TRUE(in_area(r, r.exit2, stranded_at(r, id))) << "stranded before reaching the exit";
    EXPECT_TRUE(r.sim->removed_agents().empty());
    r.sim->iterate();
    EXPECT_EQ(r.sim->agent_count(), 1u);
}

TEST(Simulation, NoOpenTargetStrandsInsideWaypoint)
{
    auto r = a_closure_room();
    const auto route = branch(r);
    r.sim->close_exit(r.exit1, true);
    r.sim->close_exit(r.exit2, true);

    const auto id = r.sim->add_agent(route, r.wp, Point{11, 10}, State{}, r.room);
    EXPECT_FALSE(r.sim->agent(id).stranded);
    iterate_until(*r.sim, [&] { return r.sim->agent(id).stranded; });

    const auto& agent = r.sim->agent(id);
    EXPECT_EQ(agent.stage_id, r.wp);
    EXPECT_EQ(agent.decision_stage, r.wp);
    EXPECT_TRUE(in_area(r, r.wp, stranded_at(r, id)));
    for(int i = 0; i < 100; ++i) {
        r.sim->iterate();
    }
    EXPECT_TRUE(in_area(r, r.wp, r.sim->agent(id).location));
}

TEST(Simulation, OpenResumesRuleAgentsAtDecisionStage)
{
    auto r = a_closure_room();
    const auto route = branch(r);
    const auto walking = add_at_waypoint(r, route, 9.7);
    ASSERT_EQ(r.sim->agent(walking).stage_id, r.exit1);
    r.sim->close_exit(r.exit2, true);
    EXPECT_EQ(r.sim->close_exit(r.exit1, true), std::vector<GenericAgent::ID>{walking});
    const auto deciding = add_at_waypoint(r, route, 10.3);
    ASSERT_TRUE(r.sim->agent(deciding).stranded);

    const auto resumed = r.sim->open_exit(r.exit1);

    EXPECT_EQ(resumed, (std::vector<GenericAgent::ID>{walking, deciding}));
    for(const auto id : resumed) {
        const auto& agent = r.sim->agent(id);
        EXPECT_FALSE(agent.stranded);
        EXPECT_EQ(agent.stage_id, r.exit1);
        EXPECT_EQ(agent.decision_stage, r.wp);
        EXPECT_TRUE(heads_for(r, id, exit1_centre));
    }
    const auto x_walking = r.sim->agent(walking).location.xy().x();
    const auto x_deciding = r.sim->agent(deciding).location.xy().x();
    for(int i = 0; i < 20; ++i) {
        r.sim->iterate();
    }
    EXPECT_LT(r.sim->agent(walking).location.xy().x(), x_walking);
    EXPECT_LT(r.sim->agent(deciding).location.xy().x(), x_deciding);
}

TEST(Simulation, OpenResumesFixedTransitionAgentsTowardExit)
{
    auto r = a_closure_room();
    const auto id = add_at_waypoint(r, corridor(r));
    r.sim->iterate();
    ASSERT_EQ(r.sim->close_exit(r.exit2, true), std::vector<GenericAgent::ID>{id});
    const auto where = r.sim->agent(id).location.xy();

    EXPECT_EQ(r.sim->open_exit(r.exit2), std::vector<GenericAgent::ID>{id});

    EXPECT_FALSE(r.sim->agent(id).stranded);
    EXPECT_EQ(r.sim->agent(id).stage_id, r.exit2);
    EXPECT_TRUE(heads_for(r, id, exit2_centre));
    for(int i = 0; i < 20; ++i) {
        r.sim->iterate();
    }
    EXPECT_GT(r.sim->agent(id).location.xy().x(), where.x());
}

TEST(Simulation, RepeatedCloseIsNoOp)
{
    auto r = a_closure_room();
    const auto id = add_at_waypoint(r, corridor(r));
    EXPECT_TRUE(r.sim->open_exit(r.exit2).empty());

    ASSERT_EQ(r.sim->close_exit(r.exit2, true), std::vector<GenericAgent::ID>{id});
    EXPECT_TRUE(r.sim->close_exit(r.exit2, true).empty());
    EXPECT_TRUE(r.sim->close_exit(r.exit2, false).empty());
    EXPECT_TRUE(r.sim->stage(r.exit2)->closed());
    EXPECT_TRUE(r.sim->agent(id).stranded);

    EXPECT_EQ(r.sim->open_exit(r.exit2), std::vector<GenericAgent::ID>{id});
    EXPECT_TRUE(r.sim->open_exit(r.exit2).empty());
    EXPECT_FALSE(r.sim->stage(r.exit2)->closed());

    // A second close on an uninformed closure informs nobody: walking agents walk on.
    EXPECT_TRUE(r.sim->close_exit(r.exit2, false).empty());
    EXPECT_TRUE(r.sim->close_exit(r.exit2, true).empty());
    EXPECT_FALSE(r.sim->agent(id).stranded);
    EXPECT_EQ(r.sim->agent(id).stage_id, r.exit2);
    EXPECT_TRUE(heads_for(r, id, exit2_centre));
}

TEST(Simulation, CloseRejectsAStageThatIsNotAnExit)
{
    auto r = a_closure_room();
    EXPECT_THROW(r.sim->close_exit(r.wp, true), SimulationError);
    EXPECT_THROW(r.sim->open_exit(r.wp), SimulationError);
}

TEST(Simulation, CloseInformedSkipsAgentsAlreadyLeaving)
{
    auto r = a_closure_room();
    const auto route = r.sim->add_route({{r.exit2, NoRuleDescription{}}});
    const auto id = r.sim->add_agent(route, r.exit2, Point{17, 10}, State{}, r.room);
    ASSERT_EQ(r.sim->removed_agents().size(), 1u);

    EXPECT_TRUE(r.sim->close_exit(r.exit2, true).empty());
    EXPECT_FALSE(r.sim->agent(id).stranded);
    r.sim->iterate();
    EXPECT_EQ(r.sim->agent_count(), 0u);
}

TEST(Simulation, SwitchToAClosedExitStrandsAtOnce)
{
    auto r = a_closure_room();
    const auto id = add_at_waypoint(r, branch(r));
    const auto to_exit2 = r.sim->add_route({{r.exit2, NoRuleDescription{}}});
    r.sim->close_exit(r.exit2, false);

    r.sim->switch_agent_route(id, to_exit2, r.exit2);

    const auto& agent = r.sim->agent(id);
    EXPECT_TRUE(agent.stranded);
    EXPECT_EQ(agent.decision_stage, r.exit2);
    EXPECT_TRUE(std::holds_alternative<Location>(agent.final_target));
    EXPECT_EQ(r.sim->open_exit(r.exit2), std::vector<GenericAgent::ID>{id});
}

TEST(Simulation, OpenDoesNotReassignReroutedAgents)
{
    auto r = a_closure_room();
    const auto id = add_at_waypoint(r, corridor(r));
    ASSERT_EQ(r.sim->close_exit(r.exit2, true), std::vector<GenericAgent::ID>{id});
    const auto to_exit1 = r.sim->add_route({{r.exit1, NoRuleDescription{}}});
    r.sim->switch_agent_route(id, to_exit1, r.exit1);
    EXPECT_FALSE(r.sim->agent(id).stranded);

    EXPECT_TRUE(r.sim->open_exit(r.exit2).empty());

    EXPECT_EQ(r.sim->agent(id).route_id, to_exit1);
    EXPECT_EQ(r.sim->agent(id).stage_id, r.exit1);
}

TEST(Simulation, RulesSkipOnlyClosedImmediateTargets)
{
    auto r = a_closure_room();
    const auto wp3 = r.sim->add_stage(WaypointDescription{{{square_at({14, 6}), r.room}}});
    const auto route = r.sim->add_route(
        {{r.wp, RoundRobinDescription({{wp3, 1}, {r.exit1, 1}})},
         {wp3, FixedRuleDescription(r.exit2)},
         {r.exit1, NoRuleDescription{}},
         {r.exit2, NoRuleDescription{}}});
    r.sim->close_exit(r.exit2, true);

    // wp3 leads only to the closed exit2, but is open itself, so it can still be chosen.
    const auto id = add_at_waypoint(r, route);
    ASSERT_EQ(r.sim->agent(id).stage_id, wp3);

    iterate_until(*r.sim, [&] { return r.sim->agent(id).stranded; });
    const auto& agent = r.sim->agent(id);
    EXPECT_EQ(agent.stage_id, wp3);
    EXPECT_EQ(agent.decision_stage, wp3);
    EXPECT_TRUE(in_area(r, wp3, stranded_at(r, id)));
}

TEST(Simulation, CloseUninformedWithNoOpenTargetStrandsAtExit)
{
    auto r = a_closure_room();
    const auto route = branch(r);
    const auto first = add_at_waypoint(r, route, 9.7);
    const auto second = add_at_waypoint(r, route, 10.3);
    ASSERT_EQ(r.sim->agent(second).stage_id, r.exit2);
    r.sim->close_exit(r.exit1, true);
    ASSERT_EQ(r.sim->agent(first).stage_id, r.exit2);

    EXPECT_TRUE(r.sim->close_exit(r.exit2, false).empty());

    iterate_until(
        *r.sim, [&] { return r.sim->agent(first).stranded && r.sim->agent(second).stranded; });
    for(const auto id : {first, second}) {
        const auto& agent = r.sim->agent(id);
        EXPECT_EQ(agent.stage_id, r.exit2);
        EXPECT_EQ(agent.decision_stage, r.wp);
        EXPECT_TRUE(in_area(r, r.exit2, stranded_at(r, id)));
    }
    EXPECT_EQ(r.sim->agent_count(), 2u);
}

TEST(Route, SharedCounterSameStep)
{
    auto r = a_closure_room();
    // wp0 lies on the waypoint: agents added in it go on to the waypoint at once and reach it in
    // the same step.
    const auto wp0 = r.sim->add_stage(WaypointDescription{{{square_at({14, 10}), r.room}}});
    const auto route = r.sim->add_route(
        {{wp0, FixedRuleDescription(r.wp)},
         {r.wp, RoundRobinDescription({{r.exit1, 3}, {r.exit2, 2}})},
         {r.exit1, NoRuleDescription{}},
         {r.exit2, NoRuleDescription{}}});
    std::vector<GenericAgent::ID> ids{};
    for(const Point p : {Point{13.7, 9.8}, Point{14.3, 9.8}, Point{14, 10.2}}) {
        ids.push_back(r.sim->add_agent(route, wp0, p, State{}, r.room));
        ASSERT_EQ(r.sim->agent(ids.back()).stage_id, r.wp);
    }

    r.sim->iterate();

    // In agent-id order, consecutive slots of 3:2: exit1, exit2, exit1.
    EXPECT_EQ(r.sim->agent(ids[0]).stage_id, r.exit1);
    EXPECT_EQ(r.sim->agent(ids[1]).stage_id, r.exit2);
    EXPECT_EQ(r.sim->agent(ids[2]).stage_id, r.exit1);
    // The next decision there takes the next slot.
    const auto next = r.sim->add_agent(route, r.wp, Point{13.6, 10.4}, State{}, r.room);
    iterate_until(*r.sim, [&] { return r.sim->agent(next).stage_id != r.wp; });
    EXPECT_EQ(r.sim->agent(next).stage_id, r.exit2);
}

TEST(Simulation, SetRoundRobinWeightsTakesEffectAtTheNextDecision)
{
    auto r = a_closure_room();
    const auto route = branch(r);
    const auto first = add_at_waypoint(r, route, 9.7);
    ASSERT_EQ(r.sim->agent(first).stage_id, r.exit1);

    r.sim->set_round_robin_weights(route, r.wp, {{r.exit1, 0}, {r.exit2, 1}});

    // Already assigned agents keep their target; the next decision uses the new weights.
    EXPECT_EQ(r.sim->agent(first).stage_id, r.exit1);
    const auto second = add_at_waypoint(r, route, 10.3);
    EXPECT_EQ(r.sim->agent(second).stage_id, r.exit2);
    const auto third = r.sim->add_agent(route, r.wp, Point{14.3, 10.0}, State{}, r.room);
    EXPECT_EQ(r.sim->agent(third).stage_id, r.exit2);
}

TEST(Simulation, SetRoundRobinWeightsRejectsOtherRules)
{
    auto r = a_closure_room();
    const auto route = corridor(r);
    EXPECT_THROW(r.sim->set_round_robin_weights(route, r.wp, {{r.exit2, 1}}), SimulationError);
    EXPECT_THROW(
        r.sim->set_round_robin_weights(Route::ID{12345}, r.wp, {{r.exit2, 1}}), SimulationError);
}

TEST(Simulation, OpenResumesOnlyAgentsWhoseRuleTargetsTheExit)
{
    auto r = a_closure_room();
    const auto exit3 = r.sim->add_stage(ExitDescription{{{square_at({10, 18}), r.room}}});
    const auto route = r.sim->add_route(
        {{r.wp, RoundRobinDescription({{r.exit1, 0}, {r.exit2, 1}})},
         {r.exit1, NoRuleDescription{}},
         {r.exit2, NoRuleDescription{}}});
    r.sim->close_exit(r.exit2, true);
    r.sim->close_exit(exit3, true);
    const auto id = add_at_waypoint(r, route);
    ASSERT_TRUE(r.sim->agent(id).stranded);

    // New weights do not resume stranded agents: they decide again when an exit of their rule
    // opens.
    r.sim->set_round_robin_weights(route, r.wp, {{r.exit1, 1}, {r.exit2, 1}});
    EXPECT_TRUE(r.sim->agent(id).stranded);

    // exit3 is no target of the rule at the waypoint, although exit1 would now be open.
    EXPECT_TRUE(r.sim->open_exit(exit3).empty());
    EXPECT_TRUE(r.sim->agent(id).stranded);

    EXPECT_EQ(r.sim->open_exit(r.exit2), std::vector<GenericAgent::ID>{id});
    EXPECT_FALSE(r.sim->agent(id).stranded);
    // Ties go to the earlier target: exit1, with the counter reset by the new weights.
    EXPECT_EQ(r.sim->agent(id).stage_id, r.exit1);
}
