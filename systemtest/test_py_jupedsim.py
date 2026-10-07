# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely


def test_can_query_agents_in_range():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )
    exit = simulation.add_exit_stage(
        [(99, 45), (99, 55), (100, 55), (100, 45)], region_id=0
    )

    journey = jps.JourneyDescription([exit])

    journey_id = simulation.add_journey(journey)

    initial_agent_positions = [
        (10, 10),
        (20, 10),
        (30, 10),
        (40, 10),
        (50, 10),
    ]

    expected_agent_ids = list()

    for new_pos, id in zip(
        initial_agent_positions,
        range(10, 10 + len(initial_agent_positions)),
    ):
        expected_agent_ids.append(
            simulation.add_agent(
                journey_id=journey_id,
                stage_id=exit,
                position=new_pos,
                state=jps.CollisionFreeSpeedModelState(),
                region_id=0,
            )
        )

    actual_ids_in_range = [
        agent.id
        for agent in simulation.agents_in_range(initial_agent_positions[2], 10)
    ]

    actual_ids_in_polygon = [
        agent.id
        for agent in simulation.agents_in_polygon(
            [(39, 11), (39, 9), (51, 9), (51, 11)]
        )
    ]

    assert actual_ids_in_range == [
        expected_agent_ids[1],
        expected_agent_ids[2],
        expected_agent_ids[3],
    ]
    assert actual_ids_in_polygon == [
        expected_agent_ids[3],
        expected_agent_ids[4],
    ]


def test_can_run_simulation():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    # jps.set_debug_callback(log_msg_handler)
    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    p1 = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = shapely.Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=p1.union(p2)
    )
    exit_stage_id = simulation.add_exit_stage(
        [(18, 4), (20, 4), (20, 6), (18, 6)],
        region_id=0,
    )

    journey = jps.JourneyDescription([exit_stage_id])

    journey_id = simulation.add_journey(journey)

    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        expected_agent_ids.add(
            simulation.add_agent(
                journey_id=journey_id,
                stage_id=exit_stage_id,
                position=new_pos,
                state=jps.CollisionFreeSpeedModelState(),
                region_id=0,
            )
        )

    actual_agent_ids = {agent.id for agent in simulation.agents()}

    assert actual_agent_ids == expected_agent_ids

    agent_id = simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_stage_id,
        position=(6, 6),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=0,
    )

    for actual, expected in zip(simulation.agents(), initial_agent_positions):
        assert actual.position == expected

    simulation.mark_agent_for_removal(agent_id)
    simulation.iterate()

    with pytest.raises(jps.SimulationError, match=r"Unknown agent id \d+"):
        simulation.mark_agent_for_removal(agent_id)

    while simulation.agent_count() > 0:
        simulation.iterate()
        assert simulation.iteration_count() < 2000


def test_get_single_agent_from_simulation():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    # jps.set_debug_callback(log_msg_handler)
    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    p1 = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = shapely.Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=p1.union(p2)
    )
    exit_id = simulation.add_exit_stage(
        [(18, 4), (20, 4), (20, 6), (18, 6)], region_id=0
    )

    journey = jps.JourneyDescription([exit_id])

    journey_id = simulation.add_journey(journey)
    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    agent_ids = set()

    for new_pos in initial_agent_positions:
        agent_ids.add(
            simulation.add_agent(
                journey_id=journey_id,
                stage_id=exit_id,
                position=new_pos,
                state=jps.CollisionFreeSpeedModelState(),
                region_id=0,
            )
        )

    for agent_id in agent_ids:
        assert simulation.agent(agent_id).id == agent_id


def test_get_agent_non_existing_agent_from_simulation():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    # jps.set_debug_callback(log_msg_handler)
    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    p1 = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = shapely.Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=p1.union(p2)
    )

    exit_id = simulation.add_exit_stage(
        [(18, 4), (20, 4), (20, 6), (18, 6)], region_id=0
    )
    journey = jps.JourneyDescription([exit_id])

    journey_id = simulation.add_journey(journey)

    agent_id = simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(7, 7),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=0,
    )

    assert simulation.agent(agent_id).id == agent_id

    with pytest.raises(
        jps.SimulationError, match=".*Trying to access unknown Agent.*"
    ):
        simulation.agent(1000)


def test_agent_can_be_removed_from_simulation():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    # jps.set_debug_callback(log_msg_handler)
    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    p1 = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = shapely.Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=p1.union(p2)
    )
    exit_stage_id = simulation.add_exit_stage(
        [(18, 4), (20, 4), (20, 6), (18, 6)],
        region_id=0,
    )

    journey = jps.JourneyDescription([exit_stage_id])

    journey_id = simulation.add_journey(journey)

    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        expected_agent_ids.add(
            simulation.add_agent(
                journey_id=journey_id,
                stage_id=exit_stage_id,
                position=new_pos,
                state=jps.CollisionFreeSpeedModelState(),
                region_id=0,
            )
        )

    actual_agent_ids = {agent.id for agent in simulation.agents()}
    assert actual_agent_ids == expected_agent_ids

    # remove one agent form simulation
    agent_removed_id = actual_agent_ids.pop()
    expected_agent_ids.remove(agent_removed_id)

    simulation.mark_agent_for_removal(agent_removed_id)
    simulation.iterate()
    actual_agent_ids = {agent.id for agent in simulation.agents()}
    assert actual_agent_ids == expected_agent_ids

    # try removing the same agent will raise an error
    with pytest.raises(jps.SimulationError, match=r"Unknown agent id \d+"):
        simulation.mark_agent_for_removal(agent_removed_id)

    # remove second agent form simulation
    second_agent_removed_id = actual_agent_ids.pop()
    expected_agent_ids.remove(second_agent_removed_id)

    simulation.mark_agent_for_removal(second_agent_removed_id)
    simulation.iterate()
    actual_agent_ids = {agent.id for agent in simulation.agents()}
    assert actual_agent_ids == expected_agent_ids


def test_agent_can_not_be_added_outside_geometry():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )

    exit_id = simulation.add_exit_stage(
        [(99, 45), (99, 55), (100, 55), (100, 45)],
        region_id=0,
    )

    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)

    agent_position = (50, 50)

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=agent_position,
        state=jps.CollisionFreeSpeedModelState(),
        region_id=0,
    )

    with pytest.raises(
        jps.SimulationError,
        match=r"Point \(-50, -50\) is not in region 0",
    ):
        assert simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(-50, -50),
            state=jps.CollisionFreeSpeedModelState(),
            region_id=0,
        )


def test_direct_steering_target_must_be_inside_geometry():
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )
    stage_id = simulation.add_direct_steering_stage()
    journey_id = simulation.add_journey(jps.JourneyDescription([stage_id]))
    agent_id = simulation.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=(50, 50),
        state=jps.CollisionFreeSpeedModelState(),
        region_id=0,
    )
    agent = simulation.agent(agent_id)

    agent.final_target = (60, 60)
    assert agent.final_target == (60, 60)

    with pytest.raises(
        jps.SimulationError,
        match=r"Point \(-50, -50\) is outside of accessible area",
    ):
        agent.final_target = (-50, -50)
