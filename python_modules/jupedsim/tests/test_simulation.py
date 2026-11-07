# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim as jps
import pytest
import shapely

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
    surface = jps.WalkableSurface()
    ground = surface.add_region(
        polygon=shapely.Polygon(rect(0, 0, 20, 20), [stairwell]),
        height=GROUND_Z,
    )
    upper = surface.add_region(
        polygon=shapely.Polygon(rect(0, 0, 20, 20), [stairwell]), height=UPPER_Z
    )
    landing = surface.add_region(
        polygon=shapely.Polygon(
            [(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)]
        ),
        height=1.5,
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
    return jps.Simulation(geometry=surface, **args), ground, upper


def square_simulation():
    surface = jps.WalkableSurface()
    room = surface.add_region(polygon=shapely.Polygon(rect(0, 0, 10, 10)))
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface, dt=0.01
    )
    return simulation, room


def journey_to_upper_exit(sim, upper):
    exit = sim.add_exit(
        [([(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)], upper)]
    )
    route = jps.Route([exit])
    return route, exit


def test_agents_spawn_on_either_of_two_stacked_floors():
    sim, ground, upper = u_stair()
    route, exit = journey_to_upper_exit(sim, upper)
    for region in (ground, upper):
        sim.add_agent(
            region.at(*STACKED_XY),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        )
    assert sim.agent_count() == 2


def test_a_region_picks_which_of_the_stacked_floors_a_location_is_on():
    sim, ground, upper = u_stair()
    downstairs = ground.at(*STACKED_XY)
    upstairs = upper.at(*STACKED_XY)
    assert (
        (downstairs.x, downstairs.y) == (upstairs.x, upstairs.y) == STACKED_XY
    )
    assert (downstairs.region, upstairs.region) == (ground, upper)
    assert downstairs.region.height == pytest.approx(GROUND_Z)
    assert upstairs.region.height == pytest.approx(UPPER_Z)


def test_a_location_off_the_given_region_is_refused():
    sim, ground, _ = u_stair()
    # In the stairwell: the ground floor has a hole there.
    with pytest.raises(ValueError, match="not in"):
        ground.at(10.0, 7.5)


def test_a_location_reads_only_a_place():
    sim, _, upper = u_stair()
    location = upper.at(*STACKED_XY)
    assert repr(location) == (
        f"Location({location.x}, {location.y}, region={upper.label})"
    )
    assert location.region == upper
    for hidden in ("face", "_geometry"):
        assert not hasattr(location, hidden)
    with pytest.raises(AttributeError):
        location.x = 0.0


def test_locations_are_the_surface_type():
    sim, ground, upper = u_stair()
    route, exit = journey_to_upper_exit(sim, upper)
    agent = sim.add_agent(
        ground.at(*STACKED_XY),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    location = upper.at(*STACKED_XY)
    assert type(location) is jps.Location
    assert type(agent.position) is jps.Location
    assert jps.Location.__module__ == "jupedsim.surface"
    assert "A point on a region" in jps.Location.__doc__


def test_an_agent_reports_the_floor_it_stands_on():
    sim, ground, upper = u_stair()
    route, exit = journey_to_upper_exit(sim, upper)
    agents = {
        region: sim.add_agent(
            region.at(*STACKED_XY),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        )
        for region in (ground, upper)
    }
    for region, z in ((ground, GROUND_Z), (upper, UPPER_Z)):
        agent = agents[region]
        assert agent.position.xy == STACKED_XY
        assert agent.position.region == region
        assert agent.position.region.height == pytest.approx(z)


def test_a_location_does_not_follow_the_agent_it_came_from():
    sim, ground, upper = u_stair()
    route, exit = journey_to_upper_exit(sim, upper)
    agent = sim.add_agent(
        ground.at(*STACKED_XY),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    where_it_started = agent.position
    sim.iterate(50)
    assert agent.position.xy != pytest.approx(STACKED_XY)
    assert (where_it_started.x, where_it_started.y) == STACKED_XY


def test_a_direct_steering_target_can_be_given_as_a_location():
    sim, _, upper = u_stair()
    target = upper.at(6.0, 7.6)
    agent = sim.add_agent(
        upper.at(*STACKED_XY),
        target=target,
        state=jps.CollisionFreeSpeedModelState(),
    )
    assert agent.target == target
    sim.iterate(100)
    # it walks along its own floor towards the target, and stays up there
    assert agent.position.x > STACKED_XY[0]
    assert agent.position.region == upper


def test_an_agent_heading_for_an_exit_has_no_target_location():
    sim, room = square_simulation()
    exit = sim.add_exit([([(8, 4), (9, 4), (9, 6), (8, 6)], room)])
    route = jps.Route([exit])
    agent = sim.add_agent(
        room.at(2.0, 5.0),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    sim.iterate()
    assert agent.target is None
    assert agent.route_orientation == pytest.approx((1.0, 0.0))


def test_an_agent_reports_the_orientation_of_its_route():
    sim, room = square_simulation()
    agent = sim.add_agent(
        room.at(2.0, 5.0),
        target=room.at(8.0, 5.0),
        state=jps.CollisionFreeSpeedModelState(),
    )
    sim.iterate()
    assert agent.route_orientation == pytest.approx((1.0, 0.0))


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
    route, exit = journey_to_upper_exit(sim, upper)
    agents = [
        sim.add_agent(ground.at(*position), route=route, state=state())
        for position in [
            (3.3, 7.6),
            (4.0, 7.6),
            (3.3, 8.3),
            (4.0, 8.3),
            (3.65, 6.9),
        ]
    ]
    watched, watched_id = agents[0], agents[0].id
    regions = [watched.position.region]
    heights = [sim._obj.agent(watched_id).location.z]
    for _ in range(6000):
        sim.iterate()
        if any(r.id == watched_id for r in sim.removed_agents()):
            watched = None
        if watched is not None:
            regions.append(watched.position.region)
            heights.append(sim._obj.agent(watched_id).location.z)
        if sim.agent_count() == 0:
            break

    assert sim.agent_count() == 0
    assert regions[0].height == pytest.approx(GROUND_Z)
    assert regions[-1].height == pytest.approx(UPPER_Z)
    # and it went up the stair rather than straight there: it crossed regions
    # that are neither of the two floors
    assert any(r not in (ground, upper) for r in regions)
    landing = sim._surface.regions[2]
    assert landing in regions
    assert any(1.0 < z < 2.0 for z in heights)


def test_one_exit_with_an_area_on_each_floor_is_reached_from_both():
    sim, ground, upper = u_stair()
    polygon = [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)]
    exit = sim.add_exit([(polygon, ground), (polygon, upper)])
    route = jps.Route([exit])
    floor_z = {
        sim.add_agent(
            region.at(*STACKED_XY),
            route=route,
            state=jps.CollisionFreeSpeedModelState(),
        ).id: z
        for region, z in ((ground, GROUND_Z), (upper, UPPER_Z))
    }
    for _ in range(1000):
        sim.iterate()
        if sim.agent_count() == 0:
            break
        for agent in sim.agents():
            assert agent.position.region.height == pytest.approx(
                floor_z[agent.id]
            )
    assert sim.agent_count() == 0


def test_an_exit_area_reaching_over_a_seam_stays_in_its_region():
    surface = jps.WalkableSurface()
    ground = surface.add_region(polygon=shapely.Polygon(rect(0, 0, 10, 10)))
    platform = surface.add_region(
        polygon=shapely.Polygon(rect(14, 0, 20, 10)), height=1.0
    )
    ramp = surface.connect_regions(
        from_region=ground,
        from_edge=((10, 0), (10, 10)),
        to_region=platform,
        to_edge=((14, 0), (14, 10)),
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface, dt=0.01
    )
    # The exit reaches 2 m onto the ramp, where the agent stands.
    exit = sim.add_exit([(rect(8, 4, 12, 6), ground)])
    sim.add_agent(
        ramp.at(11, 5),
        route=jps.Route([exit]),
        state=jps.CollisionFreeSpeedModelState(),
    )
    sim.iterate()
    assert sim.agent_count() == 1


def test_the_exit_an_agent_walks_towards_is_the_one_on_its_own_floor():
    # Same (x, y) polygon on both floors: without the region id the two exits would be one place.
    sim, ground, upper = u_stair()
    polygon = [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)]
    ground_exit = sim.add_exit([(polygon, ground)])
    upper_exit = sim.add_exit([(polygon, upper)])
    assert ground_exit != upper_exit

    route = jps.Route([ground_exit])
    agent = sim.add_agent(
        ground.at(*STACKED_XY),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    for _ in range(1000):
        sim.iterate()
        if sim.agent_count() == 0:
            break
        assert agent.position.region.height == pytest.approx(GROUND_Z)
    assert sim.agent_count() == 0


def test_leaving_the_with_block_closes_the_simulation():
    sim, _, _ = u_stair()
    with sim:
        sim.iterate()
    with pytest.raises(jps.SimulationError, match="in state Closed"):
        sim.iterate()
