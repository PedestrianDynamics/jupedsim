# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pytest
import shapely

import jupedsim as jps


def test_can_query_agents_in_range():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.VelocityModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )
    exit = simulation.add_exit_stage(
        [(99, 45), (99, 55), (100, 55), (100, 45)]
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

    expected_agent_ids = set()

    for new_pos, id in zip(
        initial_agent_positions,
        range(10, 10 + len(initial_agent_positions)),
    ):
        expected_agent_ids.add(
            simulation.add_agent(
                jps.VelocityModelAgentParameters(
                    position=new_pos,
                    journey_id=journey_id,
                    stage_id=exit,
                )
            )
        )

    actual_ids_in_range = list(
        simulation.agents_in_range(initial_agent_positions[2], 10)
    )

    actual_ids_in_polygon = list(
        simulation.agents_in_polygon([(39, 11), (39, 9), (51, 9), (51, 11)])
    )

    assert actual_ids_in_range == [3, 4, 5]
    assert actual_ids_in_polygon == [5, 6]


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
        model=jps.VelocityModel(), geometry=p1.union(p2)
    )
    exit_stage_id = simulation.add_exit_stage(
        [(18, 4), (20, 4), (20, 6), (18, 6)]
    )

    journey = jps.JourneyDescription([exit_stage_id])

    journey_id = simulation.add_journey(journey)

    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        expected_agent_ids.add(
            simulation.add_agent(
                jps.VelocityModelAgentParameters(
                    position=new_pos,
                    journey_id=journey_id,
                    stage_id=exit_stage_id,
                )
            )
        )

    actual_agent_ids = {agent.id for agent in simulation.agents()}

    assert actual_agent_ids == expected_agent_ids

    agent_id = simulation.add_agent(
        jps.VelocityModelAgentParameters(
            position=(6, 6),
            journey_id=journey_id,
            stage_id=exit_stage_id,
        )
    )

    for actual, expected in zip(simulation.agents(), initial_agent_positions):
        assert actual.position == expected

    assert simulation.mark_agent_for_removal(agent_id)
    simulation.iterate()

    with pytest.raises(RuntimeError, match=r"Unknown agent id \d+"):
        assert simulation.mark_agent_for_removal(agent_id)

    while simulation.agent_count() > 0:
        simulation.iterate()
        assert simulation.iteration_count() < 2000


def test_can_wait():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.VelocityModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )
    wp = simulation.add_waypoint_stage((50, 50), 1)
    waiting_set_id = simulation.add_waiting_set_stage(
        [
            (70, 50),
            (69, 50),
            (68, 50),
            (67, 50),
            (66, 50),
            (65, 50),
            (64, 50),
        ]
    )
    waiting_set = simulation.get_stage_proxy(waiting_set_id)
    exit = simulation.add_exit_stage(
        [(99, 40), (99, 60), (100, 60), (100, 40)]
    )
    journey = jps.JourneyDescription([wp, waiting_set_id, exit])
    journey.set_transition_for_stage(
        wp, jps.Transition.create_fixed_transition(waiting_set_id)
    )
    journey.set_transition_for_stage(
        waiting_set_id, jps.Transition.create_fixed_transition(exit)
    )

    journey_id = simulation.add_journey(journey)
    initial_agent_positions = [
        (1, 1),
        (1, 2),
        (2, 1),
        (2, 2),
        (3, 1),
        (3, 2),
        (3, 3),
        (2, 3),
        (1, 3),
    ]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        expected_agent_ids.add(
            simulation.add_agent(
                jps.VelocityModelAgentParameters(
                    position=new_pos,
                    journey_id=journey_id,
                    stage_id=wp,
                )
            )
        )

    actual_agent_ids = {agent.id for agent in simulation.agents()}

    assert actual_agent_ids == expected_agent_ids

    agent_id = simulation.add_agent(
        jps.VelocityModelAgentParameters(
            position=(30, 30),
            journey_id=journey_id,
            stage_id=wp,
        )
    )

    for actual, expected in zip(simulation.agents(), initial_agent_positions):
        assert actual.position == expected

    assert simulation.mark_agent_for_removal(agent_id)
    simulation.iterate()

    with pytest.raises(RuntimeError, match=r"Unknown agent id \d+"):
        assert simulation.mark_agent_for_removal(agent_id)

    while simulation.agent_count() > 0:
        simulation.iterate()
        if simulation.iteration_count() == 1000:
            waiting_set.state = jps.WaitingSetState.INACTIVE


def test_can_change_journey_while_waiting():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.VelocityModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )
    wp = simulation.add_waypoint_stage((50, 50), 1)
    stage_id = simulation.add_waiting_set_stage(
        [
            (60, 50),
            (59, 50),
            (58, 50),
        ]
    )
    stage = simulation.get_stage_proxy(stage_id)
    exit1 = simulation.add_exit_stage(
        [(99, 40), (99, 60), (100, 60), (100, 40)]
    )

    journey1 = jps.JourneyDescription([wp, stage_id, exit1])
    journey1.set_transition_for_stage(
        wp, jps.Transition.create_fixed_transition(stage_id)
    )
    journey1.set_transition_for_stage(
        stage_id, jps.Transition.create_fixed_transition(exit1)
    )
    journey2_stages = [
        simulation.add_waypoint_stage((60, 40), 1),
        simulation.add_waypoint_stage((40, 40), 1),
        simulation.add_waypoint_stage((40, 60), 1),
        simulation.add_waypoint_stage((60, 60), 1),
        simulation.add_exit_stage([(99, 50), (99, 70), (100, 70), (100, 50)]),
    ]
    journey2 = jps.JourneyDescription(journey2_stages)
    for src, dst in zip(journey2_stages[:-1], journey2_stages[1:]):
        journey2.set_transition_for_stage(
            src, jps.Transition.create_fixed_transition(dst)
        )

    journeys = []
    journeys.append(simulation.add_journey(journey1))
    journeys.append(simulation.add_journey(journey2))

    simulation.add_agent(
        jps.VelocityModelAgentParameters(
            position=(10, 50),
            journey_id=journeys[0],
            stage_id=wp,
        )
    )

    simulation.add_agent(
        jps.VelocityModelAgentParameters(
            position=(8, 50),
            journey_id=journeys[0],
            stage_id=wp,
        )
    )

    simulation.add_agent(
        jps.VelocityModelAgentParameters(
            position=(6, 50),
            journey_id=journeys[0],
            stage_id=wp,
        )
    )

    redirect_once = True
    signal_once = True
    while simulation.agent_count() > 0:
        agents_at_head_of_waiting = list(
            simulation.agents_in_range((60, 50), 1)
        )
        if redirect_once and agents_at_head_of_waiting:
            simulation.switch_agent_journey(
                agent_id=agents_at_head_of_waiting[0],
                journey_id=journeys[1],
                stage_id=journey2_stages[0],
            )
            redirect_once = False

        if signal_once and simulation.agents_in_range((60, 60), 1):
            stage.state = jps.WaitingSetState.INACTIVE
            signal_once = False
        simulation.iterate()


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
        model=jps.VelocityModel(), geometry=p1.union(p2)
    )
    exit_id = simulation.add_exit_stage([(18, 4), (20, 4), (20, 6), (18, 6)])

    journey = jps.JourneyDescription([exit_id])

    journey_id = simulation.add_journey(journey)
    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    agent_ids = set()

    for new_pos in initial_agent_positions:
        agent_ids.add(
            simulation.add_agent(
                jps.VelocityModelAgentParameters(
                    position=new_pos,
                    journey_id=journey_id,
                    stage_id=exit_id,
                )
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
        model=jps.VelocityModel(), geometry=p1.union(p2)
    )

    exit_id = simulation.add_exit_stage([(18, 4), (20, 4), (20, 6), (18, 6)])
    journey = jps.JourneyDescription([exit_id])

    journey_id = simulation.add_journey(journey)

    agent_id = simulation.add_agent(
        jps.VelocityModelAgentParameters(
            position=(7, 7),
            journey_id=journey_id,
            stage_id=exit_id,
        )
    )

    assert simulation.agent(agent_id).id == agent_id

    with pytest.raises(
        RuntimeError, match=".*Trying to access unknown Agent.*"
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
        model=jps.VelocityModel(), geometry=p1.union(p2)
    )
    exit_stage_id = simulation.add_exit_stage(
        [(18, 4), (20, 4), (20, 6), (18, 6)]
    )

    journey = jps.JourneyDescription([exit_stage_id])

    journey_id = simulation.add_journey(journey)

    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        expected_agent_ids.add(
            simulation.add_agent(
                jps.VelocityModelAgentParameters(
                    position=new_pos,
                    journey_id=journey_id,
                    stage_id=exit_stage_id,
                )
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
    with pytest.raises(RuntimeError, match=r"Unknown agent id \d+"):
        assert simulation.mark_agent_for_removal(agent_removed_id)

    # remove second agent form simulation
    second_agent_removed_id = actual_agent_ids.pop()
    expected_agent_ids.remove(second_agent_removed_id)

    simulation.mark_agent_for_removal(second_agent_removed_id)
    simulation.iterate()
    actual_agent_ids = {agent.id for agent in simulation.agents()}
    assert actual_agent_ids == expected_agent_ids
