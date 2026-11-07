# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest
import shapely
from jupedsim.surface import WalkableSurface

ROOM = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])


def make_sim(model=None):
    """A 20 x 10 room: wp1 in the middle, wp2 further east, an exit east."""
    surface = WalkableSurface()
    room = surface.add_region(polygon=ROOM, name="room")
    sim = jps.Simulation(model or jps.CollisionFreeSpeedModel(), surface)
    s = {
        "wp1": sim.add_waypoint((9, 5), radius=1.0, region=room, name="wp1"),
        "wp2": sim.add_waypoint((13, 5), radius=0.6, region=room, name="wp2"),
        "exit1": sim.add_exit(
            [(19, 4), (20, 4), (20, 6), (19, 6)], region=room, name="exit1"
        ),
    }
    return sim, room, s


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


def test_stay_arrives_and_stays():
    sim, room, s = make_sim()
    route = jps.Route([s["wp1"], jps.STAY])
    agent = sim.add_agent(room.at(2, 5), route=route)
    assert not agent.arrived
    iterate_until(sim, lambda: agent.arrived)
    assert inside((9, 5), 1.0, agent)
    for _ in range(100):
        sim.iterate()
    assert agent.alive
    assert agent.arrived
    assert agent.stage == s["wp1"]
    assert inside((9, 5), 1.0, agent)
    assert sim.agent_count() == 1


def test_stay_after_a_route():
    sim, room, s = make_sim()
    route = jps.Route([s["wp1"], s["wp2"], jps.STAY])
    agent = sim.add_agent(room.at(2, 5), route=route)
    iterate_until(sim, lambda: agent.stage == s["wp2"])
    # Completing wp1 does not arrive: its next stage is wp2.
    assert not agent.arrived
    iterate_until(sim, lambda: agent.arrived)
    assert agent.stage == s["wp2"]
    assert inside((13, 5), 0.6, agent)


def test_stay_pair_form():
    sim, room, s = make_sim()
    route = jps.Route([(s["wp1"], jps.STAY)], start=s["wp1"])
    agent = sim.add_agent(room.at(2, 5), route=route)
    iterate_until(sim, lambda: agent.arrived)
    assert agent.stage == s["wp1"]


@pytest.mark.parametrize(
    "model",
    [
        jps.CollisionFreeSpeedModel(),
        jps.CollisionFreeSpeedModelV2(),
        jps.AnticipationVelocityModel(),
        jps.SocialForceModel(),
        jps.GeneralizedCentrifugalForceModel(),
        jps.WarpDriverModel(),
    ],
)
def test_steer_to_point_arrives(model):
    sim, room, _ = make_sim(model)
    # A target that no whole number of steps reaches exactly.
    point = (8.37, 5.21)
    agent = sim.add_agent(room.at(2, 5), target=room.at(*point))
    assert not agent.arrived
    iterate_until(sim, lambda: agent.arrived)
    x, y = agent.position.xy
    assert math.dist((x, y), point) <= 0.05
    for _ in range(100):
        sim.iterate()
    assert agent.alive
    assert agent.arrived
    # Force based models overshoot by their inertia and steer back.
    x, y = agent.position.xy
    assert math.dist((x, y), point) <= 0.25
    assert agent.target.xy == pytest.approx(point)


def test_steer_to_target_at_start_arrives_in_first_iteration():
    sim, room, _ = make_sim()
    agent = sim.add_agent(room.at(2, 5), target=room.at(2, 5))
    sim.iterate()
    assert agent.arrived


def test_follow_clears_arrived():
    sim, room, s = make_sim()
    stay = jps.Route([s["wp1"], jps.STAY])
    onwards = jps.Route([s["wp2"], s["exit1"]])
    agent = sim.add_agent(room.at(2, 5), route=stay)
    iterate_until(sim, lambda: agent.arrived)
    agent.follow(onwards)
    assert not agent.arrived
    assert agent.stage == s["wp2"]
    iterate_until(sim, lambda: sim.removed_agents())
    assert [r.exit for r in sim.removed_agents()] == [s["exit1"]]


def test_follow_onto_stay_arrives_again():
    sim, room, s = make_sim()
    stay = jps.Route([s["wp1"], jps.STAY])
    agent = sim.add_agent(room.at(2, 5), route=stay)
    iterate_until(sim, lambda: agent.arrived)
    agent.follow(stay)
    assert not agent.arrived
    sim.iterate()
    assert agent.arrived


def test_steer_to_clears_arrived():
    sim, room, s = make_sim()
    stay = jps.Route([s["wp1"], jps.STAY])
    agent = sim.add_agent(room.at(2, 5), route=stay)
    iterate_until(sim, lambda: agent.arrived)
    agent.steer_to(room.at(4, 8))
    assert not agent.arrived
    assert agent.stage is None
    for _ in range(200):
        sim.iterate()
    # It moves again, toward the point.
    assert not inside((9, 5), 1.0, agent)
    iterate_until(sim, lambda: agent.arrived)
    assert math.dist(agent.position.xy, (4, 8)) <= 0.05


def test_steer_to_clears_arrived_at_a_point():
    sim, room, s = make_sim()
    agent = sim.add_agent(room.at(2, 5), target=room.at(4, 5))
    iterate_until(sim, lambda: agent.arrived)
    agent.steer_to(room.at(6, 5))
    assert not agent.arrived
    agent2 = sim.add_agent(room.at(2, 8), target=room.at(4, 8))
    iterate_until(sim, lambda: agent2.arrived)
    agent2.follow(jps.Route([s["wp1"], jps.STAY]))
    assert not agent2.arrived


def test_arrived_is_read_only():
    sim, room, s = make_sim()
    agent = sim.add_agent(room.at(2, 5), route=jps.Route([s["wp1"], jps.STAY]))
    with pytest.raises(AttributeError):
        agent.arrived = True


def test_follow_sets_route_orientation_at_once():
    sim, room, s = make_sim()
    agent = sim.add_agent(room.at(9, 2), target=room.at(9, 1))
    sim.iterate()
    assert agent.route_orientation[1] < -0.9
    # wp2 lies east of the agent: the orientation points there before the
    # next iterate().
    agent.follow(jps.Route([s["wp2"], jps.STAY]))
    assert agent.route_orientation[0] > 0.5


@pytest.mark.parametrize("target", [(8.37, 5.21), (13.123, 2.777)])
def test_fast_agent_with_large_dt_arrives(target):
    # Steps of 0.4 m: much longer than the 0.05 m tolerance.
    surface = WalkableSurface()
    room = surface.add_region(polygon=ROOM, name="room")
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface, dt=0.1)
    agent = sim.add_agent(
        room.at(2, 5),
        target=room.at(*target),
        state=jps.CollisionFreeSpeedModelState(desired_speed=4.0),
    )
    iterate_until(sim, lambda: agent.arrived, limit=200)
    assert math.dist(agent.position.xy, target) <= 0.4
