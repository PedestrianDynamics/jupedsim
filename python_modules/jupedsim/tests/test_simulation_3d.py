# SPDX-License-Identifier: LGPL-3.0-or-later
from pathlib import Path

import jupedsim as jps
import pytest

OBJ = Path(__file__).parents[3] / "examples/geometry/multi_level_u_stair.obj"

# The two stacked floors of the U-stair fixture, and an (x, y) that both carry.
GROUND_Z = 0.0
UPPER_Z = 3.0
STACKED_XY = (3.3, 7.6)


def mesh_simulation(geometry: str | Path = OBJ):
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=geometry, dt=0.01
    )


def test_simulation_can_be_built_from_an_obj_path():
    sim = mesh_simulation()
    assert sim.agent_count() == 0


def test_a_string_naming_an_obj_file_is_a_mesh_not_a_wkt():
    sim = mesh_simulation(str(OBJ))
    assert sim.agent_count() == 0


def test_a_mesh_built_simulation_has_no_polygon_to_hand_out():
    sim = mesh_simulation()
    with pytest.raises(jps.SimulationError, match="surface mesh"):
        sim.get_geometry()


def polygon_simulation():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry="POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))",
        dt=0.01,
    )


def test_polygon_input_still_builds_a_polygon_world():
    assert polygon_simulation().get_geometry().boundary()


def test_a_polygon_world_is_one_floor_at_height_zero():
    sim = polygon_simulation()
    assert sim.get_location(5.0, 5.0).z == 0.0
    with pytest.raises(jps.SimulationError, match="accessible area"):
        sim.get_location(50.0, 50.0)


def journey_to_upper_exit(sim):
    exit_id = sim.add_exit_stage(
        [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)], z_hint=UPPER_Z
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return journey_id, exit_id


def test_agents_spawn_on_either_of_two_stacked_floors():
    sim = mesh_simulation()
    journey_id, exit_id = journey_to_upper_exit(sim)
    for z_hint in (GROUND_Z, UPPER_Z):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            z_hint=z_hint,
        )
    assert sim.agent_count() == 2


def test_a_z_hint_between_the_floors_lands_on_neither():
    sim = mesh_simulation()
    journey_id, exit_id = journey_to_upper_exit(sim)
    with pytest.raises(jps.SimulationError, match="walkable area"):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            z_hint=1.5,
        )


def test_a_stage_between_the_floors_lands_on_neither():
    sim = mesh_simulation()
    with pytest.raises(jps.SimulationError):
        sim.add_waypoint_stage(STACKED_XY, 0.5, z_hint=1.5)


def test_a_z_hint_picks_which_of_the_stacked_floors_a_location_is_on():
    sim = mesh_simulation()
    ground = sim.get_location(*STACKED_XY, z_hint=GROUND_Z)
    upper = sim.get_location(*STACKED_XY, z_hint=UPPER_Z)
    assert (ground.x, ground.y) == (upper.x, upper.y) == STACKED_XY
    assert ground.z == pytest.approx(GROUND_Z)
    assert upper.z == pytest.approx(UPPER_Z)


def test_a_location_between_the_floors_is_on_neither():
    sim = mesh_simulation()
    with pytest.raises(jps.SimulationError, match="accessible area"):
        sim.get_location(*STACKED_XY, z_hint=1.5)


def test_a_location_keeps_the_simulation_alive():
    location = mesh_simulation().get_location(*STACKED_XY, z_hint=UPPER_Z)
    assert location.z == pytest.approx(UPPER_Z)


def test_a_location_reads_only_a_place():
    sim = mesh_simulation()
    location = sim.get_location(*STACKED_XY, z_hint=UPPER_Z)
    assert repr(location) == f"Location({location.x}, {location.y}, 3.0)"
    for hidden in ("region", "face", "_geometry"):
        assert not hasattr(location, hidden)
    with pytest.raises(AttributeError):
        location.x = 0.0


def test_an_agent_reports_the_floor_it_stands_on():
    sim = mesh_simulation()
    journey_id, exit_id = journey_to_upper_exit(sim)
    agents = {
        z_hint: sim.agent(
            sim.add_agent(
                journey_id=journey_id,
                stage_id=exit_id,
                position=STACKED_XY,
                state=jps.CollisionFreeSpeedModelState(),
                z_hint=z_hint,
            )
        )
        for z_hint in (GROUND_Z, UPPER_Z)
    }
    for z_hint, agent in agents.items():
        assert agent.position == STACKED_XY
        assert agent.location.z == pytest.approx(z_hint)
        # the 2D position and the location agree on where the agent is
        assert (agent.location.x, agent.location.y) == agent.position


def test_a_location_does_not_follow_the_agent_it_came_from():
    sim = mesh_simulation()
    journey_id, exit_id = journey_to_upper_exit(sim)
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            z_hint=GROUND_Z,
        )
    )
    where_it_started = agent.location
    sim.iterate(50)
    assert agent.position != pytest.approx(STACKED_XY)
    assert (where_it_started.x, where_it_started.y) == STACKED_XY


def test_a_direct_steering_target_can_be_given_as_a_location():
    sim = mesh_simulation()
    steering = sim.add_direct_steering_stage()
    journey_id = sim.add_journey(jps.JourneyDescription([steering]))
    agent = sim.agent(
        sim.add_agent(
            journey_id=journey_id,
            stage_id=steering,
            position=STACKED_XY,
            state=jps.CollisionFreeSpeedModelState(),
            z_hint=UPPER_Z,
        )
    )
    target = sim.get_location(6.0, 7.6, z_hint=UPPER_Z)
    agent.final_target = target
    assert agent.final_target == pytest.approx((target.x, target.y))
    sim.iterate(100)
    # it walks along its own floor towards the target, and stays up there
    assert agent.position[0] > STACKED_XY[0]
    assert agent.location.z == pytest.approx(UPPER_Z)


def test_the_exit_an_agent_walks_towards_is_the_one_on_its_own_floor():
    # Same (x, y) polygon on both floors: without the hint the two exits would be one place.
    sim = mesh_simulation()
    polygon = [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)]
    ground_exit = sim.add_exit_stage(polygon, z_hint=GROUND_Z)
    upper_exit = sim.add_exit_stage(polygon, z_hint=UPPER_Z)
    assert ground_exit != upper_exit

    journey_id = sim.add_journey(jps.JourneyDescription([ground_exit]))
    sim.add_agent(
        journey_id=journey_id,
        stage_id=ground_exit,
        position=STACKED_XY,
        state=jps.CollisionFreeSpeedModelState(),
        z_hint=GROUND_Z,
    )
    # A few metres away on its own floor, so it walks there and is gone.
    for _ in range(1000):
        sim.iterate()
        if sim.agent_count() == 0:
            break
    assert sim.agent_count() == 0
