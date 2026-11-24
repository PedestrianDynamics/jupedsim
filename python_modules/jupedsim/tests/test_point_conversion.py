# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import numpy as np
import pytest
import shapely


def _room(model):
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)]),
        height=0.0,
    )
    sim = jps.Simulation(model=model, geometry=surface.build(), dt=0.01)
    exit_ = sim.add_target(
        jps.Exit(
            [(shapely.Polygon([(18, 0), (20, 0), (20, 20), (18, 20)]), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)])
    return sim, room, route


def _add(sim, room, route, position, state):
    return sim.add_agent(
        room.at(*position),
        route=route,
        state=state,
    )


@pytest.mark.parametrize(
    "position",
    [(5, 5), (5.0, 5.0), [5, 5], np.array([5.0, 5.0])],
    ids=["ints", "floats", "list", "numpy"],
)
def test_position_accepts_any_two_element_sequence(position):
    sim, room, route = _room(jps.CollisionFreeSpeedModelV2())
    agent = _add(
        sim,
        room,
        route,
        position,
        jps.CollisionFreeSpeedModelV2State(),
    )
    assert agent.location.xy == (5.0, 5.0)


def test_position_rejects_wrong_length():
    sim, room, route = _room(jps.CollisionFreeSpeedModelV2())
    with pytest.raises(TypeError):
        _add(
            sim,
            room,
            route,
            (1, 2, 3),
            jps.CollisionFreeSpeedModelV2State(),
        )


@pytest.mark.parametrize(
    "orientation",
    [(1, 0), (1.0, 0.0), [1, 0], np.array([1.0, 0.0])],
    ids=["ints", "floats", "list", "numpy"],
)
def test_orientation_accepts_any_two_element_sequence(orientation):
    sim, room, route = _room(jps.CollisionFreeSpeedModelV2())
    agent = _add(
        sim,
        room,
        route,
        (5, 5),
        jps.CollisionFreeSpeedModelV2State(orientation=orientation),
    )
    assert agent.state.orientation == (1.0, 0.0)


def test_orientation_rejects_wrong_length():
    with pytest.raises(TypeError):
        jps.CollisionFreeSpeedModelV2State(orientation=(1, 2, 3))


@pytest.mark.parametrize("bad", [math.nan, math.inf])
def test_non_finite_position_is_not_in_region(bad):
    sim, room, route = _room(jps.CollisionFreeSpeedModelV2())
    with pytest.raises(jps.SimulationError, match="is not in region"):
        _add(
            sim,
            room,
            route,
            (bad, 5.0),
            jps.CollisionFreeSpeedModelV2State(),
        )
