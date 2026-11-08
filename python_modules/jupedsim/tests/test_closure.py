# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
from jupedsim.surface import WalkableSurface

SQUARE = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])


def make_sim():
    surface = WalkableSurface()
    room = surface.add_region(polygon=SQUARE, name="room")
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    wp = sim.add_waypoint((3, 5), radius=0.5, region=room, name="wp")
    exit1 = sim.add_exit(
        [(9, 1), (10, 1), (10, 3), (9, 3)], region=room, name="exit1"
    )
    exit2 = sim.add_exit(
        [(9, 7), (10, 7), (10, 9), (9, 9)], region=room, name="exit2"
    )
    return sim, room, wp, exit1, exit2


def rule_route(wp, exit1, exit2):
    return jps.Route([(wp, jps.RoundRobin({exit1: 1, exit2: 1}))], start=wp)


def iterate_until_decided(sim, agents, exits):
    for _ in range(2000):
        sim.iterate()
        if all(agent.stage in exits for agent in agents):
            return
    raise AssertionError("agents did not reach a decision")


def test_close_reroutes_rule_agents():
    sim, room, wp, exit1, exit2 = make_sim()
    agent = sim.add_agent(room.at(3, 5), route=rule_route(wp, exit1, exit2))
    iterate_until_decided(sim, [agent], (exit1, exit2))
    chosen = agent.stage
    other = exit2 if chosen == exit1 else exit1
    assert chosen.closed is False
    assert chosen.close(informed=True) == ()
    assert chosen.closed is True
    assert agent.stage == other
    assert not agent.stranded


def test_close_points_the_orientation_at_the_new_target():
    sim, room, wp, exit1, _ = make_sim()
    west = sim.add_exit([(0, 4), (1, 4), (1, 6), (0, 6)], region=room)
    route = jps.Route([(wp, jps.RoundRobin({exit1: 1, west: 1}))], start=wp)
    agent = sim.add_agent(room.at(3, 5), route=route)
    iterate_until_decided(sim, [agent], (exit1, west))
    assert agent.stage == exit1
    assert agent.route_orientation[0] > 0.5
    exit1.close(informed=True)
    assert agent.stage == west
    assert agent.route_orientation[0] < -0.9


def test_close_returns_stranded_agents_and_open_resumes_them():
    sim, room, _, exit1, exit2 = make_sim()
    agent = sim.add_agent(room.at(5, 2), route=jps.Route([exit1]))
    assert exit1.close(informed=True) == (agent,)
    assert agent.stranded
    assert exit1.close(informed=True) == ()
    assert exit1.open() == (agent,)
    assert not agent.stranded
    assert exit1.closed is False
    assert exit1.open() == ()


def test_closed_is_read_only():
    sim, room, _, exit1, exit2 = make_sim()
    with pytest.raises(AttributeError, match=r"use exit1\.close\(\)"):
        exit1.closed = True
    with pytest.raises(TypeError):
        exit1.close()


def decision_setup():
    sim, room, wp, exit1, exit2 = make_sim()
    route = rule_route(wp, exit1, exit2)
    agents = [
        sim.add_agent(room.at(1.5, 3.4 + 0.8 * i), route=route)
        for i in range(5)
    ]
    return sim, wp, exit1, exit2, agents


def distance_to(agent, wp, centre=(3.0, 5.0)):
    x, y = agent.position
    return ((x - centre[0]) ** 2 + (y - centre[1]) ** 2) ** 0.5


def iterate_until_about_to_decide(sim, wp, agents):
    for _ in range(2000):
        if any(distance_to(a, wp) <= 0.5 + 0.05 for a in agents):
            return
        sim.iterate()
        assert all(a.stage == wp for a in agents), "decided too early"
    raise AssertionError("no agent came close to the waypoint")


def iterate_and_collect_choices(sim, agents, exit1, exit2, closed=None):
    chosen = set()
    for _ in range(2000):
        sim.iterate()
        for agent in agents:
            if closed is not None:
                assert agent.stage != closed
            if agent.stage in (exit1, exit2):
                chosen.add(agent.stage)
        if all(a.stage in (exit1, exit2) for a in agents):
            return chosen
    raise AssertionError("agents did not reach a decision")


@pytest.mark.parametrize("informed", [True, False])
def test_close_during_decision_step(informed):
    sim, wp, exit1, exit2, agents = decision_setup()
    iterate_until_about_to_decide(sim, wp, agents)
    assert all(agent.stage == wp for agent in agents)
    exit1.close(informed=informed)
    chosen = iterate_and_collect_choices(sim, agents, exit1, exit2, exit1)
    assert chosen == {exit2}
    assert not any(agent.stranded for agent in agents)


def test_decision_step_without_close_uses_exit1():
    sim, wp, exit1, exit2, agents = decision_setup()
    iterate_until_about_to_decide(sim, wp, agents)
    chosen = iterate_and_collect_choices(sim, agents, exit1, exit2)
    assert exit1 in chosen


def test_follow_onto_closed_start_strands():
    sim, room, _, exit1, exit2 = make_sim()
    agent = sim.add_agent(room.at(5, 5), route=jps.Route([exit2]))
    exit1.close(informed=True)
    agent.follow(jps.Route([exit1]))
    assert agent.stranded


def test_follow_clears_stranded_and_decision():
    sim, room, _, exit1, exit2 = make_sim()
    agent = sim.add_agent(room.at(5, 2), route=jps.Route([exit1]))
    assert exit1.close(informed=True) == (agent,)
    agent.follow(jps.Route([exit2]))
    assert not agent.stranded
    assert exit1.open() == ()
    assert not agent.stranded
    assert agent.stage == exit2


def test_steer_to_clears_stranded():
    sim, room, _, exit1, exit2 = make_sim()
    agent = sim.add_agent(room.at(5, 2), route=jps.Route([exit1]))
    exit1.close(informed=True)
    agent.steer_to(room.at(4, 4))
    assert not agent.stranded
    assert exit1.open() == ()
    assert agent.stage is None
