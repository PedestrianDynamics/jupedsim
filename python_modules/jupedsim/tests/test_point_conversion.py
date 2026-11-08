# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import numpy as np
import pytest
import shapely


def _room(model):
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    )
    sim = jps.Simulation(model=model, geometry=surface, dt=0.01)
    exit = sim.add_exit(
        [(shapely.Polygon([(18, 0), (20, 0), (20, 20), (18, 20)]), room)]
    )
    journey = jps.Route([exit])
    return sim, room, exit, journey


def _add(sim, room, exit, journey, position, state):
    return sim.add_agent(room.at(*position), route=journey, state=state)


@pytest.mark.parametrize(
    "position",
    [(5, 5), (5.0, 5.0), [5, 5], np.array([5.0, 5.0])],
    ids=["ints", "floats", "list", "numpy"],
)
def test_position_accepts_any_two_element_sequence(position):
    sim, room, exit, journey = _room(jps.CollisionFreeSpeedModelV2())
    agent = _add(
        sim,
        room,
        exit,
        journey,
        position,
        jps.CollisionFreeSpeedModelV2State(),
    )
    assert agent.position == (5.0, 5.0)


def test_position_rejects_wrong_length():
    sim, room, exit, journey = _room(jps.CollisionFreeSpeedModelV2())
    with pytest.raises(TypeError):
        _add(
            sim,
            room,
            exit,
            journey,
            (1, 2, 3),
            jps.CollisionFreeSpeedModelV2State(),
        )


@pytest.mark.parametrize(
    "orientation",
    [(1, 0), (1.0, 0.0), [1, 0], np.array([1.0, 0.0])],
    ids=["ints", "floats", "list", "numpy"],
)
def test_orientation_accepts_any_two_element_sequence(orientation):
    sim, room, exit, journey = _room(jps.CollisionFreeSpeedModelV2())
    agent = _add(
        sim,
        room,
        exit,
        journey,
        (5, 5),
        jps.CollisionFreeSpeedModelV2State(orientation=orientation),
    )
    assert agent.state.orientation == (1.0, 0.0)


def test_orientation_rejects_wrong_length():
    with pytest.raises(TypeError):
        jps.CollisionFreeSpeedModelV2State(orientation=(1, 2, 3))


@pytest.mark.parametrize("bad", [math.nan, math.inf])
def test_non_finite_position_is_not_in_region(bad):
    sim, room, exit, journey = _room(jps.CollisionFreeSpeedModelV2())
    with pytest.raises(ValueError, match="is not in"):
        _add(
            sim,
            room,
            exit,
            journey,
            (bad, 5.0),
            jps.CollisionFreeSpeedModelV2State(),
        )


def test_agents_in_range_of_nan_is_empty():
    sim, *_ = _room(jps.CollisionFreeSpeedModelV2())
    assert list(sim.agents_in_range((math.nan, 0.0), 1.0)) == []


def test_zero_orientation_still_moves():
    sim, room, exit, journey = _room(jps.CollisionFreeSpeedModelV2())
    agent = _add(
        sim,
        room,
        exit,
        journey,
        (5, 5),
        jps.CollisionFreeSpeedModelV2State(orientation=(0.0, 0.0)),
    )
    sim.iterate(3)
    assert agent.location.x > 5.0


@pytest.mark.parametrize(
    "state", [{"mass": 0.0}, {"reaction_time": 0.0}], ids=["mass", "reaction"]
)
def test_social_force_zero_divisor_stays_finite(state):
    sim, room, exit, journey = _room(jps.SocialForceModel())
    agent = _add(
        sim, room, exit, journey, (5, 5), jps.SocialForceModelState(**state)
    )
    sim.iterate(3)
    assert all(math.isfinite(c) for c in agent.position)
