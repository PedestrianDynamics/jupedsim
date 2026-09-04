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

/// What only a whole simulation can be asked: whether a world made of a surface mesh runs, and
/// whether the places written into it land on the storey they were meant on.
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

/// A journey of one waypoint, so that agents have somewhere to be routed to.
std::pair<Journey::ID, BaseStage::ID>
journey_to(Simulation& sim, Point position, double z_hint, double distance = 0.5)
{
    const auto stage = sim.AddStage(WaypointDescription{position, distance}, z_hint);
    const auto journey = sim.AddJourney({{stage, NonTransitionDescription{}}});
    return {journey, stage};
}

} // namespace

TEST(MeshBuiltSimulation, RunsOnASurfaceMesh)
{
    auto sim = on_the_switchback_stair();
    const auto [journey, stage] = journey_to(*sim, Point{16, 6}, 3.0);

    const auto id = sim->AddAgent(journey, stage, Point{2, 2}, State{}, 0.0);
    ASSERT_EQ(sim->AgentCount(), 1u);
    EXPECT_NO_THROW(sim->Iterate());

    // Heading for the stair, which is the only way up: over there in plan, and still on the
    // ground floor.
    const auto& agent = sim->Agent(id);
    EXPECT_GT(agent.nextTarget.x, agent.location.xy().x);
    EXPECT_EQ(agent.location.z(), 0.0);
}

TEST(MeshBuiltSimulation, AnAgentIsPutOnTheStoreyItsHintNames)
{
    auto sim = on_the_switchback_stair();
    const auto [journey, stage] = journey_to(*sim, Point{16, 6}, 3.0);

    // The same (x, y) twice: the ground floor lies at z=0 and the upper floor at z=3, and
    // without the hint there is nothing to tell them apart.
    const auto downstairs = sim->AddAgent(journey, stage, Point{5, 6}, State{}, 0.0);
    const auto upstairs = sim->AddAgent(journey, stage, Point{5, 6}, State{}, 3.0);

    EXPECT_EQ(sim->Agent(downstairs).location.z(), 0.0);
    EXPECT_NEAR(sim->Agent(upstairs).location.z(), 3.0, 1e-9);
    EXPECT_NE(sim->Agent(downstairs).location.region(), sim->Agent(upstairs).location.region());
}

TEST(MeshBuiltSimulation, NoStoreyNearTheHintIsNoPlaceToStand)
{
    auto sim = on_the_switchback_stair();
    const auto [journey, stage] = journey_to(*sim, Point{16, 6}, 3.0);

    // Between the two floors: walkable surface above and below, none within reach of the hint.
    EXPECT_THROW(sim->AddAgent(journey, stage, Point{5, 6}, State{}, 1.5), SimulationError);
}

TEST(MeshBuiltSimulation, AStageIsPutOnTheStoreyItsHintNames)
{
    auto sim = on_the_switchback_stair();
    const auto [up_journey, up_stage] = journey_to(*sim, Point{5, 6}, 3.0);
    const auto [down_journey, down_stage] = journey_to(*sim, Point{5, 6}, 0.0);

    const auto id = sim->AddAgent(up_journey, up_stage, Point{2, 2}, State{}, 0.0);
    sim->Iterate();
    // The waypoint of the journey the agent is on, so its target says which storey the stage
    // was put on.
    EXPECT_NEAR(sim->Agent(id).finalTarget.z(), 3.0, 1e-9);

    sim->SwitchAgentJourney(id, down_journey, down_stage);
    sim->Iterate();
    EXPECT_EQ(sim->Agent(id).finalTarget.z(), 0.0);
}

TEST(MeshBuiltSimulation, ATargetWrittenFromOutsideLandsOnTheAgentsOwnStorey)
{
    auto sim = on_the_switchback_stair();
    const auto [journey, stage] = journey_to(*sim, Point{16, 6}, 3.0);
    const auto upstairs = sim->AddAgent(journey, stage, Point{5, 6}, State{}, 3.0);

    // Only (x, y) is given, and two storeys carry it. It has to mean the one the agent is on --
    // anything else routes it through the wrong floor.
    sim->SetAgentTarget(upstairs, Point{2, 6});
    EXPECT_NEAR(sim->Agent(upstairs).finalTarget.z(), 3.0, 1e-9);

    const auto downstairs = sim->AddAgent(journey, stage, Point{5, 6}, State{}, 0.0);
    sim->SetAgentTarget(downstairs, Point{2, 6});
    EXPECT_EQ(sim->Agent(downstairs).finalTarget.z(), 0.0);
}

TEST(MeshBuiltSimulation, HasNoPolygonToHandOut)
{
    // The geometry itself is handed out either way -- it is the polygon underneath that a mesh
    // world does not have, and that is what the viewer and the systemtests read.
    EXPECT_EQ(on_the_switchback_stair()->Geo().polygon(), nullptr);
    EXPECT_NE(on_a_flat_room()->Geo().polygon(), nullptr);
}

TEST(MeshBuiltSimulation, WalkingUpAStairToTheExitAtTheTop)
{
    auto sim =
        std::make_unique<Simulation>(model(), test_geometries::straight_stair_to_a_landing(), 0.01);

    // Start on the ground floor, exit on the landing three metres up: the whole way there leads
    // over the flight, so arriving at all means the climb worked.
    const Polygon outline{{{17, 2}, {19, 2}, {19, 6}, {17, 6}}};
    const auto exit = sim->AddStage(ExitDescription{outline}, 3.0);
    const auto journey = sim->AddJourney({{exit, NonTransitionDescription{}}});
    const auto id = sim->AddAgent(journey, exit, Point{2, 4}, State{}, 0.0);

    std::vector<double> heights{};
    std::vector<Point> positions{};
    for(int step = 0; step < 4000 && sim->AgentCount() > 0; ++step) {
        heights.push_back(sim->Agent(id).location.z());
        positions.push_back(sim->Agent(id).location.xy());
        sim->Iterate();
    }

    EXPECT_EQ(sim->AgentCount(), 0u) << "never made it to the exit";
    ASSERT_FALSE(heights.empty());

    // Up, and never back down.
    EXPECT_EQ(heights.front(), 0.0);
    for(std::size_t i = 1; i < heights.size(); ++i) {
        EXPECT_GE(heights[i], heights[i - 1] - 1e-9)
            << "dropped from " << heights[i - 1] << " to " << heights[i] << " at step " << i;
    }
    EXPECT_GE(heights.back(), 3.0 - 1e-9);

    // No standing still: a phantom wall under or over the flight would show as an agent that
    // stops making headway without ever arriving.
    for(std::size_t i = 100; i < positions.size(); i += 100) {
        EXPECT_GT((positions[i] - positions[i - 100]).Norm(), 0.05)
            << "stalled around " << positions[i].x << ", " << positions[i].y;
    }
}

TEST(MeshBuiltSimulation, WalkingUpTheUStairToTheExitAbove)
{
    auto sim = on_the_switchback_stair();

    // The exit lies on the upper floor, and the agent starts on the ground floor directly below
    // it -- inside its outline in plan. Four metres apart on paper, some thirty along the only way
    // there: east around the corner at (10, 4), up the flight, over the landing, and back west on
    // the upper floor.
    const Polygon outline{{{0, 4}, {3, 4}, {3, 8}, {0, 8}}};
    const auto exit = sim->AddStage(ExitDescription{outline}, 3.0);
    const auto journey = sim->AddJourney({{exit, NonTransitionDescription{}}});
    const auto id = sim->AddAgent(journey, exit, Point{2, 6}, State{}, 0.0);

    std::vector<double> heights{};
    std::vector<Point> positions{};
    bool past_the_corner = false;
    for(int step = 0; step < 6000 && sim->AgentCount() > 0; ++step) {
        const auto& agent = sim->Agent(id);
        heights.push_back(agent.location.z());
        positions.push_back(agent.location.xy());
        // East of the corner and already climbing: he went around it rather than into it.
        past_the_corner = past_the_corner || (agent.location.xy().x > 10.5 &&
                                              agent.location.z() > 0.0 && agent.location.z() < 3.0);
        sim->Iterate();
    }

    EXPECT_EQ(sim->AgentCount(), 0u) << "never made it to the exit";
    EXPECT_TRUE(past_the_corner) << "never went around the corner at the foot of the flight";
    ASSERT_FALSE(heights.empty());

    // Standing in the exit's outline is not standing in the exit: from the ground floor the way to
    // its centre leads to the storey above, not to the centre.
    EXPECT_EQ(heights.front(), 0.0);
    EXPECT_GT(heights.size(), 1u) << "left through the floor above, on the first step";

    // Up, and never back down.
    for(std::size_t i = 1; i < heights.size(); ++i) {
        EXPECT_GE(heights[i], heights[i - 1] - 1e-9)
            << "dropped from " << heights[i - 1] << " to " << heights[i] << " at step " << i;
    }
    EXPECT_GE(heights.back(), 3.0 - 1e-9);

    // No standing still: a phantom wall over or under him would show as an agent that stops
    // making headway without ever arriving.
    for(std::size_t i = 100; i < positions.size(); i += 100) {
        EXPECT_GT((positions[i] - positions[i - 100]).Norm(), 0.05)
            << "stalled around " << positions[i].x << ", " << positions[i].y;
    }
}
