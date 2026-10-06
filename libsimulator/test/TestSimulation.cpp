// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry.hpp"
#include "GeometryFixtures.hpp"
#include "Journey.hpp"
#include "OperationalModels/CollisionFreeSpeedModel/CollisionFreeSpeedModel.hpp"
#include "Polygon.hpp"
#include "Simulation.hpp"
#include "SimulationError.hpp"
#include "StageDescription.hpp"
#include "TestCommon.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

/// What only a whole simulation can be asked: whether a multi-storey world runs, and whether the
/// places written into it land on the storey they were meant on.
namespace
{
using State = CollisionFreeSpeedModel::State;

std::unique_ptr<CollisionFreeSpeedModel> model()
{
    return std::make_unique<CollisionFreeSpeedModel>(8.0, 0.1, 5.0, 0.02);
}

/// The switchback stair: ground floor, a flight climbing away from it, a landing, and the upper
/// floor turning back over the ground floor. Over (5, 6) there are two storeys.
std::unique_ptr<Simulation> on_the_switchback_stair()
{
    return std::make_unique<Simulation>(model(), test_geometries::switchback_stair(), 0.01);
}

std::unique_ptr<Simulation> on_a_flat_room()
{
    return std::make_unique<Simulation>(
        model(), test_geometries::rectangle({0, 0}, {20, 20}), 0.01);
}

/// Both floors of the U-stair carry this (x, y).
const Point stacked_point{2, 10};

/// A journey of one waypoint, so that agents have somewhere to be routed to.
std::pair<Journey::ID, BaseStage::ID>
journey_to(Simulation& sim, Point position, std::size_t region_id, double distance = 0.5)
{
    const auto stage = sim.AddStage(WaypointDescription{position, distance, region_id});
    const auto journey = sim.AddJourney({{stage, NonTransitionDescription{}}});
    return {journey, stage};
}

} // namespace

TEST(MultiStoreySimulation, RunsOnAMultiStoreySurface)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01};
    const auto [journey, stage] = journey_to(sim, stacked_point, stair.upper);

    const auto id = sim.AddAgent(journey, stage, Point{2, 2}, State{}, stair.ground);
    ASSERT_EQ(sim.AgentCount(), 1u);
    EXPECT_NO_THROW(sim.Iterate());

    // Heading for the stair, which is the only way up: over there in plan, and still on the
    // ground floor.
    const auto& agent = sim.Agent(id);
    EXPECT_GT(agent.routeOrientation.x, 0.0);
    EXPECT_EQ(agent.location.z(), 0.0);
}

TEST(MultiStoreySimulation, AnAgentIsPutInTheRegionItNames)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01};
    const auto [journey, stage] = journey_to(sim, stacked_point, stair.upper);

    const auto downstairs = sim.AddAgent(journey, stage, stacked_point, State{}, stair.ground);
    const auto upstairs = sim.AddAgent(journey, stage, stacked_point, State{}, stair.upper);

    EXPECT_EQ(sim.Agent(downstairs).location.region(), stair.ground);
    EXPECT_EQ(sim.Agent(upstairs).location.region(), stair.upper);
    EXPECT_NEAR(sim.Agent(upstairs).location.z(), 3.0, 1e-9);
}

TEST(MultiStoreySimulation, AStageIsPutInTheRegionItNames)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01};
    const auto [up_journey, up_stage] = journey_to(sim, stacked_point, stair.upper);
    const auto [down_journey, down_stage] = journey_to(sim, stacked_point, stair.ground);

    const auto id = sim.AddAgent(up_journey, up_stage, Point{2, 2}, State{}, stair.ground);
    sim.Iterate();
    // The waypoint of the journey the agent is on, so its target says which storey the stage
    // was put on.
    EXPECT_EQ(sim.Agent(id).finalTarget.region(), stair.upper);

    sim.SwitchAgentJourney(id, down_journey, down_stage);
    sim.Iterate();
    EXPECT_EQ(sim.Agent(id).finalTarget.region(), stair.ground);
}

TEST(MultiStoreySimulation, ATargetWrittenFromOutsideLandsOnTheAgentsOwnStorey)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01};
    const auto [journey, stage] = journey_to(sim, stacked_point, stair.upper);
    const auto upstairs = sim.AddAgent(journey, stage, stacked_point, State{}, stair.upper);

    // Only (x, y) is given, and two storeys carry it. It has to mean the one the agent is on --
    // anything else routes it through the wrong floor.
    sim.SetAgentTarget(upstairs, Point{2, 6});
    EXPECT_EQ(sim.Agent(upstairs).finalTarget.region(), stair.upper);

    const auto downstairs = sim.AddAgent(journey, stage, stacked_point, State{}, stair.ground);
    sim.SetAgentTarget(downstairs, Point{2, 6});
    EXPECT_EQ(sim.Agent(downstairs).finalTarget.region(), stair.ground);
}

TEST(Simulation, AWaypointNeedsAPositiveDistance)
{
    auto sim = on_a_flat_room();
    EXPECT_THROW(sim->AddStage(WaypointDescription{{5, 5}, 0.0, 0}), SimulationError);
}

TEST(MeshBuiltSimulation, IsRejected)
{
    EXPECT_THROW(on_the_switchback_stair(), SimulationError);
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
        const auto& agent = sim.Agent(id);
        climb.heights.push_back(agent.location.z());
        climb.positions.push_back(agent.location.xy());
        sim.Iterate();
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
        EXPECT_GT((p - climb.positions[i - 100]).Norm(), 0.05)
            << "stalled around " << p.x << ", " << p.y;
    }
}
} // namespace

TEST(MultiStoreySimulation, WalkingUpTheUStairToTheExitAbove)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01};

    // The exit lies on the upper floor, and the agent starts on the ground floor directly below
    // it -- inside its outline in plan. The only way there: east to the stairwell, up both
    // flights, and back west on the upper floor.
    const Polygon outline{{{0, 4}, {3, 4}, {3, 8}, {0, 8}}};
    const auto exit = sim.AddStage(ExitDescription{outline, stair.upper});
    const auto journey = sim.AddJourney({{exit, NonTransitionDescription{}}});
    const auto id = sim.AddAgent(journey, exit, Point{2, 6}, State{}, stair.ground);

    const auto walked = climb(sim, id, [&sim] { return sim.AgentCount() == 0; });

    EXPECT_EQ(sim.AgentCount(), 0u) << "never made it to the exit";
    // Standing in the exit's outline is not standing in the exit: from the ground floor the way to
    // its centre leads to the storey above, not to the centre.
    EXPECT_GT(walked.heights.size(), 1u) << "left through the floor above, on the first step";
    expect_climbed_without_stalling(walked);
}

TEST(MultiStoreySimulation, SteeredUpTheUStairToTheFloorAbove)
{
    auto stair = test_geometries::u_stair();
    Simulation sim{model(), std::move(stair.geometry), 0.01};

    // The same way as to the exit above, steered towards a single place.
    const auto steering = sim.AddStage(DirectSteeringDescription{});
    const auto journey = sim.AddJourney({{steering, NonTransitionDescription{}}});
    const auto id = sim.AddAgent(journey, steering, Point{2, 6}, State{}, stair.ground);
    const auto target = sim.GetLocation(2, 6, stair.upper);
    sim.SetAgentTarget(id, target);

    const auto arrived = [&] {
        const auto& at = sim.Agent(id).location;
        return at.region() == stair.upper && (at.xy() - target.xy()).Norm() < 0.2;
    };
    const auto walked = climb(sim, id, arrived);

    EXPECT_TRUE(arrived()) << "never made it to the target";
    expect_climbed_without_stalling(walked);
}
