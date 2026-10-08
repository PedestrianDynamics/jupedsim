# SPDX-License-Identifier: LGPL-3.0-or-later
import gc
import weakref

import jupedsim as jps
import pytest
import shapely


@pytest.fixture
def square_room_5x5():
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
    )
    return simulation


@pytest.fixture
def square_room_5x5_with_obstacle():
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
            [[(-0.5, -0.5), (-0.5, 0.5), (0.5, 0.5), (0.5, -0.5)]],
        ),
    )
    return simulation


def test_exception_on_empty_polygon_in_exit_stage(square_room_5x5):
    sim = square_room_5x5
    with pytest.raises(Exception, match=r"Polygon must have at least 3 points"):
        sim.add_exit_stage([], region_id=0)


def test_can_use_stage_proxy():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    polygon = shapely.union(
        shapely.Polygon([(-10, 2.5), (-10, -2.5), (10, -2.5), (10, 2.5)]),
        shapely.Polygon([(-2.5, 2.5), (-2.5, -10), (2.5, -10), (2.5, 2.5)]),
    )

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=polygon
    )

    exit_id = simulation.add_exit_stage(
        [(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)],
        region_id=0,
    )

    waypoint_id = simulation.add_waypoint_stage((9.5, 0), 1, region_id=0)

    exit_journey_id = simulation.add_journey(
        jps.JourneyDescription(
            [
                exit_id,
            ]
        )
    )

    entry_journey_id = simulation.add_journey(
        jps.JourneyDescription(
            [
                waypoint_id,
            ]
        )
    )

    exit = simulation.get_stage(exit_id)
    waypoint = simulation.get_stage(waypoint_id)

    assert exit.count_targeting() == 0
    assert waypoint.count_targeting() == 0

    agent_id = simulation.add_agent(
        journey_id=exit_journey_id,
        stage_id=exit_id,
        position=(-9.5, 0),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=0,
    )

    assert exit.count_targeting() == 1
    assert waypoint.count_targeting() == 0

    simulation.iterate()

    assert exit.count_targeting() == 1
    assert waypoint.count_targeting() == 0

    simulation.switch_agent_journey(
        agent_id=agent_id, journey_id=entry_journey_id, stage_id=waypoint_id
    )

    assert exit.count_targeting() == 0
    assert waypoint.count_targeting() == 1

    simulation.iterate()

    assert exit.count_targeting() == 0
    assert waypoint.count_targeting() == 1


@pytest.mark.parametrize(
    "waypoint_position",
    [(-2.5, -2.5), (-2, -2), (2, 2), (2.5, 2.5), (-0.5, -0.5), (0.5, 0.5)],
)
def test_can_add_waypoint(square_room_5x5_with_obstacle, waypoint_position):
    simulation = square_room_5x5_with_obstacle
    waypoint_id = simulation.add_waypoint_stage(
        waypoint_position, 1, region_id=0
    )
    waypoint_stage = simulation.get_stage(waypoint_id)

    assert type(waypoint_stage) is jps.WaypointStage


def test_can_not_add_waypoint_outside_geometry(square_room_5x5):
    simulation = square_room_5x5

    with pytest.raises(
        jps.SimulationError, match="Area does not intersect region 0"
    ):
        simulation.add_waypoint_stage((10, 10), 1, region_id=0)


def test_can_not_add_exit_completely_outside_geometry(square_room_5x5):
    simulation = square_room_5x5

    with pytest.raises(
        jps.SimulationError, match="Area does not intersect region 0"
    ):
        simulation.add_exit_stage(
            [(-10, -10), (-8, -10), (-8, -8), (-10, -8)], region_id=0
        )


@pytest.mark.parametrize(
    "polygon",
    [
        [(-3, -3), (-3, -1), (-1, -1), (-1, -3)],
        [(-4, -4), (-4, -2), (-2, -2), (-2, -4)],
    ],
)
def test_can_add_exit_partly_outside_geometry(square_room_5x5, polygon):
    square_room_5x5.add_exit_stage(polygon, region_id=0)


def test_an_agent_standing_at_the_edge_of_an_exit_leaves():
    # The exit's edge lies off the routing grid.
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )
    exit_id = simulation.add_exit_stage(
        [(8.05, 4), (9.5, 4), (9.5, 6), (8.05, 6)], region_id=0
    )
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))
    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(8.02, 5),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=0,
    )
    expected_iterations_to_finish = 10
    simulation.iterate(expected_iterations_to_finish)
    assert simulation.agent_count() == 0, (
        f"Expected agent to exit simulation within {expected_iterations_to_finish} iterations."
    )


def test_get_stage_returns_the_native_stage_types(square_room_5x5):
    sim = square_room_5x5
    waypoint_id = sim.add_waypoint_stage((0, 0), 1, region_id=0)
    exit_id = sim.add_exit_stage(
        [(2, -1), (2.5, -1), (2.5, 1), (2, 1)], region_id=0
    )
    steering_id = sim.add_direct_steering_stage()

    waypoint = sim.get_stage(waypoint_id)
    exit = sim.get_stage(exit_id)
    steering = sim.get_stage(steering_id)

    assert type(waypoint) is jps.WaypointStage
    assert type(exit) is jps.ExitStage
    assert type(steering) is jps.DirectSteeringStage
    assert jps.WaypointStage.__module__ == "jupedsim.py_jupedsim"
    assert "Models a waypoint" in jps.WaypointStage.__doc__
    assert "Models an exit" in jps.ExitStage.__doc__
    assert "direct control of the target" in jps.DirectSteeringStage.__doc__
    assert steering.count_targeting() == 0


def test_a_stage_keeps_its_simulation_alive():
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
    )
    stage = sim.get_stage(sim.add_waypoint_stage((0, 0), 1, region_id=0))
    native = weakref.ref(sim._obj)
    del sim
    gc.collect()
    assert native() is not None
    assert stage.count_targeting() == 0


@pytest.mark.parametrize("bad_id", [-1, 1.5, "3", 2**64])
def test_get_stage_rejects_ids_of_the_wrong_type(square_room_5x5, bad_id):
    with pytest.raises(TypeError):
        square_room_5x5.get_stage(bad_id)
