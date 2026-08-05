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


def test_polygon_input_still_builds_a_polygon_world():
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry="POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))",
        dt=0.01,
    )
    assert sim.get_geometry().boundary()


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
