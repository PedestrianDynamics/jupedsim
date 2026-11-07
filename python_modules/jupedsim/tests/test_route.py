# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import numpy as np
import pytest
import shapely
from jupedsim.pyjupedsim import Rule as NativeRule
from jupedsim.route import (
    STAY,
    LeastTargeted,
    LeastTravelTime,
    RoundRobin,
    Route,
)
from jupedsim.surface import WalkableSurface

EXIT_AREAS = {
    "exit1": [(9, 1), (10, 1), (10, 3), (9, 3)],
    "exit2": [(9, 7), (10, 7), (10, 9), (9, 9)],
}


def _simulation():
    surface = WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    )
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
    return sim, room


@pytest.fixture
def stages():
    sim, room = _simulation()
    names = ("wp1", "wp2", "wp3", "wp4")
    wps = {
        name: sim.add_waypoint((1 + i, 5), radius=0.4, region=room, name=name)
        for i, name in enumerate(names)
    }
    exits = {
        name: sim.add_exit(area, region=room, name=name)
        for name, area in EXIT_AREAS.items()
    }
    return sim, wps | exits


def test_linear_equals_pairs(stages):
    _, s = stages
    linear = Route([s["wp1"], s["wp2"], s["exit1"]])
    pairs = Route(
        [(s["wp1"], s["wp2"]), (s["wp2"], s["exit1"])], start=s["wp1"]
    )
    assert linear.stages == pairs.stages
    assert linear.start == pairs.start == s["wp1"]
    for stage in linear.stages:
        assert linear.next(stage) == pairs.next(stage)
    assert linear.next(s["wp1"]) == s["wp2"]
    assert linear.next(s["exit1"]) is None


def test_single_exit_route(stages):
    _, s = stages
    route = Route([s["exit1"]])
    assert route.stages == (s["exit1"],)
    assert route.start == s["exit1"]


def test_pairs_have_no_default_start(stages):
    _, s = stages
    route = Route([(s["wp1"], s["exit1"])])
    assert route.start is None


def route_c(s, **kwargs):
    return Route(
        [
            (s["wp1"], s["wp2"]),
            (s["wp2"], RoundRobin({s["wp3"]: 1, s["exit1"]: 2, s["exit2"]: 1})),
            (s["wp3"], s["exit2"]),
        ],
        start=s["wp1"],
        **kwargs,
    )


def test_inspection(stages):
    _, s = stages
    route = route_c(s)
    assert route.stages == (
        s["wp1"],
        s["wp2"],
        s["wp3"],
        s["exit1"],
        s["exit2"],
    )
    assert route.next(s["wp2"]) == RoundRobin(
        {s["wp3"]: 1, s["exit1"]: 2, s["exit2"]: 1}
    )
    weights = route.weights(s["wp2"])
    assert list(weights.values()) == [1, 2, 1]
    weights.clear()
    assert len(route.weights(s["wp2"])) == 3


def test_inspection_errors(stages):
    _, s = stages
    route = route_c(s)
    with pytest.raises(
        ValueError, match="wp1 has a fixed transition; use route.next"
    ):
        route.weights(s["wp1"])
    with pytest.raises(
        ValueError,
        match="wp4 is not part of this route; stages are "
        "wp1, wp2, wp3, exit1, exit2",
    ):
        route.next(s["wp4"])


def test_repr(stages):
    _, s = stages
    assert repr(route_c(s)) == (
        "Route(start=wp1)\n"
        "  wp1 -> wp2\n"
        "  wp2 -> RoundRobin(wp3: 1, exit1: 2, exit2: 1)\n"
        "  wp3 -> exit2"
    )


def test_loops(stages):
    _, s = stages
    patrol = Route([s["wp1"], s["wp2"], s["wp3"], s["wp1"]], endless=True)
    assert patrol.next(s["wp3"]) == s["wp1"]
    assert patrol.stages == (s["wp1"], s["wp2"], s["wp3"])
    looped = Route(
        [
            (s["wp1"], s["wp2"]),
            (s["wp2"], RoundRobin({s["wp1"]: 1, s["exit1"]: 1})),
        ]
    )
    assert looped.stages == (s["wp1"], s["wp2"], s["exit1"])
    assert looped.next(s["wp2"]) == RoundRobin({s["wp1"]: 1, s["exit1"]: 1})


def test_endless_pairs(stages):
    _, s = stages
    tour = Route(
        [
            (s["wp1"], s["wp2"]),
            (s["wp2"], RoundRobin({s["wp3"]: 1, s["wp4"]: 1})),
            (s["wp3"], s["wp1"]),
            (s["wp4"], s["wp1"]),
        ],
        start=s["wp1"],
        endless=True,
    )
    assert tour.stages == (s["wp1"], s["wp2"], s["wp3"], s["wp4"])


def test_stay_alone_is_rejected(stages):
    for steps in ([STAY], [STAY, STAY]):
        with pytest.raises(ValueError, match="STAY must"):
            Route(steps)


class _Recorder:
    """Stands in for the native rule class and records how rules are made."""

    @staticmethod
    def create_fixed_rule(stage_id):
        return ("fixed", stage_id)

    @staticmethod
    def create_round_robin_rule(pairs):
        return ("round_robin", list(pairs))

    @staticmethod
    def create_least_targeted_rule(ids):
        return ("least_targeted", list(ids))

    @staticmethod
    def create_least_travel_time_rule(ids):
        return ("least_travel_time", list(ids))

    @staticmethod
    def create_none_rule():
        return ("none",)


def test_native_rules_follow_the_route(stages, monkeypatch):
    import jupedsim.route as route_module

    _, s = stages
    route = Route(
        [
            (s["wp1"], s["wp2"]),
            (s["wp2"], RoundRobin({s["wp3"]: 1, s["exit1"]: 2})),
            (s["wp3"], LeastTargeted([s["exit1"], s["exit2"]])),
        ],
        start=s["wp1"],
    )
    monkeypatch.setattr(route_module, "_NativeRule", _Recorder)
    expected = {
        s["wp1"].id: _Recorder.create_fixed_rule(s["wp2"].id),
        s["wp2"].id: _Recorder.create_round_robin_rule(
            [(s["wp3"].id, 1), (s["exit1"].id, 2)]
        ),
        s["wp3"].id: _Recorder.create_least_targeted_rule(
            [s["exit1"].id, s["exit2"].id]
        ),
        s["exit1"].id: _Recorder.create_none_rule(),
        s["exit2"].id: _Recorder.create_none_rule(),
    }
    built = {
        st.id: route_module._native_rule(st, route._transitions)
        for st in route.stages
    }
    assert built == expected


def test_least_travel_time_native_rule(stages, monkeypatch):
    import jupedsim.route as route_module

    _, s = stages
    route = Route(
        [(s["wp1"], LeastTravelTime([s["exit2"], s["exit1"]]))], start=s["wp1"]
    )
    monkeypatch.setattr(route_module, "_NativeRule", _Recorder)
    built = route_module._native_rule(s["wp1"], route._transitions)
    assert built == ("least_travel_time", [s["exit2"].id, s["exit1"].id])


def test_every_rule_kind_is_one_native_type():
    rules = [
        NativeRule.create_fixed_rule(1),
        NativeRule.create_round_robin_rule([(1, 1), (2, 3)]),
        NativeRule.create_least_targeted_rule([1, 2]),
        NativeRule.create_least_travel_time_rule([1, 2]),
        NativeRule.create_none_rule(),
    ]
    assert all(type(rule) is NativeRule for rule in rules)
    assert NativeRule.__module__ == "jupedsim.pyjupedsim"
    assert "Describes the rule at a stage" in NativeRule.__doc__
    assert "Create a fixed rule" in NativeRule.create_fixed_rule.__doc__


@pytest.mark.parametrize(
    "create",
    [
        lambda: NativeRule.create_fixed_rule(0),
        lambda: NativeRule.create_round_robin_rule([(0, 1)]),
        lambda: NativeRule.create_least_targeted_rule([0]),
        lambda: NativeRule.create_least_travel_time_rule([0]),
    ],
)
def test_a_rule_to_the_invalid_stage_id_is_rejected(create):
    with pytest.raises(jps.SimulationError, match="invalid stage id"):
        create()


def test_a_route_with_every_rule_kind_leads_to_the_exit():
    sim, room = _simulation()
    first = sim.add_waypoint((2, 5), radius=0.5, region=room)
    upper = sim.add_waypoint((5, 7), radius=0.5, region=room)
    lower = sim.add_waypoint((5, 3), radius=0.5, region=room)
    last = sim.add_waypoint((8, 5), radius=0.5, region=room)
    exit = sim.add_exit([(9, 4), (10, 4), (10, 6), (9, 6)], region=room)
    route = Route(
        [
            (first, RoundRobin({upper: 1, lower: 1})),
            (upper, LeastTargeted([last])),
            (lower, LeastTargeted([last])),
            (last, exit),
        ],
        start=first,
    )
    for y in (4.0, 6.0):
        sim.add_agent(
            room.at(1.0, y),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        )
    while sim.agent_count() > 0 and sim.iteration_count() < 5000:
        sim.iterate()
    assert sim.agent_count() == 0


def test_least_targeted_rule(stages):
    _, s = stages
    route = Route(
        [(s["wp1"], LeastTargeted([s["exit1"], s["exit2"]]))], start=s["wp1"]
    )
    assert route.next(s["wp1"]) == LeastTargeted([s["exit1"], s["exit2"]])
    with pytest.raises(ValueError, match="uses LeastTargeted, which has no"):
        route.weights(s["wp1"])


def test_route_bound_at_construction(stages):
    sim, s = stages
    route = Route([s["wp1"], s["exit1"]])
    assert route._simulation is sim
    assert isinstance(route._native_id, int)
    other_sim, _ = _simulation()
    route._check_simulation(sim)
    with pytest.raises(ValueError, match="route belongs to another simulation"):
        route._check_simulation(other_sim)
    # update_weights works before any agent is added and applies to the
    # first agent added afterwards; without it that agent would go to exit1.
    sim, room, decision, exits = _decision_room()
    route = _split(decision, exits, 1, 1)
    route.update_weights(decision, {exits["exit1"]: 0})
    agent = sim.add_agent(room.at(5, 5), route=route)
    assert agent.stage == exits["exit2"]
    # hold works before any agent is added and applies to the first agent
    # added afterwards; without it that agent would go to exit1.
    sim, room, decision, exits = _decision_room()
    route = _split(decision, exits, 1, 1)
    route.hold(decision)
    agent = sim.add_agent(room.at(5, 5), route=route)
    assert agent.held
    assert agent.stage == decision


def test_each_route_gets_its_own_native_route(stages):
    _, s = stages
    first = Route([s["wp1"], s["exit1"]])
    second = Route([s["wp1"], s["exit1"]])
    assert first._native_id != second._native_id


def test_empty_route():
    with pytest.raises(ValueError, match="^route is empty$"):
        Route([])


def test_no_next_stage(stages):
    _, s = stages
    with pytest.raises(
        ValueError,
        match=r"wp3 has no next stage; end with an exit or add \(wp3, jps.STAY\)",
    ):
        Route([(s["wp1"], s["wp2"]), (s["wp2"], s["wp3"])])


def test_two_outgoing_transitions(stages):
    _, s = stages
    with pytest.raises(
        ValueError,
        match="wp2 has 2 outgoing transitions; combine them into one rule",
    ):
        Route(
            [
                (s["wp1"], s["wp2"]),
                (s["wp2"], s["wp3"]),
                (s["wp2"], RoundRobin({s["exit1"]: 2, s["exit2"]: 1})),
            ]
        )


def test_loop_without_exit(stages):
    _, s = stages
    with pytest.raises(
        ValueError,
        match="no exit or STAY reachable; to loop on purpose, use endless=True",
    ):
        Route([(s["wp1"], s["wp2"]), (s["wp2"], s["wp1"])])


def test_exit_unreachable_through_zero_weight(stages):
    _, s = stages
    with pytest.raises(ValueError, match="no exit or STAY reachable"):
        Route(
            [
                (s["wp1"], RoundRobin({s["wp2"]: 1, s["exit1"]: 0})),
                (s["wp2"], s["wp1"]),
            ]
        )


def test_endless_with_exit(stages):
    _, s = stages
    with pytest.raises(
        ValueError, match="route can end at exit1; drop endless=True"
    ):
        Route([s["wp1"], s["wp2"], s["exit1"]], endless=True)


def test_stage_twice(stages):
    _, s = stages
    with pytest.raises(
        ValueError,
        match="wp1 appears twice; only repeating the first stage at the end "
        "closes a loop",
    ):
        Route([s["wp1"], s["wp2"], s["wp1"], s["exit1"]])


def test_exit_not_terminal(stages):
    _, s = stages
    with pytest.raises(
        ValueError, match=r"exits are terminal; remove \(exit1, wp1\)"
    ):
        Route([s["exit1"], s["wp1"]])


def test_point_used_as_stage(stages):
    _, s = stages
    with pytest.raises(
        TypeError,
        match=r"\(15, 6.5\) is not a stage; use sim.add_waypoint\(\(15, 6.5\), "
        r"\.\.\.\)",
    ):
        Route([(s["wp1"], (15, 6.5)), ((15, 6.5), s["exit1"])], start=s["wp1"])


def test_stages_of_two_simulations(stages):
    _, s = stages
    other, room = _simulation()
    foreign = other.add_waypoint((5, 5), radius=0.4, region=room)
    with pytest.raises(
        ValueError, match="all stages must belong to the same simulation"
    ):
        Route([s["wp1"], foreign, s["exit1"]])


def test_start_must_be_in_route(stages):
    _, s = stages
    with pytest.raises(ValueError, match="wp4 is not part of this route"):
        Route([s["wp1"], s["exit1"]], start=s["wp4"])


def test_non_integer_weights(stages):
    _, s = stages
    with pytest.raises(ValueError, match="^weights must be integers$"):
        RoundRobin({s["exit1"]: 0.6, s["exit2"]: 0.4})


def test_numpy_integer_weights(stages):
    _, s = stages
    rule = RoundRobin({s["exit1"]: np.int64(3), s["exit2"]: np.uint8(2)})
    assert rule.weights == {s["exit1"]: 3, s["exit2"]: 2}
    assert all(type(w) is int for w in rule.weights.values())


def test_bool_weights_are_rejected(stages):
    _, s = stages
    with pytest.raises(ValueError, match="^weights must be integers$"):
        RoundRobin({s["exit1"]: True, s["exit2"]: 1})


def test_all_zero_weights(stages):
    _, s = stages
    with pytest.raises(ValueError, match="^at least one weight must be > 0$"):
        RoundRobin({s["exit1"]: 0, s["exit2"]: 0})


def test_stay_is_accepted(stages):
    _, s = stages
    route = Route([s["wp1"], STAY])
    assert route.stages == (s["wp1"],)


def test_zero_weights_and_least_travel_time_are_accepted(stages):
    _, s = stages
    Route([(s["wp1"], LeastTravelTime([s["exit1"], s["exit2"]]))])
    route = Route(
        [(s["wp1"], RoundRobin({s["exit1"]: 0, s["exit2"]: 1}))],
        start=s["wp1"],
    )
    assert route.weights(s["wp1"]) == {s["exit1"]: 0, s["exit2"]: 1}


def _decision_room(exit_areas=EXIT_AREAS):
    """A 10 x 10 room with a decision waypoint of radius 1 at (5, 5)."""
    sim, room = _simulation()
    decision = sim.add_waypoint(
        (5, 5), radius=1.0, region=room, name="decision"
    )
    exits = {
        name: sim.add_exit(area, region=room, name=name)
        for name, area in exit_areas.items()
    }
    return sim, room, decision, exits


def _split(decision, exits, w1, w2):
    rule = RoundRobin({exits["exit1"]: w1, exits["exit2"]: w2})
    return Route([(decision, rule)], start=decision)


# Spots inside the decision waypoint, far enough apart for the model.
SPOTS = [(4.5, 5), (5.5, 5), (5, 4.5), (5, 5.5), (5, 5)]


def _decide(sim, room, route, spot):
    """Add an agent in the decision waypoint; it decides at once."""
    return sim.add_agent(room.at(*SPOTS[spot]), route=route).stage


def test_update_weights_patch(stages):
    _, s = stages
    route = route_c(s)
    route.update_weights(s["wp2"], {s["wp3"]: 0})
    assert route.weights(s["wp2"]) == {
        s["wp3"]: 0,
        s["exit1"]: 2,
        s["exit2"]: 1,
    }
    assert list(route.weights(s["wp2"])) == [s["wp3"], s["exit1"], s["exit2"]]
    assert route.next(s["wp2"]) == RoundRobin(
        {s["wp3"]: 0, s["exit1"]: 2, s["exit2"]: 1}
    )


def test_update_weights_keeps_rules_returned_earlier(stages):
    _, s = stages
    route = route_c(s)
    before = route.next(s["wp2"])
    route.update_weights(s["wp2"], {s["wp3"]: 0})
    assert before.weights == {s["wp3"]: 1, s["exit1"]: 2, s["exit2"]: 1}


def test_update_weights_takes_effect_at_the_next_decision():
    sim, room, decision, exits = _decision_room()
    route = _split(decision, exits, 2, 1)
    first = sim.add_agent(room.at(*SPOTS[0]), route=route)
    assert first.stage == exits["exit1"]
    route.update_weights(decision, {exits["exit1"]: 1, exits["exit2"]: 2})
    # Assigned agents keep their target; the counter starts afresh: 1:2
    # gives exit2, exit1, exit2.
    assert first.stage == exits["exit1"]
    decided = [_decide(sim, room, route, i) for i in (1, 2, 3)]
    assert decided == [exits["exit2"], exits["exit1"], exits["exit2"]]


def test_update_weights_with_unchanged_weights_keeps_the_counter():
    sim, room, decision, exits = _decision_room()
    route = _split(decision, exits, 2, 1)
    # 2:1 gives exit1, exit2, exit1.
    assert _decide(sim, room, route, 0) == exits["exit1"]
    route.update_weights(decision, {exits["exit1"]: 2, exits["exit2"]: 1})
    route.update_weights(decision, {exits["exit2"]: 1})
    # A reset counter would give exit1 again.
    assert _decide(sim, room, route, 1) == exits["exit2"]
    assert _decide(sim, room, route, 2) == exits["exit1"]


def test_reapplying_the_weights_every_iteration_lets_agents_leave():
    sim, room, decision, exits = _decision_room()
    back = sim.add_waypoint((2, 5), radius=0.5, region=room, name="back")
    weights = {back: 2, exits["exit1"]: 1}
    route = Route(
        [(decision, RoundRobin(weights)), (back, decision)], start=decision
    )
    agent = sim.add_agent(room.at(5, 5), route=route)
    for _ in range(3000):
        if not agent.alive:
            break
        route.update_weights(decision, weights)
        sim.iterate()
    assert not agent.alive


def test_update_weights_invalid_keeps_state():
    sim, room, decision, exits = _decision_room()
    route = _split(decision, exits, 2, 1)
    # 2:1 gives exit1, exit2, exit1.
    assert _decide(sim, room, route, 0) == exits["exit1"]
    with pytest.raises(ValueError):
        route.update_weights(decision, {exits["exit1"]: 0, exits["exit2"]: 0})
    with pytest.raises(ValueError):
        route.update_weights(decision, {exits["exit1"]: 1.5})
    assert route.weights(decision) == {exits["exit1"]: 2, exits["exit2"]: 1}
    # A reset counter would give exit1 again.
    assert _decide(sim, room, route, 1) == exits["exit2"]
    assert _decide(sim, room, route, 2) == exits["exit1"]


def test_update_weights_cycle_rejected():
    sim, room, decision, exits = _decision_room()
    back = sim.add_waypoint((2, 5), radius=0.5, region=room, name="back")
    route_g = Route(
        [
            (decision, RoundRobin({back: 1, exits["exit1"]: 1})),
            (back, decision),
        ],
        start=decision,
    )
    assert _decide(sim, room, route_g, 0) == back
    with pytest.raises(
        ValueError,
        match="^with these weights no exit or STAY is reachable from "
        "decision, back; weights unchanged$",
    ):
        route_g.update_weights(decision, {exits["exit1"]: 0})
    assert route_g.weights(decision) == {back: 1, exits["exit1"]: 1}
    # The counter is unchanged too: the next agent takes the next slot.
    assert _decide(sim, room, route_g, 1) == exits["exit1"]


def test_update_weights_on_an_endless_route(stages):
    _, s = stages
    tour = Route(
        [
            (s["wp1"], RoundRobin({s["wp2"]: 1, s["wp3"]: 1})),
            (s["wp2"], s["wp1"]),
            (s["wp3"], s["wp1"]),
        ],
        start=s["wp1"],
        endless=True,
    )
    tour.update_weights(s["wp1"], {s["wp3"]: 0})
    assert tour.weights(s["wp1"]) == {s["wp2"]: 1, s["wp3"]: 0}


def test_update_weights_errors(stages):
    _, s = stages
    route = route_c(s)
    with pytest.raises(
        ValueError, match="^wp1 has a fixed transition, not a weighted rule$"
    ):
        route.update_weights(s["wp1"], {s["wp2"]: 3})
    route_n = Route(
        [(s["wp1"], LeastTravelTime([s["exit1"], s["exit2"]]))], start=s["wp1"]
    )
    with pytest.raises(
        ValueError, match="^wp1 uses LeastTravelTime, which has no weights$"
    ):
        route_n.update_weights(s["wp1"], {s["exit1"]: 2})
    with pytest.raises(
        ValueError,
        match=r"^wp4 is not a target of wp2; targets are wp3, exit1, exit2 "
        r"\(see route.weights\(wp2\)\)$",
    ):
        route.update_weights(s["wp2"], {s["wp4"]: 1})
    with pytest.raises(
        ValueError,
        match=r"^at least one target at wp2 needs weight > 0; to stop agents, "
        r"use route.hold\(wp2\)$",
    ):
        route.update_weights(
            s["wp2"], {s["wp3"]: 0, s["exit1"]: 0, s["exit2"]: 0}
        )
    assert route.weights(s["wp2"]) == {
        s["wp3"]: 1,
        s["exit1"]: 2,
        s["exit2"]: 1,
    }


def test_update_weights_errors_reuse_the_weights_wording(stages):
    _, s = stages
    route = route_c(s)
    with pytest.raises(
        ValueError, match="^exit1 is an exit; it has no weights$"
    ):
        route.update_weights(s["exit1"], {s["exit2"]: 1})
    route_t = Route(
        [(s["wp1"], LeastTargeted([s["exit1"], s["exit2"]]))], start=s["wp1"]
    )
    with pytest.raises(
        ValueError, match="^wp1 uses LeastTargeted, which has no weights$"
    ):
        route_t.update_weights(s["wp1"], {s["exit1"]: 2})
    with pytest.raises(ValueError, match="^wp4 is not part of this route"):
        route.update_weights(s["wp4"], {s["exit1"]: 2})
    with pytest.raises(ValueError, match="^weights must be integers$"):
        route.update_weights(s["wp2"], {s["exit1"]: 0.5})
    with pytest.raises(ValueError, match="^weights must be >= 0$"):
        route.update_weights(s["wp2"], {s["exit1"]: -1})


def test_counters_per_route():
    sim, room, decision, exits = _decision_room()
    first = _split(decision, exits, 1, 1)
    second = _split(decision, exits, 1, 1)
    assert _decide(sim, room, first, 0) == exits["exit1"]
    assert _decide(sim, room, second, 1) == exits["exit1"]
    assert _decide(sim, room, first, 2) == exits["exit2"]
    assert _decide(sim, room, second, 3) == exits["exit2"]


def test_update_weights_does_not_resume_stranded_agents():
    areas = EXIT_AREAS | {"exit3": [(4, 9), (6, 9), (6, 10), (4, 10)]}
    sim, room, decision, exits = _decision_room(areas)
    route = _split(decision, exits, 0, 1)
    exits["exit2"].close(informed=True)
    exits["exit3"].close(informed=True)
    agent = sim.add_agent(room.at(5, 5), route=route)
    assert agent.stranded
    route.update_weights(decision, {exits["exit1"]: 1})
    assert agent.stranded
    # exit3 is no target at the decision waypoint: the agent waits for an
    # exit of its own rule, although exit1 would now be chosen.
    assert exits["exit3"].open() == ()
    assert agent.stranded
    assert exits["exit2"].open() == (agent,)
    assert not agent.stranded
    assert agent.stage == exits["exit1"]


def test_least_travel_time_picks_nearest():
    # exit1 lies to the lower right of the decision waypoint, exit2 to the
    # upper right.
    sim, room, decision, exits = _decision_room()
    route = Route(
        [(decision, LeastTravelTime([exits["exit1"], exits["exit2"]]))],
        start=decision,
    )
    lower = sim.add_agent(room.at(5, 4.4), route=route)
    upper = sim.add_agent(room.at(5, 5.6), route=route)
    assert lower.stage == exits["exit1"]
    assert upper.stage == exits["exit2"]


def test_least_travel_time_leads_agents_out():
    sim, room, decision, exits = _decision_room()
    route = Route(
        [(decision, LeastTravelTime([exits["exit1"], exits["exit2"]]))],
        start=decision,
    )
    for spot in SPOTS[:4]:
        sim.add_agent(room.at(*spot), route=route)
    while sim.agent_count() > 0 and sim.iteration_count() < 5000:
        sim.iterate()
    assert sim.agent_count() == 0


def _unreachable(sim, monkeypatch, stage):
    """Make ``stage`` unreachable for the reachability check.

    The floor field rejects geometries with unreachable walkable cells at
    construction, so no geometry gives an infinite travel time; the query is
    replaced instead.
    """
    real = sim.travel_time

    def travel_time(target, location):
        if target == stage:
            return float("inf")
        return real(target, location)

    monkeypatch.setattr(sim, "travel_time", travel_time)


def test_add_agent_rejects_an_unreachable_first_target(stages, monkeypatch):
    sim, s = stages
    route = Route([s["wp1"], s["exit1"]])
    _unreachable(sim, monkeypatch, s["wp1"])
    location = sim._surface.region(id=0).at(5, 2)
    with pytest.raises(
        ValueError,
        match=r"^wp1 cannot be reached from Location\(5(\.0)?, 2(\.0)?, "
        r"region=.*\)$",
    ):
        sim.add_agent(location, route=route)
    assert sim.agent_count() == 0
    # A start that can be reached is fine.
    sim.add_agent(location, route=route, start=s["exit1"])
    assert sim.agent_count() == 1


def test_follow_rejects_an_unreachable_first_target(stages, monkeypatch):
    sim, s = stages
    route = Route([s["wp1"], s["exit1"]])
    other = Route([s["wp2"], s["exit2"]])
    agent = sim.add_agent(sim._surface.region(id=0).at(5, 2), route=route)
    _unreachable(sim, monkeypatch, s["wp2"])
    with pytest.raises(ValueError, match="^wp2 cannot be reached from "):
        agent.follow(other)
    assert agent.route is route
    assert agent.stage == s["wp1"]


def _unreachable_target(sim, monkeypatch, target):
    """Make the steering target ``target`` unreachable, see _unreachable."""
    real = sim._path_length

    def path_length(start, end):
        if end == target:
            return float("inf")
        return real(start, end)

    monkeypatch.setattr(sim, "_path_length", path_length)


def test_add_agent_rejects_an_unreachable_target(stages, monkeypatch):
    sim, _ = stages
    room = sim._surface.region(id=0)
    target = room.at(8, 8)
    _unreachable_target(sim, monkeypatch, target)
    with pytest.raises(
        ValueError,
        match=r"^Location\(8(\.0)?, 8(\.0)?, region=.*\) cannot be reached "
        r"from Location\(5(\.0)?, 2(\.0)?, region=.*\)$",
    ):
        sim.add_agent(room.at(5, 2), target=target)
    assert sim.agent_count() == 0
    sim.add_agent(room.at(5, 2), target=room.at(7, 7))
    assert sim.agent_count() == 1


def test_steer_to_rejects_an_unreachable_target(stages, monkeypatch):
    sim, s = stages
    room = sim._surface.region(id=0)
    route = Route([s["wp1"], s["exit1"]])
    agent = sim.add_agent(room.at(5, 2), route=route)
    target = room.at(8, 8)
    _unreachable_target(sim, monkeypatch, target)
    with pytest.raises(ValueError, match=r"^Location\(8(\.0)?, 8(\.0)?, "):
        agent.steer_to(target)
    assert agent.route is route
    assert agent.stage == s["wp1"]
    agent.steer_to(room.at(7, 7))
    assert agent.route is None


def test_path_length_over_the_surface(stages):
    sim, _ = stages
    room = sim._surface.region(id=0)
    assert sim._path_length(room.at(2, 2), room.at(5, 6)) == pytest.approx(5.0)


def test_weights_are_bounded(stages):
    _, s = stages
    with pytest.raises(ValueError, match="^weights must be <= 1000000000$"):
        RoundRobin({s["exit1"]: 1_000_000_001, s["exit2"]: 1})
    route = route_c(s)
    with pytest.raises(ValueError, match="^weights must be <= 1000000000$"):
        route.update_weights(s["wp2"], {s["exit1"]: 10**12})
    route.update_weights(s["wp2"], {s["exit1"]: 1_000_000_000})
    assert route.weights(s["wp2"])[s["exit1"]] == 1_000_000_000


@pytest.mark.parametrize("kind", [LeastTravelTime, LeastTargeted])
def test_rule_listing_its_own_stage_warns(stages, kind):
    _, s = stages
    with pytest.warns(
        UserWarning,
        match="^rule at wp1 lists its own stage as a candidate; agents may "
        "never leave it$",
    ):
        Route([(s["wp1"], kind([s["wp1"], s["exit1"]]))], start=s["wp1"])


def test_rule_without_its_own_stage_does_not_warn(stages, recwarn):
    _, s = stages
    Route(
        [
            (s["wp1"], LeastTravelTime([s["wp2"], s["exit1"]])),
            (s["wp2"], LeastTargeted([s["wp1"], s["exit2"]])),
        ],
        start=s["wp1"],
    )
    assert not [w for w in recwarn if issubclass(w.category, UserWarning)]
