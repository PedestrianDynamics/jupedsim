# SPDX-License-Identifier: LGPL-3.0-or-later
import gc
import weakref

import jupedsim as jps
import pytest
import shapely


@pytest.fixture
def square_room_5x5():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        boundary=[(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)]
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    return simulation, room


@pytest.fixture
def square_room_5x5_with_obstacle():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
            [[(-0.5, -0.5), (-0.5, 0.5), (0.5, 0.5), (0.5, -0.5)]],
        )
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    return simulation, room


def test_exception_on_empty_polygon_in_exit_stage(square_room_5x5):
    sim, room = square_room_5x5
    with pytest.raises(Exception, match=r"Polygon must have at least 3 points"):
        sim.add_exit_stage([([], room)])


def test_exception_on_exit_without_areas(square_room_5x5):
    sim, _ = square_room_5x5
    with pytest.raises(
        jps.SimulationError, match="A stage needs at least one area"
    ):
        sim.add_exit_stage([])


def test_exception_on_exit_polygon_with_hole(square_room_5x5):
    sim, room = square_room_5x5
    exit_with_hole = shapely.Polygon(
        [(-2, -2), (2, -2), (2, 2), (-2, 2)],
        [[(-1, -1), (1, -1), (1, 1), (-1, 1)]],
    )
    with pytest.raises(ValueError, match="Polygon must not have holes"):
        sim.add_exit_stage([(exit_with_hole, room)])


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

    surface = jps.WalkableSurface()
    room = surface.add_region(polygon=polygon)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )

    exit_id = simulation.add_exit_stage(
        [([(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)], room)]
    )

    waypoint_id = simulation.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(9.5, 0), radius=1), room)]
    )

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
        region_id=room,
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
    simulation, room = square_room_5x5_with_obstacle
    waypoint_id = simulation.add_waypoint_stage(
        [(jps.circle_to_polygon(center=waypoint_position, radius=1), room)]
    )
    waypoint_stage = simulation.get_stage(waypoint_id)

    assert type(waypoint_stage) is jps.WaypointStage


def test_a_waypoint_given_by_its_area_is_reached(square_room_5x5):
    sim, room = square_room_5x5
    waypoint_id = sim.add_waypoint_stage(
        [([(1, -0.5), (2, -0.5), (2, 0.5), (1, 0.5)], room)]
    )
    exit_id = sim.add_exit_stage(
        [([(-2.5, -1), (-2, -1), (-2, 1), (-2.5, 1)], room)]
    )
    journey = jps.JourneyDescription([waypoint_id, exit_id])
    journey.set_transition_for_stage(
        waypoint_id, jps.Transition.create_fixed_transition(exit_id)
    )
    agent_id = sim.add_agent(
        journey_id=sim.add_journey(journey),
        stage_id=waypoint_id,
        position=(1.5, 0),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=room,
    )
    # It starts inside the waypoint, so the first step moves it on to the exit.
    sim.iterate()
    assert sim.agent(agent_id).stage_id == exit_id


def test_can_not_add_waypoint_outside_geometry(square_room_5x5):
    simulation, room = square_room_5x5

    with pytest.raises(
        jps.SimulationError, match=f"Area does not intersect region {room}"
    ):
        simulation.add_waypoint_stage(
            [(jps.circle_to_polygon(center=(10, 10), radius=1), room)]
        )


def test_can_not_add_exit_completely_outside_geometry(square_room_5x5):
    simulation, room = square_room_5x5

    with pytest.raises(
        jps.SimulationError, match=f"Area does not intersect region {room}"
    ):
        simulation.add_exit_stage(
            [([(-10, -10), (-8, -10), (-8, -8), (-10, -8)], room)]
        )


@pytest.mark.parametrize(
    "polygon",
    [
        [(-3, -3), (-3, -1), (-1, -1), (-1, -3)],
        [(-4, -4), (-4, -2), (-2, -2), (-2, -4)],
    ],
)
def test_can_add_exit_partly_outside_geometry(square_room_5x5, polygon):
    sim, room = square_room_5x5
    sim.add_exit_stage([(polygon, room)])


def test_an_agent_standing_at_the_edge_of_an_exit_leaves():
    # The exit's edge lies off the routing grid.
    surface = jps.WalkableSurface()
    room = surface.add_region(boundary=[(0, 0), (10, 0), (10, 10), (0, 10)])
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    exit_id = simulation.add_exit_stage(
        [([(8.05, 4), (9.5, 4), (9.5, 6), (8.05, 6)], room)]
    )
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))
    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(8.02, 5),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=room,
    )
    expected_iterations_to_finish = 10
    simulation.iterate(expected_iterations_to_finish)
    assert simulation.agent_count() == 0, (
        f"Expected agent to exit simulation within {expected_iterations_to_finish} iterations."
    )


def test_get_stage_returns_the_native_stage_types(square_room_5x5):
    sim, room = square_room_5x5
    waypoint_id = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(0, 0), radius=1), room)]
    )
    exit_id = sim.add_exit_stage(
        [([(2, -1), (2.5, -1), (2.5, 1), (2, 1)], room)]
    )
    steering_id = sim.add_direct_steering_stage()

    waypoint = sim.get_stage(waypoint_id)
    exit = sim.get_stage(exit_id)
    steering = sim.get_stage(steering_id)

    assert type(waypoint) is jps.WaypointStage
    assert type(exit) is jps.ExitStage
    assert type(steering) is jps.DirectSteeringStage
    assert jps.WaypointStage.__module__ == "jupedsim.pyjupedsim"
    assert "Models a waypoint" in jps.WaypointStage.__doc__
    assert "Models an exit" in jps.ExitStage.__doc__
    assert "direct control of the target" in jps.DirectSteeringStage.__doc__
    assert steering.count_targeting() == 0


def test_a_stage_keeps_its_simulation_alive():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        boundary=[(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)]
    )
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
    stage = sim.get_stage(
        sim.add_waypoint_stage(
            [(jps.circle_to_polygon(center=(0, 0), radius=1), room)]
        )
    )
    native = weakref.ref(sim._obj)
    del sim
    gc.collect()
    assert native() is not None
    assert stage.count_targeting() == 0


@pytest.mark.parametrize("bad_id", [-1, 1.5, "3", 2**64])
def test_get_stage_rejects_ids_of_the_wrong_type(square_room_5x5, bad_id):
    sim, _ = square_room_5x5
    with pytest.raises(TypeError):
        sim.get_stage(bad_id)
