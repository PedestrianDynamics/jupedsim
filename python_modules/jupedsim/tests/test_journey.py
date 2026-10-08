# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest


def _simulation():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
        dt=0.01,
    )


def test_every_transition_kind_is_one_native_type():
    transitions = [
        jps.Transition.create_fixed_transition(1),
        jps.Transition.create_round_robin_transition([(1, 1), (2, 3)]),
        jps.Transition.create_least_targeted_transition([1, 2]),
        jps.Transition.create_none_transition(),
    ]
    assert all(type(t) is jps.Transition for t in transitions)
    assert jps.Transition.__module__ == "jupedsim.py_jupedsim"
    assert "Describes the Transition at a stage" in jps.Transition.__doc__
    assert (
        "Create a fixed transition"
        in jps.Transition.create_fixed_transition.__doc__
    )


def test_a_journey_with_every_transition_kind_leads_to_the_exit():
    sim = _simulation()
    first = sim.add_waypoint_stage((2, 5), 0.5, region_id=0)
    upper = sim.add_waypoint_stage((5, 7), 0.5, region_id=0)
    lower = sim.add_waypoint_stage((5, 3), 0.5, region_id=0)
    last = sim.add_waypoint_stage((8, 5), 0.5, region_id=0)
    exit_id = sim.add_exit_stage(
        [(9, 4), (10, 4), (10, 6), (9, 6)], region_id=0
    )
    journey = jps.JourneyDescription()
    journey.add([first, upper, lower])  # a list (was a TypeError)
    journey.add(last)
    journey.add(exit_id)
    journey.set_transition_for_stage(
        first,
        jps.Transition.create_round_robin_transition([(upper, 1), (lower, 1)]),
    )
    for middle in (upper, lower):
        journey.set_transition_for_stage(
            middle, jps.Transition.create_least_targeted_transition([last])
        )
    journey.set_transition_for_stage(
        last, jps.Transition.create_fixed_transition(exit_id)
    )
    journey_id = sim.add_journey(journey)
    for y in (4.0, 6.0):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=first,
            position=(1.0, y),
            state=jps.CollisionFreeSpeedModelState(),
            region_id=0,
        )
    while sim.agent_count() > 0 and sim.iteration_count() < 5000:
        sim.iterate()
    assert sim.agent_count() == 0


def test_journey_description_add_accepts_ids_and_lists():
    journey = jps.JourneyDescription([1])
    journey.add(2)
    journey.add([3, 4])
    assert sorted(journey._transitions) == [1, 2, 3, 4]
    assert all(type(t) is jps.Transition for t in journey._transitions.values())


@pytest.mark.parametrize(
    "create",
    [
        lambda: jps.Transition.create_fixed_transition(0),
        lambda: jps.Transition.create_round_robin_transition([(0, 1)]),
        lambda: jps.Transition.create_least_targeted_transition([0]),
    ],
)
def test_a_transition_to_the_invalid_stage_id_is_rejected(create):
    with pytest.raises(jps.SimulationError, match="invalid stage id"):
        create()


def test_a_journey_holding_something_else_than_a_transition_is_rejected():
    sim = _simulation()
    stage = sim.add_waypoint_stage((2, 5), 0.5, region_id=0)
    journey = jps.JourneyDescription([stage])
    journey.set_transition_for_stage(stage, None)
    with pytest.raises(TypeError):
        sim.add_journey(journey)
