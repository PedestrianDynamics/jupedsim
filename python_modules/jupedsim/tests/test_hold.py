# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
from jupedsim.surface import WalkableSurface

ROOM = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])


def make_sim():
    """A 20 x 10 room: wp1 and wp2 in the middle, wp3 above, exits east."""
    surface = WalkableSurface()
    room = surface.add_region(polygon=ROOM, name="room")
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    s = {
        "wp1": sim.add_waypoint((4, 5), radius=0.6, region=room, name="wp1"),
        "wp2": sim.add_waypoint((9, 5), radius=1.0, region=room, name="wp2"),
        "wp3": sim.add_waypoint((13, 8), radius=0.6, region=room, name="wp3"),
        "exit1": sim.add_exit(
            [(19, 1), (20, 1), (20, 3), (19, 3)], region=room, name="exit1"
        ),
        "exit2": sim.add_exit(
            [(19, 7), (20, 7), (20, 9), (19, 9)], region=room, name="exit2"
        ),
    }
    return sim, room, s


def route_c(s):
    return jps.Route(
        [
            (s["wp1"], s["wp2"]),
            (
                s["wp2"],
                jps.RoundRobin({s["wp3"]: 1, s["exit1"]: 2, s["exit2"]: 1}),
            ),
            (s["wp3"], s["exit2"]),
        ],
        start=s["wp1"],
    )


def split(s):
    """At wp2, a 1:1 round robin to exit1 and exit2."""
    rule = jps.RoundRobin({s["exit1"]: 1, s["exit2"]: 1})
    return jps.Route([(s["wp2"], rule)], start=s["wp2"])


def iterate_until(sim, done, limit=5000):
    for _ in range(limit):
        if done():
            return
        sim.iterate()
    raise AssertionError("condition never met")


def inside(stage_centre, radius, agent):
    """Whether the agent stands in a disc stage; the stage area is read on
    the floor field grid, so allow a cell's slack."""
    x, y = agent.position.xy
    cx, cy = stage_centre
    return (x - cx) ** 2 + (y - cy) ** 2 <= (radius + 0.2) ** 2


def test_hold_scoped_to_route():
    sim, room, s = make_sim()
    route = route_c(s)
    other = jps.Route([s["wp2"], s["exit1"]])
    # Completed wp2 before the hold: not affected.
    passed = sim.add_agent(room.at(9, 5), route=route, start=s["wp2"])
    assert passed.stage != s["wp2"]
    route.hold(s["wp2"])
    assert route.is_held(s["wp2"])
    assert not other.is_held(s["wp2"])
    assert not route.is_held(s["wp1"])
    stage = passed.stage
    sim.iterate()
    assert not passed.held
    assert passed.stage == stage


def test_hold_applies_to_later_agents():
    sim, room, s = make_sim()
    route = route_c(s)
    # Held while walking to wp2 (first) and added after the call (later).
    first = sim.add_agent(room.at(2, 5), route=route)
    for _ in range(100):
        sim.iterate()
    route.hold(s["wp2"])
    later = sim.add_agent(room.at(2, 3), route=route)
    iterate_until(sim, lambda: first.held and later.held)
    stops = [agent.position.xy for agent in (first, later)]
    for agent in (first, later):
        assert agent.stage == s["wp2"]
        assert inside((9, 5), 1.0, agent)
    for _ in range(200):
        sim.iterate()
    for agent, stop in zip((first, later), stops):
        assert agent.held
        assert agent.position.xy == pytest.approx(stop, abs=0.05)
    assert route.held_at(s["wp2"]) == (first, later)


def test_release_in_arrival_order():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    far = sim.add_agent(room.at(3, 5), route=route)
    near = sim.add_agent(room.at(6.5, 5), route=route)
    iterate_until(sim, lambda: near.held)
    # Agents still walking to the held stage are not listed.
    assert route.held_at(s["wp2"]) == (near,)
    iterate_until(sim, lambda: far.held)
    assert route.held_at(s["wp2"]) == (near, far)
    route.release(s["wp2"])
    # The first slot goes to the agent that arrived first, not the lower id.
    assert near.stage == s["exit1"]
    assert far.stage == s["exit2"]
    assert not near.held and not far.held
    assert not route.is_held(s["wp2"])
    assert route.held_at(s["wp2"]) == ()


def test_release_frees_walking_agents():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    agent = sim.add_agent(room.at(3, 5), route=route)
    for _ in range(10):
        sim.iterate()
    assert agent.stage == s["wp2"] and not agent.held
    route.release(s["wp2"])
    iterate_until(sim, lambda: agent.stage != s["wp2"])
    assert not agent.held
    assert agent.stage in (s["exit1"], s["exit2"])


def test_follow_clears_held():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    agent = sim.add_agent(room.at(9, 5), route=route)
    assert agent.held
    agent.follow(jps.Route([s["exit1"]]))
    assert not agent.held
    assert agent.stage == s["exit1"]
    assert route.held_at(s["wp2"]) == ()
    sim.iterate()
    assert not agent.held
    # steer_to() clears it as well.
    agent.follow(route)
    sim.iterate()
    assert agent.held
    agent.steer_to(room.at(15, 5))
    assert not agent.held
    assert route.held_at(s["wp2"]) == ()


def test_follow_onto_held_start_holds_again():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    agent = sim.add_agent(room.at(9, 5), route=route)
    assert agent.held
    for _ in range(5):
        sim.iterate()
    agent.follow(route)
    assert not agent.held
    assert route.held_at(s["wp2"]) == ()
    sim.iterate()
    assert agent.held
    assert route.held_at(s["wp2"]) == (agent,)


def test_repeated_hold_noop():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    agent = sim.add_agent(room.at(9, 5), route=route)
    route.hold(s["wp2"])
    assert route.is_held(s["wp2"])
    assert agent.held
    route.release(s["wp2"])
    assert not route.is_held(s["wp2"])
    assert not agent.held
    with pytest.raises(ValueError, match="^wp2 is not held$"):
        route.release(s["wp2"])


def test_hold_close_release():
    sim, room, s = make_sim()
    route = route_c(s)
    route.hold(s["wp2"])
    spots = [(8.5, 5), (9.5, 5), (9, 4.4), (9, 5.6)]
    agents = [
        sim.add_agent(room.at(*xy), route=route, start=s["wp2"]) for xy in spots
    ]
    assert all(agent.held for agent in agents)
    assert s["exit2"].close(informed=True) == ()
    assert all(agent.held for agent in agents)
    route.release(s["wp2"])
    # Arrival order (all in the same iteration: by id), smooth round robin
    # over the open targets wp3: 1 and exit1: 2.
    assert [agent.stage for agent in agents] == [
        s["exit1"],
        s["wp3"],
        s["exit1"],
        s["exit1"],
    ]
    assert not any(agent.held or agent.stranded for agent in agents)
    # wp3 leads to exit2 by a fixed transition: stranded inside wp3.
    iterate_until(sim, lambda: agents[1].stranded)
    assert agents[1].stage == s["wp3"]
    assert inside((13, 8), 0.6, agents[1])


def test_hold_close_release_without_open_target_strands_inside():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    agent = sim.add_agent(room.at(9, 5), route=route)
    s["exit1"].close(informed=True)
    s["exit2"].close(informed=True)
    route.release(s["wp2"])
    assert not agent.held
    assert agent.stranded
    assert agent.stage == s["wp2"]
    for _ in range(100):
        sim.iterate()
    assert inside((9, 5), 1.0, agent)
    assert s["exit2"].open() == (agent,)
    assert agent.stage == s["exit2"]


def test_holds_per_route():
    sim, room, s = make_sim()
    held_route = split(s)
    free_route = jps.Route([s["wp2"], s["exit1"]])
    held_route.hold(s["wp2"])
    held = sim.add_agent(room.at(3, 4), route=held_route)
    free = sim.add_agent(room.at(3, 6), route=free_route)
    iterate_until(sim, lambda: held.held)
    iterate_until(sim, lambda: free.stage == s["exit1"])
    assert not free.held
    assert free_route.held_at(s["wp2"]) == ()
    assert held_route.held_at(s["wp2"]) == (held,)
    x = free.position.xy[0]
    for _ in range(50):
        sim.iterate()
    assert free.position.xy[0] > x
    assert held.held


def test_update_weights_while_held_applies_on_release():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    agent = sim.add_agent(room.at(9, 5), route=route)
    assert agent.held
    # Without the change the first slot of 1:1 would be exit1.
    route.update_weights(s["wp2"], {s["exit1"]: 0})
    assert agent.held and agent.stage == s["wp2"]
    route.release(s["wp2"])
    assert agent.stage == s["exit2"]


def test_open_exit_leaves_held_agents_held():
    sim, room, s = make_sim()
    route = split(s)
    route.hold(s["wp2"])
    s["exit2"].close(informed=True)
    agent = sim.add_agent(room.at(9, 5), route=route)
    assert agent.held
    assert s["exit2"].open() == ()
    assert agent.held
    assert not agent.stranded
    assert agent.stage == s["wp2"]


def test_hold_errors():
    _, _, s = make_sim()
    route = route_c(s)
    _, _, other = make_sim()
    with pytest.raises(
        ValueError, match=r"^exits cannot be held; use exit1\.close\(\)$"
    ):
        route.hold(s["exit1"])
    with pytest.raises(ValueError, match="^wp1 is not held$"):
        route.release(s["wp1"])
    narrow = jps.Route([s["wp1"], s["exit1"]])
    with pytest.raises(ValueError, match="^wp2 is not part of this route"):
        narrow.is_held(s["wp2"])
    with pytest.raises(ValueError, match="^wp2 is not part of this route"):
        narrow.hold(s["wp2"])
    with pytest.raises(ValueError, match="^wp2 is not part of this route"):
        narrow.release(s["wp2"])
    with pytest.raises(ValueError, match="^wp2 is not part of this route"):
        narrow.held_at(s["wp2"])
    with pytest.raises(ValueError, match="is not part of this route"):
        route.hold(other["wp2"])
    assert not route.is_held(s["exit1"])


def test_held_is_read_only():
    sim, room, s = make_sim()
    agent = sim.add_agent(room.at(2, 5), route=route_c(s))
    assert agent.held is False
    with pytest.raises(AttributeError):
        agent.held = True
