// SPDX-License-Identifier: LGPL-3.0-or-later
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "geometry_fixtures.hpp"
#include "journey.hpp"
#include "kernel.hpp"
#include "mesh_fixtures.hpp"
#include "operational_models/agent_rng.hpp"
#include "operational_models/anticipation_velocity_model/anticipation_velocity_model.hpp"
#include "operational_models/collision_free_speed_model/collision_free_speed_model.hpp"
#include "polygon.hpp"
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
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
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

/// A journey of one waypoint, so that agents have somewhere to be routed to.
std::pair<Journey::ID, BaseStage::ID>
journey_to(Simulation& sim, Point position, std::size_t region_id)
{
    const Vector half{0.5, 0.5};
    const Polygon area{test_geometries::rectangle_points(position - half, position + half)};
    const auto stage = sim.add_stage(WaypointDescription{{{area, region_id}}});
    const auto journey = sim.add_journey({{stage, NonTransitionDescription{}}});
    return {journey, stage};
}

/// A simulation whose model gives every agent a random number stream, ready to take agents.
struct SeededRoom {
    static constexpr std::uint64_t seed = 1234;
    using AgentState = AnticipationVelocityModel::State;

    test_geometries::SingleRoom room = test_geometries::rectangle({0, 0}, {20, 20});
    std::size_t room_id = room.room_id;
    Simulation sim{
        std::make_unique<AnticipationVelocityModel>(0.3, seed),
        std::move(room.geometry),
        0.01,
        FloorfieldOptions{}};
    std::pair<Journey::ID, BaseStage::ID> route = journey_to(sim, Point{18, 18}, room_id);

    GenericAgent::ID add(Point position, AgentState state = {})
    {
        return sim.add_agent(route.first, route.second, position, state, room_id);
    }

    /// Whether agent `id` got the stream of the `index`-th agent taking one from the sequence.
    bool has_stream(GenericAgent::ID id, std::uint64_t index) const
    {
        AgentRngSeeder sequence{seed};
        for(std::uint64_t skipped = 0; skipped < index; ++skipped) {
            sequence.next();
        }
        return std::get<AgentState>(sim.agent(id).state).rng == sequence.next();
    }
};

} // namespace

TEST(MultiStoreySimulation, RunsOnAMultiStoreySurface)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [journey, stage] = journey_to(sim, stacked_point, stair.room_2);

    const auto id = sim.add_agent(journey, stage, Point{2, 2}, State{}, stair.room_1);
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
    const auto [journey, stage] = journey_to(sim, stacked_point, stair.room_2);

    const auto downstairs = sim.add_agent(journey, stage, stacked_point, State{}, stair.room_1);
    const auto upstairs = sim.add_agent(journey, stage, stacked_point, State{}, stair.room_2);

    EXPECT_EQ(sim.agent(downstairs).location.region(), stair.room_1);
    EXPECT_EQ(sim.agent(upstairs).location.region(), stair.room_2);
    EXPECT_NEAR(sim.agent(upstairs).location.z(), 3.0, 1e-9);
}

TEST(MultiStoreySimulation, AStageIsPutInTheRegionItNames)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [up_journey, up_stage] = journey_to(sim, stacked_point, stair.room_2);
    const auto [down_journey, down_stage] = journey_to(sim, stacked_point, stair.room_1);

    // Upstairs, the waypoint is only reached over the stair to the east.
    const auto id = sim.add_agent(up_journey, up_stage, Point{2, 2}, State{}, stair.room_1);
    sim.iterate();
    EXPECT_GT(sim.agent(id).route_orientation.x(), 0.5);

    // Downstairs, it is in plain sight. Floor field directions follow a gradient on a grid, so
    // straight ahead holds only up to the grid.
    sim.switch_agent_journey(id, down_journey, down_stage);
    sim.iterate();
    const Vector straight = (stacked_point - sim.agent(id).location.xy()).normalized();
    EXPECT_NEAR(sim.agent(id).route_orientation.x(), straight.x(), 1e-2);
    EXPECT_NEAR(sim.agent(id).route_orientation.y(), straight.y(), 1e-2);
}

TEST(MultiStoreySimulation, ATargetWrittenFromOutsideLandsOnTheAgentsOwnStorey)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01, FloorfieldOptions{}};
    const auto [journey, stage] = journey_to(sim, stacked_point, stair.room_2);
    const auto upstairs = sim.add_agent(journey, stage, stacked_point, State{}, stair.room_2);

    // Only (x, y) is given, and two storeys carry it. It has to mean the one the agent is on --
    // anything else routes it through the wrong floor.
    sim.set_agent_target(upstairs, Point{2, 6});
    EXPECT_EQ(std::get<Location>(sim.agent(upstairs).final_target).region(), stair.room_2);

    const auto downstairs = sim.add_agent(journey, stage, stacked_point, State{}, stair.room_1);
    sim.set_agent_target(downstairs, Point{2, 6});
    EXPECT_EQ(std::get<Location>(sim.agent(downstairs).final_target).region(), stair.room_1);
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
    EXPECT_TRUE(flat.sim->has_custom_speed_field(custom, flat.room_id));
    EXPECT_FALSE(flat.sim->custom_speed_fields(other).at(flat.room_id).has_value());
    EXPECT_FALSE(flat.sim->has_custom_speed_field(other, flat.room_id));
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
    EXPECT_FALSE(flat.sim->has_custom_speed_field(custom, flat.room_id));
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
    EXPECT_THROW(flat.sim->has_custom_speed_field(exit, 99), SimulationError);

    // A stage without an area has nothing to route to.
    EXPECT_TRUE(flat.sim->custom_speed_fields(steering).empty());
    EXPECT_FALSE(flat.sim->has_custom_speed_field(steering, flat.room_id));
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
    const auto journey = sim.add_journey({{exit, NonTransitionDescription{}}});
    const auto id = sim.add_agent(journey, exit, Point{2, 6}, State{}, stair.room_1);

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
    const auto journey = sim.add_journey({{steering, NonTransitionDescription{}}});
    const auto id = sim.add_agent(journey, steering, Point{2, 6}, State{}, stair.room_1);
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
    const auto [journey, stage] = journey_to(sim, Point{15, 15}, flat.room_id);
    const auto id = sim.add_agent(journey, stage, Point{5, 5}, State{}, flat.room_id);
    sim.iterate();

    sim.close();

    const auto closed = ThrowsMessage<SimulationError>(HasSubstr("in state Closed"));
    EXPECT_THAT([&] { sim.iterate(); }, closed);
    EXPECT_THAT([&] { sim.agent(id); }, closed);
    EXPECT_THAT([&] { sim.add_agent(journey, stage, Point{5, 5}, State{}, flat.room_id); }, closed);
    EXPECT_THAT([&] { journey_to(sim, Point{10, 10}, flat.room_id); }, closed);
    // The stage outlives close, its destination's floor field does not.
    EXPECT_THAT([&] { sim.travel_times(stage); }, closed);
    EXPECT_THAT([&] { sim.custom_speed_fields(stage); }, closed);
    EXPECT_THAT([&] { sim.has_custom_speed_field(stage, flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.set_custom_speed_field(stage, flat.room_id, {}); }, closed);
    EXPECT_THAT([&] { sim.clear_custom_speed_field(stage, flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.default_speed_fields(); }, closed);
    EXPECT_THAT([&] { sim.set_default_speed_field(flat.room_id, {}); }, closed);
    EXPECT_THAT([&] { sim.grid_info(flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.get_location(5, 5, flat.room_id); }, closed);
    EXPECT_THAT([&] { sim.geo(); }, closed);
}

TEST(AgentRngSeeding, AgentsAddedFromTheSameStateGetTheirOwnStreams)
{
    SeededRoom world;
    const SeededRoom::AgentState shared{};
    const auto first = world.add({2, 2}, shared);
    const auto second = world.add({6, 2}, shared);
    EXPECT_TRUE(world.has_stream(first, 0));
    EXPECT_TRUE(world.has_stream(second, 1));
}

TEST(AgentRngSeeding, ARejectedAgentUsesUpNoStream)
{
    SeededRoom world;
    SeededRoom::AgentState invalid{};
    invalid.radius = -1.0;
    EXPECT_THROW(world.add({2, 2}, invalid), SimulationError);
    EXPECT_TRUE(world.has_stream(world.add({2, 2}), 0));
}

TEST(AgentRngSeeding, ARemovedAgentsStreamIsNotHandedOutAgain)
{
    SeededRoom world;
    const auto removed = world.add({2, 2});
    world.add({6, 2});
    world.sim.mark_agent_for_removal(removed);
    world.sim.iterate();
    ASSERT_EQ(world.sim.agent_count(), 1u);
    EXPECT_TRUE(world.has_stream(world.add({10, 2}), 2));
}
