# SPDX-License-Identifier: LGPL-3.0-or-later

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
    surface = jps.WalkableSurface()
    ground = surface.add_region(
        exterior=rect(0, 0, 20, 20), interior=[stairwell], height=GROUND_Z
    )
    upper = surface.add_region(
        exterior=rect(0, 0, 20, 20), interior=[stairwell], height=UPPER_Z
    )
    landing = surface.add_region(
        exterior=[(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)], height=1.5
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


def polygon_simulation():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry="POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))",
        dt=0.01,
    )


def test_polygon_input_still_builds_a_polygon_world():
    assert polygon_simulation().get_geometry().polygon(region_id=0).boundary()


def test_a_polygon_world_is_one_floor_at_height_zero():
    sim = polygon_simulation()
    assert sim.get_location(5.0, 5.0, region_id=0).z == 0.0
    with pytest.raises(jps.SimulationError, match="not in region 0"):
        sim.get_location(50.0, 50.0, region_id=0)


def journey_to_upper_exit(sim, upper):
    exit_id = sim.add_exit_stage(
        [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)], region_id=upper
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return journey_id, exit_id


def test_agents_spawn_on_either_of_two_stacked_floors():
    sim, ground, upper = u_stair()
    journey_id, exit_id = journey_to_upper_exit(sim, upper)
    for region_id in (ground, upper):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            region_id=region_id,
        )
    assert sim.agent_count() == 2


def test_a_region_id_picks_which_of_the_stacked_floors_a_location_is_on():
    sim, ground, upper = u_stair()
    downstairs = sim.get_location(*STACKED_XY, region_id=ground)
    upstairs = sim.get_location(*STACKED_XY, region_id=upper)
    assert (
        (downstairs.x, downstairs.y) == (upstairs.x, upstairs.y) == STACKED_XY
    )
    assert (downstairs.region_id, upstairs.region_id) == (ground, upper)
    assert downstairs.z == pytest.approx(GROUND_Z)
    assert upstairs.z == pytest.approx(UPPER_Z)


def test_a_location_off_the_given_region_is_refused():
    sim, ground, _ = u_stair()
    # In the stairwell: the ground floor has a hole there.
    with pytest.raises(jps.SimulationError, match="not in region"):
        sim.get_location(10.0, 7.5, region_id=ground)


def test_a_location_keeps_the_simulation_alive():
    sim, _, upper = u_stair()
    location = sim.get_location(*STACKED_XY, region_id=upper)
    del sim
    assert location.z == pytest.approx(UPPER_Z)


def test_a_location_reads_only_a_place():
    sim, _, upper = u_stair()
    location = sim.get_location(*STACKED_XY, region_id=upper)
    assert repr(location) == f"Location({location.x}, {location.y}, 3.0)"
    assert location.region_id == upper
    for hidden in ("face", "_geometry"):
        assert not hasattr(location, hidden)
    with pytest.raises(AttributeError):
        location.x = 0.0


def test_locations_are_the_native_type():
    sim, ground, upper = u_stair()
    journey_id, exit_id = journey_to_upper_exit(sim, upper)
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            region_id=ground,
        )
    )
    location = sim.get_location(*STACKED_XY, region_id=upper)
    assert type(location) is jps.Location
    assert type(agent.location) is jps.Location
    assert jps.Location.__module__ == "jupedsim.py_jupedsim"
    assert "A point on the walkable surface" in jps.Location.__doc__
    assert "Height of the surface" in jps.Location.z.__doc__


def test_an_agent_reports_the_floor_it_stands_on():
    sim, ground, upper = u_stair()
    journey_id, exit_id = journey_to_upper_exit(sim, upper)
    agents = {
        region_id: sim.agent(
            sim.add_agent(
                journey_id=journey_id,
                stage_id=exit_id,
                position=STACKED_XY,
                state=jps.CollisionFreeSpeedModelState(),
                region_id=region_id,
            )
        )
        for region_id in (ground, upper)
    }
    for region_id, z in ((ground, GROUND_Z), (upper, UPPER_Z)):
        agent = agents[region_id]
        assert agent.position == STACKED_XY
        assert agent.location.region_id == region_id
        assert agent.location.z == pytest.approx(z)
        # the 2D position and the location agree on where the agent is
        assert (agent.location.x, agent.location.y) == agent.position


def test_a_location_does_not_follow_the_agent_it_came_from():
    sim, ground, upper = u_stair()
    journey_id, exit_id = journey_to_upper_exit(sim, upper)
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            region_id=ground,
        )
    )
    where_it_started = agent.location
    sim.iterate(50)
    assert agent.position != pytest.approx(STACKED_XY)
    assert (where_it_started.x, where_it_started.y) == STACKED_XY


def test_a_direct_steering_target_can_be_given_as_a_location():
    sim, _, upper = u_stair()
    steering = sim.add_direct_steering_stage()
    journey_id = sim.add_journey(jps.JourneyDescription([steering]))
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=steering,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            region_id=upper,
        )
    )
    target = sim.get_location(6.0, 7.6, region_id=upper)
    agent.final_target = target
    final_target = agent.final_target
    assert final_target is not None
    assert (final_target.x, final_target.y, final_target.z) == (
        target.x,
        target.y,
        target.z,
    )
    sim.iterate(100)
    # it walks along its own floor towards the target, and stays up there
    assert agent.position[0] > STACKED_XY[0]
    assert agent.location.region_id == upper


def test_an_agent_heading_for_an_exit_has_no_final_target_location():
    sim = polygon_simulation()
    exit_id = sim.add_exit_stage([(8, 4), (9, 4), (9, 6), (8, 6)], region_id=0)
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(2.0, 5.0),
            state=jps.CollisionFreeSpeedModelState(),
            region_id=0,
        )
    )
    sim.iterate()
    assert agent.final_target is None
    assert agent.route_orientation == pytest.approx((1.0, 0.0))


def test_an_agent_reports_the_orientation_of_its_route():
    sim = polygon_simulation()
    steering = sim.add_direct_steering_stage()
    journey_id = sim.add_journey(jps.JourneyDescription([steering]))
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=steering,
            position=(2.0, 5.0),
            state=jps.CollisionFreeSpeedModelState(),
            region_id=0,
        )
    )
    agent.final_target = (8.0, 5.0)
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
    journey_id, exit_id = journey_to_upper_exit(sim, upper)
    agents = [
        sim.agent(
            sim.add_agent(
                journey_id=journey_id,
                stage_id=exit_id,
                position=position,
                state=state(),
                region_id=ground,
            )
        )
        for position in [
            (3.3, 7.6),
            (4.0, 7.6),
            (3.3, 8.3),
            (4.0, 8.3),
            (3.65, 6.9),
        ]
    ]
    watched, watched_id = agents[0], agents[0].id
    heights = [watched.location.z]
    for _ in range(6000):
        sim.iterate()
        if watched_id in sim.removed_agents():
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


def test_the_exit_an_agent_walks_towards_is_the_one_on_its_own_floor():
    # Same (x, y) polygon on both floors: without the region id the two exits would be one place.
    sim, ground, upper = u_stair()
    polygon = [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)]
    ground_exit = sim.add_exit_stage(polygon, region_id=ground)
    upper_exit = sim.add_exit_stage(polygon, region_id=upper)
    assert ground_exit != upper_exit

    journey_id = sim.add_journey(jps.JourneyDescription([ground_exit]))
    sim.add_agent(
        journey_id=journey_id,
        stage_id=ground_exit,
        position=STACKED_XY,
        state=jps.CollisionFreeSpeedModelState(),
        region_id=ground,
    )
    # A few metres away on its own floor, so it walks there and is gone.
    for _ in range(1000):
        sim.iterate()
        if sim.agent_count() == 0:
            break
    assert sim.agent_count() == 0
