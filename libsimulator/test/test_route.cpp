// SPDX-License-Identifier: LGPL-3.0-or-later
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/location.hpp"
#include "geometry_fixtures.hpp"
#include "kernel.hpp"
#include "operational_models/collision_free_speed_model_tordeux2016/collision_free_speed_model_tordeux2016.hpp"
#include "polygon.hpp"
#include "records.hpp"
#include "route.hpp"
#include "simulation.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "test_common.hpp"

#include <boost/random/splitmix64.hpp>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

TEST(FixedTransition, NextIsCorrect)
{
    int stage;

    FixedTransition sut(reinterpret_cast<BaseStage*>(&stage));

    for(auto i = 0; i < 20; ++i) {
        ASSERT_EQ(reinterpret_cast<BaseStage*>(&stage), sut.next_stage());
    }
}

TEST(RoundRobinTransition, SimpleNextIsCorrect)
{
    int stage1;
    int stage2;
    int stage3;

    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 1},
        {reinterpret_cast<BaseStage*>(&stage2), 1},
        {reinterpret_cast<BaseStage*>(&stage3), 1}};

    RoundRobinTransition sut(weighted_stages);

    for(auto i = 0; i < 5; ++i) {
        for(auto const& [stage, _] : weighted_stages) {
            ASSERT_EQ(stage, sut.next_stage());
        }
    }
}

TEST(RoundRobinTransition, WeightedRoundRobinInterleaves)
{
    int stage1;
    int stage2;
    int stage3;
    auto* s1 = reinterpret_cast<BaseStage*>(&stage1);
    auto* s2 = reinterpret_cast<BaseStage*>(&stage2);
    auto* s3 = reinterpret_cast<BaseStage*>(&stage3);

    RoundRobinTransition sut({{s1, 1}, {s2, 2}, {s3, 3}});

    const std::vector<BaseStage*> cycle{s3, s2, s1, s3, s2, s3};
    for(auto i = 0; i < 5; ++i) {
        for(auto* stage : cycle) {
            ASSERT_EQ(sut.next_stage(), stage);
        }
    }
}

TEST(RoundRobinTransition, OnlyTheRatioOfTheWeightsMatters)
{
    int stage1;
    int stage2;
    auto* s1 = reinterpret_cast<BaseStage*>(&stage1);
    auto* s2 = reinterpret_cast<BaseStage*>(&stage2);

    RoundRobinTransition small({{s1, 3}, {s2, 2}});
    RoundRobinTransition large({{s1, 60}, {s2, 40}});

    for(auto i = 0; i < 20; ++i) {
        ASSERT_EQ(small.next_stage(), large.next_stage());
    }
}

TEST(RoundRobinTransition, ZeroWeightGivesException)
{
    int stage1;
    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), 0}};

    ASSERT_THROW(RoundRobinTransition sut(weighted_stages), SimulationError);
}

TEST(RoundRobinTransition, TooLargeWeightGivesException)
{
    int stage1;
    std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages = {
        {reinterpret_cast<BaseStage*>(&stage1), RoundRobinTransition::max_weight + 1}};

    ASSERT_THROW(RoundRobinTransition sut(weighted_stages), SimulationError);
}

TEST(LeastTargetedTransition, NextIsCorrect)
{
    const auto room = test_geometries::rectangle({-10, -10}, {10, 10});
    FloorfieldRoutingEngine engine{*room.geometry, FloorfieldOptions{}};
    const auto waypoint = [&] {
        const Polygon area{test_geometries::rectangle_points({-1, -1}, {1, 1})};
        return Waypoint(
            engine.register_destination(room.geometry->clip_to_region(area, room.room_id)), {});
    };
    const auto set_targeting = [](BaseStage& stage, std::size_t count) {
        while(stage.count_targeting() > 0) {
            stage.decrease_targeting();
        }
        for(std::size_t i = 0; i < count; ++i) {
            stage.increase_targeting();
        }
    };
    auto stage1 = waypoint();
    auto stage2 = waypoint();
    auto stage3 = waypoint();

    LeastTargetedTransition sut({&stage1, &stage2, &stage3});

    set_targeting(stage1, 3);
    set_targeting(stage2, 2);
    set_targeting(stage3, 1);
    ASSERT_EQ(&stage3, sut.next_stage());

    set_targeting(stage1, 1);
    set_targeting(stage2, 1);
    set_targeting(stage3, 1);
    ASSERT_EQ(&stage1, sut.next_stage());

    set_targeting(stage1, 5);
    set_targeting(stage2, 1);
    set_targeting(stage3, 5);
    ASSERT_EQ(&stage2, sut.next_stage());
}

namespace
{
using State = CollisionFreeSpeedModelTordeux2016::State;
using testing::HasSubstr;
using testing::ThrowsMessage;

/// A simulation on a flat 20 x 20 room.
class OnAFlatRoom : public ::testing::Test
{
protected:
    test_geometries::SingleRoom _room = test_geometries::rectangle({0, 0}, {20, 20});
    std::size_t _room_id = _room.room_id;
    Simulation _sim{
        std::make_unique<CollisionFreeSpeedModelTordeux2016>(8.0, 0.1, 5.0, 0.02),
        std::move(_room.geometry),
        0.01,
        FloorfieldOptions{}};

    Location at(Point xy) const { return _sim.get_location(xy.x(), xy.y(), _room_id); }

    /// A 1 x 1 square around @p center.
    StageArea square(Point center) const
    {
        const Vector half{0.5, 0.5};
        return {Polygon{test_geometries::rectangle_points(center - half, center + half)}, _room_id};
    }

    BaseStage::ID waypoint(Point center, std::optional<std::string> name = {})
    {
        return _sim.add_stage(WaypointDescription{{square(center)}}, std::move(name));
    }

    BaseStage::ID exit(Point center) { return _sim.add_stage(ExitDescription{{square(center)}}); }

    static RouteTransitionDescription fixed(std::optional<BaseStage::ID> source, BaseStage::ID next)
    {
        return {source, FixedTransitionDescription{next}};
    }

    GenericAgent::ID
    add_agent(Point xy, Route::ID route, std::optional<BaseStage::ID> first_stage = {})
    {
        return _sim.add_agent(at(xy), route, first_stage, State{});
    }
};

using AddRoute = OnAFlatRoom;

TEST_F(AddRoute, EmptyIsRejected)
{
    EXPECT_THAT(
        [&] { _sim.add_route({}); }, ThrowsMessage<SimulationError>(HasSubstr("route is empty")));
}

TEST_F(AddRoute, WithoutATransitionFromTheAgentLocationIsRejected)
{
    const auto wp1 = waypoint({5, 5});
    const auto wp2 = waypoint({10, 10});
    EXPECT_THAT(
        [&] { _sim.add_route({fixed(wp1, wp2)}); },
        ThrowsMessage<SimulationError>(HasSubstr(
            "No transition leaves AGENT_LOCATION; add one, e.g. "
            "jps.FixedTransition(jps.AGENT_LOCATION, ...).")));
}

TEST_F(AddRoute, WithTwoTransitionsFromTheAgentLocationIsRejected)
{
    const auto wp1 = waypoint({5, 5});
    const auto wp2 = waypoint({10, 10});
    EXPECT_THAT(
        [&] { _sim.add_route({fixed({}, wp1), fixed({}, wp2)}); },
        ThrowsMessage<SimulationError>(HasSubstr("2 transitions leave AGENT_LOCATION")));
}

TEST_F(AddRoute, WithTwoTransitionsFromOneStageIsRejected)
{
    const auto wp1 = waypoint({5, 5}, "wp1");
    const auto wp2 = waypoint({10, 10});
    const auto wp3 = waypoint({15, 15});
    EXPECT_THAT(
        [&] { _sim.add_route({fixed({}, wp1), fixed(wp1, wp2), fixed(wp1, wp3)}); },
        ThrowsMessage<SimulationError>(
            HasSubstr(fmt::format("Target {} (wp1) has 2 outgoing transitions", wp1))));
}

TEST_F(AddRoute, WithATransitionLeavingAnExitIsRejected)
{
    const auto exit1 = _sim.add_stage(ExitDescription{{square({2, 2})}}, "exit1");
    const auto wp1 = waypoint({10, 10});
    EXPECT_THAT(
        [&] { _sim.add_route({fixed({}, exit1), fixed(exit1, wp1)}); },
        ThrowsMessage<SimulationError>(
            HasSubstr(fmt::format("Target {} (exit1) is an exit", exit1))));
}

TEST_F(AddRoute, WithAStageFollowingItselfIsRejected)
{
    const auto wp1 = waypoint({5, 5}, "wp1");
    const auto wp2 = waypoint({10, 10});
    EXPECT_THAT(
        [&] { _sim.add_route({fixed({}, wp1), fixed(wp1, wp1)}); },
        ThrowsMessage<SimulationError>(HasSubstr(
            fmt::format("Target {} (wp1) cannot follow itself; remove the self-transition", wp1))));
    EXPECT_THAT(
        [&] {
            _sim.add_route(
                {fixed({}, wp1), {wp1, RoundRobinTransitionDescription{{{wp1, 1}, {wp2, 1}}}}});
        },
        ThrowsMessage<SimulationError>(HasSubstr("remove it from the targets")));
    EXPECT_THAT(
        [&] {
            _sim.add_route({fixed({}, wp1), {wp1, LeastTargetedTransitionDescription{{wp1, wp2}}}});
        },
        ThrowsMessage<SimulationError>(HasSubstr("remove it from the targets")));
}

TEST_F(AddRoute, WithAPartOfItsOwnIsRejected)
{
    const auto wp1 = waypoint({5, 5});
    const auto wp2 = waypoint({10, 10}, "wp2");
    const auto wp3 = waypoint({15, 15});
    EXPECT_THAT(
        [&] { _sim.add_route({fixed({}, wp1), fixed(wp2, wp3)}); },
        ThrowsMessage<SimulationError>(HasSubstr(
            fmt::format(
                "Targets {} (wp2), {} are not connected to the rest of the route", wp2, wp3))));
}

TEST_F(AddRoute, WithASideEntryOrALoopIsAccepted)
{
    const auto wp1 = waypoint({5, 5});
    const auto wp2 = waypoint({10, 10});
    const auto wp3 = waypoint({15, 15});
    // wp3 cannot be reached from the agent's location, but leads into the route.
    EXPECT_NO_THROW(_sim.add_route({fixed({}, wp1), fixed(wp1, wp2), fixed(wp3, wp2)}));
    EXPECT_NO_THROW(_sim.add_route({fixed({}, wp1), fixed(wp1, wp2), fixed(wp2, wp1)}));
}

TEST_F(AddRoute, WithANameInUseIsRejected)
{
    const auto wp1 = waypoint({5, 5});
    _sim.add_route({fixed({}, wp1)}, "evacuation");
    EXPECT_THAT(
        [&] { _sim.add_route({fixed({}, wp1)}, "evacuation"); },
        ThrowsMessage<SimulationError>(HasSubstr("'evacuation' is already used")));
}

using FollowRoute = OnAFlatRoom;

TEST_F(FollowRoute, TheFirstTransitionChoosesTheFirstStage)
{
    const auto wp1 = waypoint({5, 15});
    const auto wp2 = waypoint({15, 15});
    const auto route =
        _sim.add_route({{std::nullopt, RoundRobinTransitionDescription{{{wp1, 2}, {wp2, 1}}}}});

    std::vector<BaseStage::ID> first_stages{};
    for(const double x : {2.0, 4.0, 6.0, 8.0}) {
        first_stages.push_back(_sim.agent(add_agent({x, 2}, route)).stage_id);
    }
    EXPECT_EQ(first_stages, (std::vector<BaseStage::ID>{wp1, wp2, wp1, wp1}));
}

TEST_F(FollowRoute, AGivenFirstStageTakesNoTurnInTheFirstTransition)
{
    const auto wp1 = waypoint({5, 15});
    const auto wp2 = waypoint({15, 15});
    const auto route =
        _sim.add_route({{std::nullopt, RoundRobinTransitionDescription{{{wp1, 1}, {wp2, 1}}}}});

    EXPECT_EQ(_sim.agent(add_agent({2, 2}, route, wp2)).stage_id, wp2);
    EXPECT_EQ(_sim.agent(add_agent({4, 2}, route)).stage_id, wp1);
}

TEST_F(FollowRoute, AFirstStageOutsideTheRouteIsRejected)
{
    const auto wp1 = waypoint({5, 15}, "wp1");
    const auto wp2 = waypoint({15, 15}, "wp2");
    const auto route = _sim.add_route({fixed({}, wp1)}, "to wp1");
    EXPECT_THAT(
        [&] { add_agent({2, 2}, route, wp2); },
        ThrowsMessage<SimulationError>(HasSubstr(
            fmt::format("Target {} (wp2) is not part of route {} (to wp1).", wp2, route))));
    EXPECT_EQ(_sim.agent_count(), 0u);
}

TEST_F(FollowRoute, AnAgentAddedInsideItsFirstStageHasReachedIt)
{
    const auto wp1 = waypoint({2, 2});
    const auto wp2 = waypoint({15, 15});
    const auto route = _sim.add_route({fixed({}, wp1), fixed(wp1, wp2)});
    const auto id = add_agent({2, 2}, route);

    ASSERT_EQ(_sim.reached_targets().size(), 1u);
    const auto& reached = _sim.reached_targets().front();
    EXPECT_EQ(reached.agent_id, id);
    EXPECT_EQ(std::get<BaseStage::ID>(reached.target), wp1);
    EXPECT_EQ(reached.time, 0.0);
    EXPECT_EQ(reached.location.xy(), Point(2, 2));
    EXPECT_EQ(_sim.agent(id).stage_id, wp1) << "moves on in the next iteration";

    _sim.iterate();
    EXPECT_TRUE(_sim.reached_targets().empty());
    EXPECT_EQ(_sim.agent(id).stage_id, wp2);
}

TEST_F(FollowRoute, TheLastStageIsReachedOnce)
{
    const auto wp1 = waypoint({2, 2});
    const auto route = _sim.add_route({fixed({}, wp1)});
    const auto id = add_agent({2, 2}, route);
    EXPECT_EQ(_sim.reached_targets().size(), 1u);

    _sim.iterate();
    EXPECT_TRUE(_sim.reached_targets().empty());
    _sim.iterate();
    EXPECT_TRUE(_sim.reached_targets().empty());
    EXPECT_EQ(_sim.agent(id).stage_id, wp1);
}

TEST_F(FollowRoute, AStageTheAgentStandsInAlreadyIsReachedInTheNextStep)
{
    const auto wp1 = waypoint({2, 2});
    const auto wp2 = waypoint({2, 2});
    const auto route = _sim.add_route({fixed({}, wp1), fixed(wp1, wp2), fixed(wp2, wp1)});
    add_agent({2, 2}, route);

    for(const auto expected : {wp2, wp1, wp2}) {
        _sim.iterate();
        ASSERT_EQ(_sim.reached_targets().size(), 1u);
        EXPECT_EQ(std::get<BaseStage::ID>(_sim.reached_targets().front().target), expected);
    }
}

TEST_F(FollowRoute, AnAgentReachingAnExitLeavesAtTheBeginningOfTheNextIteration)
{
    const auto exit1 = exit({4, 2});
    const auto id = add_agent({2, 2}, _sim.add_route({fixed({}, exit1)}));

    while(_sim.reached_targets().empty()) {
        ASSERT_LT(_sim.iteration(), 1000u);
        _sim.iterate();
    }
    const auto reached_at = _sim.reached_targets().front().time;
    EXPECT_TRUE(_sim.agent(id).leaving);
    EXPECT_TRUE(_sim.removed_agents().empty());

    _sim.iterate();
    EXPECT_EQ(_sim.agent_count(), 0u);
    ASSERT_EQ(_sim.removed_agents().size(), 1u);
    const auto& removal = _sim.removed_agents().front();
    EXPECT_EQ(removal.agent_id, id);
    EXPECT_EQ(removal.exit, exit1);
    EXPECT_EQ(removal.time, reached_at);
}

TEST_F(FollowRoute, AMarkedAgentIsRecordedWhenItIsRemoved)
{
    const auto route = _sim.add_route({fixed({}, waypoint({15, 15}))});
    const auto id = add_agent({2, 2}, route);
    _sim.iterate();

    _sim.mark_agent_for_removal(id);
    _sim.mark_agent_for_removal(id);
    EXPECT_TRUE(_sim.removed_agents().empty());

    _sim.iterate();
    EXPECT_EQ(_sim.agent_count(), 0u);
    ASSERT_EQ(_sim.removed_agents().size(), 1u);
    EXPECT_EQ(_sim.removed_agents().front().agent_id, id);
    EXPECT_EQ(_sim.removed_agents().front().exit, std::nullopt);
    EXPECT_EQ(_sim.removed_agents().front().time, _sim.dt());
}

TEST_F(FollowRoute, AnAgentMarkedAfterReachingAnExitIsRecordedOnce)
{
    const auto exit1 = exit({2, 2});
    const auto id = add_agent({2, 2}, _sim.add_route({fixed({}, exit1)}));

    _sim.mark_agent_for_removal(id);
    _sim.iterate();
    EXPECT_EQ(_sim.agent_count(), 0u);
    ASSERT_EQ(_sim.removed_agents().size(), 1u);
    EXPECT_EQ(_sim.removed_agents().front().exit, exit1);
}

TEST_F(FollowRoute, ALeavingAgentCannotBeRerouted)
{
    const auto wp1 = waypoint({15, 15});
    const auto to_wp1 = _sim.add_route({fixed({}, wp1)});
    const auto exited = add_agent({2, 2}, _sim.add_route({fixed({}, exit({2, 2}))}));
    const auto marked = add_agent({8, 8}, to_wp1);
    _sim.mark_agent_for_removal(marked);

    const auto leaves = ThrowsMessage<SimulationError>(HasSubstr("leaves the simulation"));
    EXPECT_THAT([&] { _sim.reroute(exited, to_wp1); }, leaves);
    EXPECT_THAT([&] { _sim.reroute(marked, wp1); }, leaves);
    EXPECT_THAT([&] { _sim.reroute(marked, at({10, 10})); }, leaves);
}

TEST_F(FollowRoute, AnAgentNameIsFreeAgainOnceItsAgentHasLeft)
{
    const auto route = _sim.add_route({fixed({}, exit({2, 2}))});
    const auto id = _sim.add_agent(at({2, 2}), route, {}, State{}, "anna");
    EXPECT_EQ(_sim.agent_named("anna"), id) << "still there, about to leave";
    EXPECT_THAT(
        [&] { _sim.add_agent(at({5, 5}), route, {}, State{}, "anna"); },
        ThrowsMessage<SimulationError>(HasSubstr("'anna' is already used")));

    _sim.iterate();
    EXPECT_EQ(_sim.agent_named("anna"), std::nullopt);
    EXPECT_NO_THROW(_sim.add_agent(at({5, 5}), route, {}, State{}, "anna"));
}

TEST_F(FollowRoute, RerouteWithinTheRouteJumpsToTheStage)
{
    const auto wp1 = waypoint({2, 2});
    const auto wp2 = waypoint({15, 15});
    const auto wp3 = waypoint({15, 5});
    const auto route = _sim.add_route({fixed({}, wp1), fixed(wp1, wp2), fixed(wp2, wp3)});
    const auto id = add_agent({2, 2}, route);
    _sim.iterate();
    ASSERT_EQ(_sim.agent(id).stage_id, wp2);

    _sim.reroute(id, wp1);
    EXPECT_EQ(_sim.agent(id).stage_id, wp1);
    _sim.iterate();
    EXPECT_EQ(_sim.reached_targets().size(), 1u) << "reached again after the step";
    EXPECT_EQ(_sim.agent(id).stage_id, wp1);
    _sim.iterate();
    EXPECT_EQ(_sim.agent(id).stage_id, wp2);
}

TEST_F(FollowRoute, ARerouteIsReachedAfterTheNextStepEvenInsideTheTarget)
{
    const auto id = add_agent({2, 2}, _sim.add_route({fixed({}, waypoint({2, 2}))}));
    _sim.iterate();
    const auto wp2 = waypoint({2, 2});

    _sim.reroute(id, _sim.add_route({fixed({}, wp2)}));
    EXPECT_TRUE(_sim.reached_targets().empty());
    _sim.iterate();
    ASSERT_EQ(_sim.reached_targets().size(), 1u);
    EXPECT_EQ(std::get<BaseStage::ID>(_sim.reached_targets().front().target), wp2);
}

TEST_F(FollowRoute, ASteeredAgentHasNoRouteToJumpIn)
{
    const auto wp1 = waypoint({2, 2});
    const auto id = _sim.add_agent(at({2, 2}), at({15, 15}), State{});
    EXPECT_THAT(
        [&] { _sim.reroute(id, wp1); },
        ThrowsMessage<SimulationError>(HasSubstr("The agent has no route")));
}

TEST_F(FollowRoute, ASteeredAgentArrivesOnce)
{
    const auto target = at({3, 2});
    const auto id = _sim.add_agent(at({2, 2}), target, State{});

    std::size_t arrivals = 0;
    for(int step = 0; step < 300; ++step) {
        _sim.iterate();
        for(const auto& reached : _sim.reached_targets()) {
            EXPECT_EQ(reached.agent_id, id);
            EXPECT_EQ(std::get<Location>(reached.target).xy(), target.xy());
            ++arrivals;
        }
    }
    EXPECT_EQ(arrivals, 1u);
    EXPECT_LT((_sim.agent(id).location.xy() - target.xy()).norm(), 0.1);
}

TEST_F(FollowRoute, ALocationOfAnotherSimulationIsRejected)
{
    const auto other = test_geometries::rectangle({0, 0}, {20, 20});
    const auto elsewhere = other.geometry->get_location(2, 2, other.room_id);
    const auto route = _sim.add_route({fixed({}, waypoint({15, 15}))});
    EXPECT_THROW(_sim.add_agent(elsewhere, route, {}, State{}), SimulationError);
    EXPECT_THROW(_sim.add_agent(at({2, 2}), elsewhere, State{}), SimulationError);
}
} // namespace

using RouteFormatter = OnAFlatRoom;

TEST_F(RouteFormatter, ShowsIdNameAndTargets)
{
    const auto wp1 = waypoint({5, 5});
    const auto wp2 = waypoint({10, 10});
    const auto named = _sim.add_route({fixed({}, wp1), fixed(wp1, wp2)}, "r");
    const auto unnamed = _sim.add_route({fixed({}, wp1)});

    const Simulation& sim = _sim;
    EXPECT_EQ(
        fmt::format("{}", sim.route(named)),
        fmt::format("Route(id={}, name=r, targets=[{}, {}])", named, wp1, wp2));
    EXPECT_EQ(
        fmt::format("{}", sim.route(unnamed)),
        fmt::format("Route(id={}, targets=[{}])", unnamed, wp1));
}

using RecordFormatter = OnAFlatRoom;

TEST_F(RecordFormatter, ShowsAllFields)
{
    const auto exit1 = exit({5, 5});
    const auto location = at({2, 2});

    EXPECT_EQ(
        fmt::format("{}", TargetReachedRecord{7, exit1, 1.5, location}),
        fmt::format(
            "TargetReachedRecord(agent_id=7, target={}, time=1.5, location={})", exit1, location));
    EXPECT_EQ(
        fmt::format("{}", RemovalRecord{7, exit1, 1.5}),
        fmt::format("RemovalRecord(agent_id=7, exit={}, time=1.5)", exit1));
    EXPECT_EQ(
        fmt::format("{}", RemovalRecord{7, std::nullopt, 1.5}),
        "RemovalRecord(agent_id=7, time=1.5)");
}

namespace
{
/// Simulation::agent() finds agents by binary search, so the agents have to stay sorted by id
/// whatever the simulation does with them. This test tries to guard that against any future
/// change, e.g. to the agent container or the order agents are stepped in: a random mix of
/// operations, from a fixed seed, on agents whose spatial order has nothing to do with their ids.
/// After every operation, every agent ever added has to be found exactly while it is in the
/// simulation.
class AgentLookup : public OnAFlatRoom, public ::testing::WithParamInterface<std::uint64_t>
{
protected:
    /// The raw draws of a generator with a fixed algorithm, unlike std distributions, are the
    /// same on every platform, so a seed reproduces a failure everywhere.
    boost::random::splitmix64 _rng{GetParam()};
    std::vector<GenericAgent::ID> _added{};
    std::set<GenericAgent::ID> _present{};
    std::set<BaseStage::ID> _exits{};
    /// Agents that reached an exit or were marked for removal; they leave at the beginning of the
    /// next iteration.
    std::set<GenericAgent::ID> _leaving{};

    std::size_t draw(std::size_t count) { return _rng() % count; }

    void add_agents(const std::vector<Route::ID>& routes)
    {
        for(std::size_t i = draw(3) + 1; i > 0; --i) {
            const Point xy{static_cast<double>(draw(19) + 1), static_cast<double>(draw(19) + 1)};
            if(!_sim.agents_in_range(at(xy), 1.0).empty()) {
                continue;
            }
            const auto id = add_agent(xy, routes[draw(routes.size())]);
            _added.push_back(id);
            _present.insert(id);
        }
        note_reached_exits();
    }

    void note_reached_exits()
    {
        for(const auto& reached : _sim.reached_targets()) {
            const auto* stage = std::get_if<BaseStage::ID>(&reached.target);
            if(stage != nullptr && _exits.contains(*stage)) {
                _leaving.insert(reached.agent_id);
            }
        }
    }

    GenericAgent::ID any_present_agent()
    {
        return *std::next(_present.begin(), static_cast<std::ptrdiff_t>(draw(_present.size())));
    }

    void iterate()
    {
        for(const auto id : _leaving) {
            _present.erase(id);
        }
        _leaving.clear();
        _sim.iterate();
        note_reached_exits();
    }

    ::testing::AssertionResult finds_exactly_the_present_agents()
    {
        auto& agents = _sim.agents();
        if(!std::is_sorted(
               agents.begin(), agents.end(), [](const GenericAgent& a, const GenericAgent& b) {
                   return a.id < b.id;
               })) {
            return ::testing::AssertionFailure()
                   << "The agents are not sorted by id. Run this test in a Debug build: an assert "
                      "in Simulation stops at the first operation that breaks the order.";
        }
        const Simulation& sim = _sim;
        for(const auto id : _added) {
            bool found = false;
            try {
                found = sim.agent(id).id == id;
            } catch(const SimulationError&) {
            }
            if(found != _present.contains(id)) {
                return ::testing::AssertionFailure()
                       << "Agent " << id.get_id()
                       << (found ? " is found after it left." : " is not found.");
            }
        }
        if(sim.agent_count() != _present.size()) {
            return ::testing::AssertionFailure() << "The simulation holds " << sim.agent_count()
                                                 << " agents, expected " << _present.size() << ".";
        }
        return ::testing::AssertionSuccess();
    }
};
} // namespace

TEST_P(AgentLookup, FindsExactlyTheAgentsInTheSimulation)
{
    // Large exits, so that agents leave throughout the run.
    const auto wide_exit = [this](Point lower, Point upper) {
        return _sim.add_stage(
            ExitDescription{
                {{Polygon{test_geometries::rectangle_points(lower, upper)}, _room_id}}});
    };
    const auto exit1 = wide_exit({1, 15}, {5, 19});
    const auto exit2 = wide_exit({15, 1}, {19, 5});
    _exits = {exit1, exit2};
    const auto wp1 = waypoint({10, 10});
    const std::vector<Route::ID> routes{
        _sim.add_route({fixed({}, exit1)}),
        _sim.add_route({fixed({}, exit2)}),
        _sim.add_route({fixed({}, wp1), fixed(wp1, exit1)}),
        _sim.add_route({fixed({}, waypoint({18, 18}))})};

    for(int operation = 0; operation < 200; ++operation) {
        switch(draw(10)) {
            case 0:
            case 1:
            case 2:
                add_agents(routes);
                break;
            case 3:
                if(!_present.empty()) {
                    const auto id = any_present_agent();
                    _sim.mark_agent_for_removal(id);
                    _leaving.insert(id);
                }
                break;
            case 4:
                if(!_present.empty()) {
                    const auto id = any_present_agent();
                    const auto route = routes[draw(routes.size())];
                    if(_leaving.contains(id)) {
                        EXPECT_THROW(_sim.reroute(id, route), SimulationError);
                    } else {
                        _sim.reroute(id, route);
                    }
                }
                break;
            default:
                for(std::size_t step = draw(5) + 1; step > 0; --step) {
                    iterate();
                }
        }
        ASSERT_TRUE(finds_exactly_the_present_agents())
            << "seed " << GetParam() << ", operation " << operation;
    }
}

INSTANTIATE_TEST_SUITE_P(Seeds, AgentLookup, ::testing::Values(1, 2, 3, 4, 42));
