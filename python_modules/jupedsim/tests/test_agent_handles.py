# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
from jupedsim.agent import Agent
from jupedsim.pyjupedsim import WalkableSurface as NativeSurface
from jupedsim.route import Route
from jupedsim.surface import Location, WalkableSurface

SQUARE = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
EXIT_AREA = [(9, 4), (10, 4), (10, 6), (9, 6)]


def make_sim(model=None):
    surface = WalkableSurface()
    room = surface.add_region(polygon=SQUARE, name="room")
    sim = jps.Simulation(
        model or jps.CollisionFreeSpeedModel(), surface, dt=0.01
    )
    wps = [
        sim.add_waypoint((2 + i, 2), radius=0.4, region=room, name=f"wp{i}")
        for i in range(3)
    ]
    exit1 = sim.add_exit(EXIT_AREA, region=room, name="exit1")
    return sim, surface, room, wps, exit1


@pytest.fixture
def world():
    sim, surface, room, wps, exit1 = make_sim()
    route_a = Route([wps[0], wps[1], exit1])
    route_b = Route([wps[1], wps[2], exit1])
    return sim, surface, room, wps, exit1, route_a, route_b


def test_positional_model_and_geometry():
    sim, *_ = make_sim()
    assert sim.delta_time() == 0.01
    with pytest.raises(TypeError):
        jps.Simulation(jps.CollisionFreeSpeedModel(), WalkableSurface(), 0.1)


def test_add_agent_on_route(world):
    sim, _, room, wps, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    assert isinstance(agent, Agent)
    assert agent.stage == wps[0]
    assert agent.route is route_a
    assert agent.target is None
    assert agent.location == room.at(1, 8)
    assert agent.alive
    assert repr(agent) == f"Agent(id={agent.id})"


def test_add_agent_start_overrides_default(world):
    sim, _, room, wps, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a, start=wps[1])
    assert agent.stage == wps[1]


def test_add_agent_with_target(world):
    sim, _, room, _, _, _, _ = world
    agent = sim.add_agent(room.at(1, 8), target=room.at(5, 5))
    assert agent.route is None
    assert agent.stage is None
    assert agent.target == room.at(5, 5)


def test_direct_steering_is_hidden(world):
    sim, _, room, wps, exit1, route_a, _ = world
    before = sim.stages
    first = sim.add_agent(room.at(1, 8), target=room.at(5, 5))
    second = sim.add_agent(room.at(2, 8), target=room.at(6, 5))
    assert sim.stages == before
    assert first.target != second.target
    assert len(sim._obj.stage_ids()) == len(before) + 1


def test_agent_walks_to_steer_target(world):
    sim, _, room, _, _, _, _ = world
    agent = sim.add_agent(room.at(1, 8), target=room.at(3, 8))
    sim.iterate(1500)
    assert agent.location.x == pytest.approx(3, abs=0.3)


def test_follow_restarts(world):
    sim, _, room, wps, _, route_a, route_b = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    agent.follow(route_b)
    assert agent.route is route_b
    assert agent.stage == route_b.start
    agent.follow(route_b, start=wps[2])
    assert agent.stage == wps[2]
    agent.follow(route_b)
    assert agent.stage == route_b.start


def test_steer_to_switches_to_direct(world):
    sim, _, room, _, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    agent.steer_to(room.at(4, 4))
    assert agent.route is None
    assert agent.stage is None
    assert agent.target == room.at(4, 4)
    agent.steer_to(room.at(6, 6))
    assert agent.target == room.at(6, 6)
    agent.follow(route_a)
    assert agent.route is route_a
    assert agent.stage == route_a.start
    assert agent.target is None


def test_steer_to_points_the_orientation_at_the_new_target(world):
    sim, _, room, _, exit1, _, _ = world
    agent = sim.add_agent(room.at(5, 5), route=Route([exit1]))
    assert agent.route_orientation[0] > 0.9
    agent.steer_to(room.at(1, 5))
    assert agent.route_orientation[0] < -0.9
    steered = sim.add_agent(room.at(5, 7), target=room.at(1, 7))
    assert steered.route_orientation[0] < -0.9


def test_agents_in_id_order(world):
    sim, _, room, _, _, route_a, _ = world
    made = [sim.add_agent(room.at(1, 1 + i), route=route_a) for i in range(4)]
    agents = sim.agents()
    assert isinstance(agents, tuple)
    assert [a.id for a in agents] == sorted(a.id for a in made)
    assert agents[2] == sim.agent(made[2].id)
    assert sim.agent_count() == 4


def test_agents_targeting(world):
    sim, _, room, wps, _, route_a, route_b = world
    on_a = sim.add_agent(room.at(1, 8), route=route_a)
    on_b = sim.add_agent(room.at(2, 8), route=route_b, start=wps[1])
    other = sim.add_agent(room.at(3, 8), route=route_a, start=wps[1])
    assert sim.agents_targeting(wps[1]) == (on_b, other)
    assert sim.agents_targeting(wps[1], route=route_a) == (other,)
    assert sim.agents_targeting(wps[0]) == (on_a,)
    with pytest.raises(ValueError, match=r"wp0 is not part of this route"):
        sim.agents_targeting(wps[0], route=route_b)


def test_agents_targeting_is_snapshot(world):
    sim, _, room, wps, _, route_a, route_b = world
    original = [
        sim.add_agent(room.at(1, 1 + i), route=route_a, start=wps[1]).id
        for i in range(3)
    ]
    visited = []
    for agent in sim.agents_targeting(wps[1]):
        visited.append(agent.id)
        agent.follow(route_b, start=wps[2])
    assert visited == original
    assert sim.agents_targeting(wps[1]) == ()
    assert len(sim.agents_targeting(wps[2])) == 3


def test_removed_agent_handle(world):
    sim, _, room, wps, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    agent_id = agent.id
    agent.remove()
    assert agent.alive
    sim.iterate()
    assert not agent.alive
    assert agent.id == agent_id
    for name in (
        "position",
        "route",
        "stage",
        "target",
        "route_orientation",
        "state",
        "stranded",
    ):
        with pytest.raises(RuntimeError, match=r"has left the simulation"):
            getattr(agent, name)
    for call in (
        agent.remove,
        lambda: agent.follow(route_a),
        lambda: agent.steer_to(room.at(2, 2)),
    ):
        with pytest.raises(RuntimeError, match=r"has left the simulation"):
            call()
    assert repr(agent) == f"Agent(id={agent_id})"


def test_route_from_other_simulation(world):
    sim, _, room, wps, _, route_a, _ = world
    _, _, _, owps, oexit = make_sim()
    other_route = Route([owps[0], oexit])
    with pytest.raises(
        ValueError, match=r"route belongs to another simulation"
    ):
        sim.add_agent(room.at(1, 8), route=other_route)
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(
        ValueError, match=r"route belongs to another simulation"
    ):
        agent.follow(other_route)
    with pytest.raises(
        ValueError, match=r"route belongs to another simulation"
    ):
        sim.agents_targeting(wps[0], route=other_route)


def test_location_from_other_surface_rejected(world):
    sim, _, room, _, _, route_a, _ = world
    _, _, other_room, _, _ = make_sim()
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(ValueError, match=r"belongs to another surface"):
        sim.add_agent(other_room.at(1, 8), route=route_a)
    with pytest.raises(ValueError, match=r"belongs to another surface"):
        sim.add_agent(room.at(1, 8), target=other_room.at(1, 8))
    with pytest.raises(ValueError, match=r"belongs to another surface"):
        agent.steer_to(other_room.at(1, 8))


def test_location_from_region_added_later_rejected(world):
    sim, surface, room, _, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    later = surface.add_region(
        polygon=shapely.Polygon([(20, 0), (30, 0), (30, 10), (20, 10)]),
        name="later",
    )
    place = Location(21.0, 5.0, later)
    msg = r"later is not part of this simulation's geometry"
    with pytest.raises(ValueError, match=msg):
        sim.add_agent(place, route=route_a)
    with pytest.raises(ValueError, match=msg):
        agent.steer_to(place)


def test_position_and_target_must_be_locations(world):
    sim, _, room, _, _, route_a, _ = world
    with pytest.raises(
        TypeError, match=r"position must be a Location, e.g. floor0.at\(x, y\)"
    ):
        sim.add_agent((1, 8), route=route_a)
    with pytest.raises(
        TypeError, match=r"target must be a Location, e.g. floor0.at\(x, y\)"
    ):
        sim.add_agent(room.at(1, 8), target=(1, 8))
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(TypeError, match=r"target must be a Location"):
        agent.steer_to((1, 8))


def test_route_and_target_exclusive(world):
    sim, _, room, _, _, route_a, _ = world
    with pytest.raises(ValueError, match=r"use route= or target=, not both"):
        sim.add_agent(room.at(1, 8), route=route_a, target=room.at(2, 2))


def test_pairs_route_needs_start(world):
    sim, _, room, wps, exit1, _, _ = world
    pairs = Route([(wps[0], wps[1]), (wps[1], exit1)])
    with pytest.raises(
        ValueError,
        match=r"route given as pairs has no default start; "
        r"pass start= here or to Route",
    ):
        sim.add_agent(room.at(1, 8), route=pairs)
    agent = sim.add_agent(room.at(1, 8), route=pairs, start=wps[0])
    assert agent.stage == wps[0]


def test_start_outside_route_rejected(world):
    sim, _, room, wps, _, route_a, _ = world
    with pytest.raises(ValueError, match=r"wp2 is not part of this route"):
        sim.add_agent(room.at(1, 8), route=route_a, start=wps[2])


def test_failed_follow_keeps_state(world):
    sim, _, room, wps, _, route_a, route_b = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(ValueError, match=r"wp0 is not part of this route"):
        agent.follow(route_b, start=wps[0])
    assert agent.route is route_a
    assert agent.stage == wps[0]


def test_failed_steer_to_keeps_state(world):
    sim, _, room, wps, _, route_a, _ = world
    _, _, other_room, _, _ = make_sim()
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(ValueError, match=r"belongs to another surface"):
        agent.steer_to(other_room.at(1, 8))
    assert agent.route is route_a
    assert agent.stage == wps[0]
    assert agent.target is None


def test_custom_model_needs_state():
    class Model(jps.CustomOperationalModel):
        def compute_next_state(self, *args, **kwargs):
            raise NotImplementedError

        def check_model_constraint(self, *args, **kwargs):
            return None

    sim, _, room, wps, exit1 = make_sim(Model())
    with pytest.raises(
        ValueError,
        match=r"custom models need state=; pass your model's agent state",
    ):
        sim.add_agent(room.at(1, 8), route=Route([wps[0], exit1]))


def test_explicit_state(world):
    sim, _, room, _, _, route_a, _ = world
    agent = sim.add_agent(
        room.at(1, 8),
        route=route_a,
        state=jps.CollisionFreeSpeedModelState(desired_speed=1.2),
    )
    assert agent.state.desired_speed == pytest.approx(1.2)


def test_attribute_assignment_points_to_methods(world):
    sim, _, room, _, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(AttributeError, match=r"use agent.follow"):
        agent.route = route_a
    with pytest.raises(AttributeError, match=r"use agent.steer_to"):
        agent.target = (1, 2)


def test_remove_twice_before_iterate(world):
    sim, _, room, _, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    agent.remove()
    agent.remove()
    assert agent.alive
    sim.iterate()
    assert not agent.alive
    assert sim.agent_count() == 0


@pytest.mark.parametrize("bad_id", ["1", 1.0, True, None])
def test_agent_lookup_rejects_ids_that_are_not_ints(world, bad_id):
    sim, _, room, _, _, route_a, _ = world
    with pytest.raises(TypeError, match="agent_id must be an int"):
        sim.agent(bad_id)


def test_agent_lookup_rejects_a_handle(world):
    sim, _, room, _, _, route_a, _ = world
    agent = sim.add_agent(room.at(1, 8), route=route_a)
    with pytest.raises(TypeError, match=r"sim.agent\(agent.id\)"):
        sim.agent(agent)
    assert sim.agent(agent.id) == agent


def test_simulation_rejects_native_surfaces():
    native = NativeSurface()
    native.add_region(boundary=[(0, 0), (10, 0), (10, 10), (0, 10)])
    for geometry in (native, native.create_geometry()):
        with pytest.raises(
            TypeError, match="geometry must be a WalkableSurface"
        ):
            jps.Simulation(
                model=jps.CollisionFreeSpeedModel(), geometry=geometry
            )
