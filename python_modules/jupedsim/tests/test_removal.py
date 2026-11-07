# SPDX-License-Identifier: LGPL-3.0-or-later
import dataclasses

import jupedsim as jps
import pytest
import shapely
from jupedsim.surface import WalkableSurface

SQUARE = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
EXIT_AREA = [(9, 4), (10, 4), (10, 6), (9, 6)]


def make_sim():
    surface = WalkableSurface()
    room = surface.add_region(polygon=SQUARE, name="room")
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface, dt=0.01)
    exit1 = sim.add_exit(EXIT_AREA, region=room, name="exit1")
    return sim, room, exit1


def iterate_until_removed(sim):
    for _ in range(5000):
        sim.iterate()
        if sim.removed_agents():
            return
    raise AssertionError("no agent was removed")


def test_removal_holds_exit_and_time():
    sim, room, exit1 = make_sim()
    agent = sim.add_agent(room.at(7, 5), route=jps.Route([exit1]))
    reached_at = 0.0
    for _ in range(5000):
        reached_at = sim.elapsed_time()
        sim.iterate()
        if sim.removed_agents():
            break
    (removal,) = sim.removed_agents()
    assert removal.id == agent.id
    assert removal.exit is exit1
    assert removal.time == pytest.approx(reached_at)
    assert agent.alive
    sim.iterate()
    assert not agent.alive
    assert sim.removed_agents() == ()


def test_removal_is_frozen():
    sim, room, exit1 = make_sim()
    sim.add_agent(room.at(7, 5), route=jps.Route([exit1]))
    iterate_until_removed(sim)
    (removal,) = sim.removed_agents()
    assert isinstance(removal, jps.Removal)
    with pytest.raises(dataclasses.FrozenInstanceError):
        removal.time = 0.0


def test_removed_agents_stable_within_step():
    sim, room, exit1 = make_sim()
    sim.add_agent(room.at(7, 5), route=jps.Route([exit1]))
    iterate_until_removed(sim)
    first = sim.removed_agents()
    assert first == sim.removed_agents()
    sim.iterate()
    assert sim.removed_agents() != first
    assert sim.removed_agents() == ()


def test_remove_has_no_exit():
    sim, room, exit1 = make_sim()
    agent = sim.add_agent(room.at(2, 5), route=jps.Route([exit1]))
    sim.iterate(count=3)
    removal_time = sim.elapsed_time()
    agent.remove()
    (removal,) = sim.removed_agents()
    assert removal.id == agent.id
    assert removal.exit is None
    assert removal.time == pytest.approx(removal_time)
