# SPDX-License-Identifier: LGPL-3.0-or-later

import gc

import jupedsim as jps
import pytest

# The two stacked floors of the U-stair, and an (x, y) that both carry.
GROUND_Z = 0.0
UPPER_Z = 3.0
STACKED_XY = (3.3, 7.6)


def rect(x0, y0, x1, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


def u_stair(**simulation_args):
    """The U-stair of examples/example_3d.py, without the small holes.

    Returns a simulation on it, and the region ids of its ground and upper floor.
    """
    # (8, 9) and (13, 9) split the stairwell sides so each flight has an edge of its own.
    stairwell = [(8, 6), (14, 6), (14, 12), (8, 12), (8, 9)]
    surface = jps.GeometryBuilder()
    ground = surface.add_region(
        boundary=rect(0, 0, 20, 20), holes=[stairwell], height=GROUND_Z
    )
    upper = surface.add_region(
        boundary=rect(0, 0, 20, 20), holes=[stairwell], height=UPPER_Z
    )
    landing = surface.add_region(
        boundary=[(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)], height=1.5
    )
    surface.connect_regions(
        from_region=ground,
        from_edge=((8, 6), (8, 9)),
        to_region=landing,
        to_edge=((13, 6), (13, 9)),
    )
    surface.connect_regions(
        from_region=landing,
        from_edge=((13, 9), (13, 12)),
        to_region=upper,
        to_edge=((8, 9), (8, 12)),
    )
    args = {
        "model": jps.CollisionFreeSpeedModel(),
        "dt": 0.01,
    } | simulation_args
    return jps.Simulation(geometry=surface.build(), **args), ground, upper


def square_simulation():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=rect(0, 0, 10, 10), height=0.0)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface.build(), dt=0.01
    )
    return simulation, room


def route_to_upper_exit(sim, upper):
    exit_ = sim.add_target(
        jps.Exit(
            [([(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)], upper)]
        )
    )
    return sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)])


def test_agents_spawn_on_either_of_two_stacked_floors():
    sim, ground, upper = u_stair()
    route = route_to_upper_exit(sim, upper)
    for region_id in (ground, upper):
        sim.add_agent(
            region_id.at(*STACKED_XY),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        )
    assert sim.agent_count() == 2


def test_a_region_id_picks_which_of_the_stacked_floors_a_location_is_on():
    sim, ground, upper = u_stair()
    downstairs = ground.at(*STACKED_XY)
    upstairs = upper.at(*STACKED_XY)
    assert (
        (downstairs.x, downstairs.y) == (upstairs.x, upstairs.y) == STACKED_XY
    )
    assert (downstairs.region, upstairs.region) == (ground, upper)
    assert downstairs.z == pytest.approx(GROUND_Z)
    assert upstairs.z == pytest.approx(UPPER_Z)


def test_a_location_off_the_given_region_is_refused():
    sim, ground, _ = u_stair()
    # In the stairwell: the ground floor has a hole there.
    with pytest.raises(jps.SimulationError, match="not in region"):
        ground.at(10.0, 7.5)


def test_a_location_keeps_the_simulation_alive():
    sim, _, upper = u_stair()
    location = upper.at(*STACKED_XY)
    del sim
    assert location.z == pytest.approx(UPPER_Z)


def test_a_location_reads_only_a_place():
    sim, _, upper = u_stair()
    location = upper.at(*STACKED_XY)
    assert repr(location) == f"Location({location.x}, {location.y}, 3.0)"
    assert location.region == upper
    for hidden in ("face", "_geometry"):
        assert not hasattr(location, hidden)
    with pytest.raises(AttributeError):
        location.x = 0.0


def test_locations_are_the_native_type():
    sim, ground, upper = u_stair()
    route = route_to_upper_exit(sim, upper)
    agent = sim.add_agent(
        ground.at(*STACKED_XY),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    location = upper.at(*STACKED_XY)
    assert type(location) is jps.Location
    assert type(agent.location) is jps.Location
    assert jps.Location.__module__ == "jupedsim.pyjupedsim"
    assert "A point on the walkable surface" in jps.Location.__doc__
    assert "Height of the surface" in jps.Location.z.__doc__


def test_an_agent_reports_the_floor_it_stands_on():
    sim, ground, upper = u_stair()
    route = route_to_upper_exit(sim, upper)
    agents = {
        region_id: sim.add_agent(
            region_id.at(*STACKED_XY),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        )
        for region_id in (ground, upper)
    }
    for region_id, z in ((ground, GROUND_Z), (upper, UPPER_Z)):
        agent = agents[region_id]
        assert agent.location.xy == STACKED_XY
        assert agent.location.region == region_id
        assert agent.location.z == pytest.approx(z)
        # the 2D position and the location agree on where the agent is
        assert (agent.location.x, agent.location.y) == agent.location.xy


def test_a_location_does_not_follow_the_agent_it_came_from():
    sim, ground, upper = u_stair()
    route = route_to_upper_exit(sim, upper)
    agent = sim.add_agent(
        ground.at(*STACKED_XY),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    where_it_started = agent.location
    sim.iterate(50)
    assert agent.location.xy != pytest.approx(STACKED_XY)
    assert (where_it_started.x, where_it_started.y) == STACKED_XY


def test_an_agent_is_steered_to_a_location():
    sim, _, upper = u_stair()
    target = upper.at(6.0, 7.6)
    agent = sim.add_agent(upper.at(*STACKED_XY), steer_to=target)
    assert agent.route is None
    assert agent.current_target.xy == target.xy
    sim.iterate()
    assert agent.route_orientation == pytest.approx((1.0, 0.0))
    sim.iterate(100)
    # it walks along its own floor towards the target, and stays up there
    assert agent.location.x > STACKED_XY[0]
    assert agent.location.region == upper


# The models that carry a whole storey change today. The two force models
# (SocialForceModel, GeneralizedCentrifugalForceModel) stall at the stair's
# turn and are deliberately absent.
WALK_UP_MODELS = [
    (jps.CollisionFreeSpeedModel, jps.CollisionFreeSpeedModelState),
    (jps.CollisionFreeSpeedModelV2, jps.CollisionFreeSpeedModelV2State),
    (jps.WarpDriverModel, jps.WarpDriverModelState),
    (
        lambda: jps.AnticipationVelocityModel(rng_seed=1),
        jps.AnticipationVelocityModelState,
    ),
]


@pytest.mark.parametrize(
    "model, state",
    WALK_UP_MODELS,
    ids=["cfsm", "cfsm_v2", "warp_driver", "avm"],
)
def test_agents_walk_up_the_u_stair_to_an_exit_on_the_floor_above(model, state):
    # No waypoints on the way: the exit upstairs is the only stage, so the
    # route around the stair's turn is the router's to find.
    sim, ground, upper = u_stair(model=model())
    route = route_to_upper_exit(sim, upper)
    agents = [
        sim.add_agent(
            ground.at(*position),
            route=route,
            state=state(),
        )
        for position in [
            (3.3, 7.6),
            (4.0, 7.6),
            (3.3, 8.3),
            (4.0, 8.3),
            (3.65, 6.9),
        ]
    ]
    watched = agents[0]
    heights = [watched.location.z]
    for _ in range(6000):
        sim.iterate()
        if any(r.agent_id == agents[0].id for r in sim.removed_agents()):
            watched = None
        if watched is not None:
            heights.append(watched.location.z)
        if sim.agent_count() == 0:
            break

    assert sim.agent_count() == 0
    assert heights[0] == pytest.approx(GROUND_Z)
    assert heights[-1] == pytest.approx(UPPER_Z)
    # and it went up the stair rather than straight there: the landing is halfway
    assert any(1.0 < z < 2.0 for z in heights)


def test_one_exit_with_an_area_on_each_floor_is_reached_from_both():
    sim, ground, upper = u_stair()
    polygon = [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)]
    exit_ = sim.add_target(jps.Exit([(polygon, ground), (polygon, upper)]))
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)])
    floor_z = {
        sim.add_agent(
            region_id.at(*STACKED_XY),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        ): z
        for region_id, z in ((ground, GROUND_Z), (upper, UPPER_Z))
    }
    for _ in range(1000):
        sim.iterate()
        if sim.agent_count() == 0:
            break
        for agent in sim.agents():
            assert agent.location.z == pytest.approx(floor_z[agent])
    assert sim.agent_count() == 0


def test_an_exit_area_reaching_over_a_seam_stays_in_its_region():
    surface = jps.GeometryBuilder()
    ground = surface.add_region(boundary=rect(0, 0, 10, 10), height=0.0)
    platform = surface.add_region(boundary=rect(14, 0, 20, 10), height=1.0)
    ramp = surface.connect_regions(
        from_region=ground,
        from_edge=((10, 0), (10, 10)),
        to_region=platform,
        to_edge=((14, 0), (14, 10)),
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface.build(), dt=0.01
    )
    # The exit reaches 2 m onto the ramp, where the agent stands.
    exit_ = sim.add_target(jps.Exit([(rect(8, 4, 12, 6), ground)]))
    sim.add_agent(
        ramp.at(11, 5),
        route=sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)]),
        state=jps.CollisionFreeSpeedModelState(),
    )
    sim.iterate()
    assert sim.agent_count() == 1


def test_the_exit_an_agent_walks_towards_is_the_one_on_its_own_floor():
    # Same (x, y) polygon on both floors: without the region id the two exits would be one place.
    sim, ground, upper = u_stair()
    polygon = [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)]
    ground_exit = sim.add_target(jps.Exit([(polygon, ground)]))
    upper_exit = sim.add_target(jps.Exit([(polygon, upper)]))
    assert ground_exit != upper_exit

    route = sim.add_route(
        [jps.FixedTransition(jps.AGENT_LOCATION, ground_exit)]
    )
    agent = sim.add_agent(
        ground.at(*STACKED_XY),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    for _ in range(1000):
        sim.iterate()
        if sim.agent_count() == 0:
            break
        assert agent.location.z == pytest.approx(GROUND_Z)
    assert sim.agent_count() == 0


def test_leaving_the_with_block_closes_the_simulation():
    sim, _, _ = u_stair()
    with sim:
        sim.iterate()
    with pytest.raises(jps.SimulationError, match="in state Closed"):
        sim.iterate()


@pytest.mark.parametrize(
    "model, state",
    [
        (jps.WarpDriverModel, jps.WarpDriverModelState),
        (jps.AnticipationVelocityModel, jps.AnticipationVelocityModelState),
    ],
    ids=["warp_driver", "avm"],
)
def test_identically_set_up_simulations_stay_identical(model, state):
    # Agents walk head-on through a corridor, so that both models draw random numbers. Agent ids
    # count across simulations, so each run keeps its own and its agents are compared in the
    # order they were added. Any difference in the draws shows in the positions first.
    class Run:
        def __init__(self, seed):
            surface = jps.GeometryBuilder()
            self.room = surface.add_region(
                boundary=rect(0, 0, 20, 4), height=0.0
            )
            self.sim = jps.Simulation(
                model=model(rng_seed=seed), geometry=surface.build(), dt=0.05
            )
            self.routes = {}
            for heading, area in [
                (1, rect(19, 0, 20, 4)),
                (-1, rect(0, 0, 1, 4)),
            ]:
                exit_ = self.sim.add_target(jps.Exit([(area, self.room)]))
                self.routes[heading] = self.sim.add_route(
                    [jps.FixedTransition(jps.AGENT_LOCATION, exit_)]
                )
            self.agents = []

        def add_agent(self, position, heading):
            self.agents.append(
                self.sim.add_agent(
                    self.room.at(*position),
                    route=self.routes[heading],
                    state=state(),
                )
            )

        def positions(self):
            return [agent.location.xy for agent in self.agents]

    runs = [Run(seed=7), Run(seed=7), Run(seed=8)]
    first, second, reseeded = runs
    for y in (1.0, 2.0, 3.0):
        for position, heading in [((8.0, y), 1), ((12.0, y), -1)]:
            for run in runs:
                run.add_agent(position, heading)

    diverged = False
    for _ in range(100):
        for run in runs:
            run.sim.iterate()
        # Compared without tolerance.
        assert first.positions() == second.positions()
        diverged = diverged or first.positions() != reseeded.positions()
    # Otherwise no random number was drawn, and the comparison above shows nothing.
    assert diverged


def test_simulation_rejects_a_geometry_builder():
    builder = jps.GeometryBuilder()
    builder.add_region(boundary=rect(0, 0, 10, 10), height=0.0)
    with pytest.raises(TypeError):
        jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=builder)


def test_a_geometry_is_shared_not_consumed():
    builder = jps.GeometryBuilder()
    builder.add_region(boundary=rect(0, 0, 10, 10), height=0.0)
    geometry = builder.build()
    engine = jps.SurfaceMeshShortestPathRoutingEngine(geometry)
    with pytest.raises(jps.SimulationError):
        jps.Simulation(
            model=jps.CollisionFreeSpeedModel(),
            geometry=geometry,
            floorfield_options=jps.FloorfieldOptions(cell_size=-1),
        )
    first = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=geometry
    )
    second = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=geometry
    )
    del first, second
    gc.collect()
    path = engine.get_shortest_path((1, 1, 0), (9, 9, 0))
    assert path[0] == pytest.approx((1, 1, 0))
    assert path[-1] == pytest.approx((9, 9, 0))
