# SPDX-License-Identifier: LGPL-3.0-or-later
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
        sim.add_exit_stage([])


def test_can_share_queue_between_stages():
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

    wp_j1 = simulation.add_waypoint_stage((-1, 0), 0.5)

    common_exit = simulation.add_exit_stage(
        [(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)]
    )

    common_queue = simulation.add_queue_stage(
        [(0, -9), (0, -8), (0, -7), (0, -6), (0, -5), (0, -4)]
    )
    queue = simulation.get_stage(common_queue)

    journey1 = jps.JourneyDescription(
        [
            wp_j1,
            common_queue,
            common_exit,
        ]
    )
    journey1.set_transition_for_stage(
        wp_j1, jps.Transition.create_fixed_transition(common_queue)
    )
    journey1.set_transition_for_stage(
        common_queue, jps.Transition.create_fixed_transition(common_exit)
    )
    wp_j2 = simulation.add_waypoint_stage((1, 0), 0.5)
    journey2 = jps.JourneyDescription([wp_j2, common_queue, common_exit])
    journey2.set_transition_for_stage(
        wp_j2, jps.Transition.create_fixed_transition(common_queue)
    )
    journey2.set_transition_for_stage(
        common_queue, jps.Transition.create_fixed_transition(common_exit)
    )

    journeys = [
        (simulation.add_journey(journey1), wp_j1),
        (simulation.add_journey(journey2), wp_j2),
    ]

    agents = [
        ((-9.5, 0), journeys[0]),
        ((-8.5, 0), journeys[0]),
        ((-7.5, 0), journeys[0]),
        ((-6.5, 0), journeys[0]),
        ((-5.5, 0), journeys[0]),
        ((9.5, 0), journeys[1]),
        ((8.5, 0), journeys[1]),
        ((7.5, 0), journeys[1]),
        ((6.5, 0), journeys[1]),
        ((5.5, 0), journeys[1]),
    ]

    agent_parameters = jps.CollisionFreeSpeedModelAgentParameters()

    for pos, (journey_id, stage_id) in agents:
        agent_parameters.position = pos
        agent_parameters.journey_id = journey_id
        agent_parameters.stage_id = stage_id
        simulation.add_agent(agent_parameters)

    while simulation.agent_count() > 0:
        if simulation.iteration_count() % 4 == 0:
            pass
        if (
            max(simulation.iteration_count() - 3000, 0) > 0
            and simulation.iteration_count() % 200 == 0
        ):
            queue.pop(1)
        simulation.iterate()


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
        [(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)]
    )

    waypoint_id = simulation.add_waypoint_stage((9.5, 0), 1)

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

    agent_parameters = jps.CollisionFreeSpeedModelAgentParameters()
    agent_parameters.position = (-9.5, 0)
    agent_parameters.journey_id = exit_journey_id
    agent_parameters.stage_id = exit_id
    agent_id = simulation.add_agent(agent_parameters)

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
    waypoint_id = simulation.add_waypoint_stage(waypoint_position, 1)
    waypoint_stage = simulation.get_stage(waypoint_id)

    assert type(waypoint_stage) is jps.WaypointStage


def test_can_not_add_waypoint_outside_geometry(square_room_5x5):
    simulation = square_room_5x5

    with pytest.raises(
        RuntimeError, match="WayPoint .* not inside walkable area"
    ):
        simulation.add_waypoint_stage((10, 10), 1)


def test_can_not_add_exit_completely_outside_geometry(square_room_5x5):
    simulation = square_room_5x5

    with pytest.raises(RuntimeError, match=r"Exit .* not inside walkable area"):
        simulation.add_exit_stage([(-10, -10), (-8, -10), (-8, -8), (-10, -8)])


def test_can_add_exit_partly_outside_geometry_centroid_inside(square_room_5x5):
    simulation = square_room_5x5
    simulation.add_exit_stage([(-3, -3), (-3, -1), (-1, -1), (-1, -3)])


def test_can_not_add_exit_partly_outside_geometry_centroid_outside(
    square_room_5x5,
):
    simulation = square_room_5x5

    with pytest.raises(RuntimeError, match=r"Exit .* not inside walkable area"):
        simulation.add_exit_stage([(-4, -4), (-4, -2), (-2, -2), (-2, -4)])


def test_can_not_add_notifiable_waiting_set_outside_geometry(square_room_5x5):
    simulation = square_room_5x5

    with pytest.raises(
        RuntimeError,
        match=r"NotifiableWaitingSet point .* not inside walkable area",
    ):
        simulation.add_waiting_set_stage([(2, -2), (-10, -10)])


def test_can_not_add_notifiable_queue_outside_geometry(square_room_5x5):
    simulation = square_room_5x5

    with pytest.raises(
        RuntimeError,
        match=r"NotifiableQueue point .* not inside walkable area",
    ):
        simulation.add_queue_stage([(2, -2), (-10, -10)])
