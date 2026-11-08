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
    assert sim.removed_agents() == ()
    sim.iterate()
    (removal,) = sim.removed_agents()
    assert removal.id == agent.id
    assert removal.exit is None
    assert removal.time == pytest.approx(removal_time)
    assert not agent.alive


def test_iterate_loop_sees_removals_by_remove():
    sim, room, exit1 = make_sim()
    removed = sim.add_agent(room.at(2, 5), route=jps.Route([exit1]))
    leaving = sim.add_agent(room.at(7, 5), route=jps.Route([exit1]))
    records = []
    for i in range(5000):
        if i == 3:
            removed.remove()
        sim.iterate()
        records.extend(sim.removed_agents())
        if not leaving.alive:
            break
    exits = {r.id: r.exit for r in records}
    assert exits == {removed.id: None, leaving.id: exit1}


def test_remove_after_exit_keeps_one_record():
    sim, room, exit1 = make_sim()
    agent = sim.add_agent(room.at(7, 5), route=jps.Route([exit1]))
    iterate_until_removed(sim)
    agent.remove()
    (removal,) = sim.removed_agents()
    assert removal.exit is exit1
    sim.iterate()
    assert sim.removed_agents() == ()


def test_remove_twice_keeps_one_record():
    sim, room, exit1 = make_sim()
    agent = sim.add_agent(room.at(2, 5), route=jps.Route([exit1]))
    agent.remove()
    agent.remove()
    sim.iterate()
    (removal,) = sim.removed_agents()
    assert removal.id == agent.id
    assert removal.exit is None


def test_two_exits_in_one_step():
    sim, room, exit1 = make_sim()
    exit2 = sim.add_exit([(0, 4), (1, 4), (1, 6), (0, 6)], region=room)
    a = sim.add_agent(room.at(7, 5), route=jps.Route([exit1]))
    b = sim.add_agent(room.at(3, 5), route=jps.Route([exit2]))
    iterate_until_removed(sim)
    exits = {r.id: r.exit for r in sim.removed_agents()}
    assert exits[a.id] is exit1
    assert exits[b.id] is exit2


def test_agent_spawned_inside_its_exit_is_reported_once():
    sim, room, exit1 = make_sim()
    agent = sim.add_agent(room.at(9.5, 5), route=jps.Route([exit1]))
    assert sim.removed_agents() == ()
    seen = []
    for _ in range(3):
        sim.iterate()
        seen.extend(sim.removed_agents())
    assert [(r.id, r.exit) for r in seen] == [(agent.id, exit1)]
    assert seen[0].time == 0.0
    assert not agent.alive


def test_every_agent_spawned_inside_an_exit_gets_one_record():
    sim, room, exit1 = make_sim()
    exit2 = sim.add_exit([(0, 4), (1, 4), (1, 6), (0, 6)], region=room)
    spawned = {
        sim.add_agent(room.at(9.5, y), route=jps.Route([exit1])): exit1
        for y in (4.3, 5.0, 5.7)
    } | {
        sim.add_agent(room.at(0.5, y), route=jps.Route([exit2])): exit2
        for y in (4.3, 5.7)
    }
    walker = sim.add_agent(room.at(5, 5), route=jps.Route([exit1]))
    seen = []
    for _ in range(5000):
        if not sim.agent_count():
            break
        sim.iterate()
        seen.extend(sim.removed_agents())
    assert sim.agent_count() == 0
    assert sorted(r.id for r in seen) == sorted(
        [agent.id for agent in spawned] + [walker.id]
    )
    for agent, exit_stage in spawned.items():
        (record,) = [r for r in seen if r.id == agent.id]
        assert record.exit is exit_stage
        assert record.time == 0.0
