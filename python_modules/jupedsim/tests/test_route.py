# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
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
    def create_fixed_transition(stage_id):
        return ("fixed", stage_id)

    @staticmethod
    def create_round_robin_transition(pairs):
        return ("round_robin", list(pairs))

    @staticmethod
    def create_least_targeted_transition(ids):
        return ("least_targeted", list(ids))

    @staticmethod
    def create_none_transition():
        return ("none",)


def test_native_rules_match_journey_description(stages, monkeypatch):
    import jupedsim.journey as journey_module
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
    monkeypatch.setattr(journey_module, "Transition", _Recorder)
    journey = jps.JourneyDescription([st.id for st in route.stages])
    journey.set_transition_for_stage(
        s["wp1"].id, _Recorder.create_fixed_transition(s["wp2"].id)
    )
    journey.set_transition_for_stage(
        s["wp2"].id,
        _Recorder.create_round_robin_transition(
            [(s["wp3"].id, 1), (s["exit1"].id, 2)]
        ),
    )
    journey.set_transition_for_stage(
        s["wp3"].id,
        _Recorder.create_least_targeted_transition(
            [s["exit1"].id, s["exit2"].id]
        ),
    )
    built = {
        st.id: route_module._native_rule(st, route._transitions)
        for st in route.stages
    }
    assert built == journey._transitions


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


def test_all_zero_weights(stages):
    _, s = stages
    with pytest.raises(ValueError, match="^at least one weight must be > 0$"):
        RoundRobin({s["exit1"]: 0, s["exit2"]: 0})


def test_unsupported_features_are_not_implemented(stages):
    _, s = stages
    with pytest.raises(NotImplementedError):
        Route([s["wp1"], STAY])
    with pytest.raises(NotImplementedError):
        Route([(s["wp1"], LeastTravelTime([s["exit1"], s["exit2"]]))])
    with pytest.raises(NotImplementedError):
        Route(
            [(s["wp1"], RoundRobin({s["exit1"]: 0, s["exit2"]: 1}))],
            start=s["wp1"],
        )
