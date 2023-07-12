import py_jupedsim as jps
import pytest


def test_can_query_agents_in_range():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([(0, 0), (100, 0), (100, 100), (0, 100)])
    geometry = geo_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 1
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.15
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    journey = jps.JourneyDescription()
    journey.add_exit([(99, 45), (99, 55), (100, 55), (100, 45)])

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    initial_agent_positions = [
        (10, 10),
        (20, 10),
        (30, 10),
        (40, 10),
        (50, 10),
    ]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        agent_parameters.position = new_pos
        expected_agent_ids.add(simulation.add_agent(agent_parameters))

    actual_ids_in_range = list(
        simulation.agents_in_range(initial_agent_positions[2], 10)
    )

    actual_ids_in_polygon = list(
        simulation.agents_in_polygon([(39, 11), (39, 9), (51, 9), (51, 11)])
    )

    assert actual_ids_in_range == [2, 3, 4]
    assert actual_ids_in_polygon == [4, 5]


def test_can_run_simulation():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    # jps.set_debug_callback(log_msg_handler)
    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([(0, 0), (10, 0), (10, 10), (0, 10)])
    geo_builder.add_accessible_area([(10, 4), (20, 4), (20, 6), (10, 6)])
    geometry = geo_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.15
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    journey = jps.JourneyDescription()
    journey.add_exit([(18, 4), (20, 4), (20, 6), (18, 6)])

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    expected_agent_ids = set()

    for new_pos in initial_agent_positions:
        agent_parameters.position = new_pos
        expected_agent_ids.add(simulation.add_agent(agent_parameters))

    actual_agent_ids = {agent.id for agent in simulation.agents()}

    assert actual_agent_ids == expected_agent_ids

    agent_parameters.position = (6, 6)
    agent_id = simulation.add_agent(agent_parameters)
    assert simulation.remove_agent(agent_id)
    with pytest.raises(RuntimeError, match=r"Unknown agent id \d+"):
        assert simulation.remove_agent(agent_id)

    for actual, expected in zip(simulation.agents(), initial_agent_positions):
        assert actual.position == jps.Point(expected)

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

    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([(0, 0), (100, 0), (100, 100), (0, 100)])
    geometry = geo_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.15
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    journey = jps.JourneyDescription()
    journey.add_waypoint((50, 50), 1)
    stage = journey.add_notifiable_waiting_set(
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
    journey.add_exit([(99, 40), (99, 60), (100, 60), (100, 40)])

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

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
        agent_parameters.position = new_pos
        expected_agent_ids.add(simulation.add_agent(agent_parameters))

    actual_agent_ids = {agent.id for agent in simulation.agents()}

    assert actual_agent_ids == expected_agent_ids

    agent_parameters.position = (30, 30)
    agent_id = simulation.add_agent(agent_parameters)
    assert simulation.remove_agent(agent_id)
    with pytest.raises(RuntimeError, match=r"Unknown agent id \d+"):
        assert simulation.remove_agent(agent_id)

    for actual, expected in zip(simulation.agents(), initial_agent_positions):
        assert actual.position == jps.Point(expected)

    while simulation.agent_count() > 0:
        simulation.iterate()
        if simulation.iteration_count() == 1000:
            simulation.notify_waiting_set(journey_id, stage, False)


def test_can_change_journey_while_waiting():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([(0, 0), (100, 0), (100, 100), (0, 100)])
    geometry = geo_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.15
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    journey1 = jps.JourneyDescription()
    journey1.add_waypoint((50, 50), 1)
    stage = journey1.add_notifiable_waiting_set(
        [
            (60, 50),
            (59, 50),
            (58, 50),
        ]
    )
    journey1.add_exit([(99, 40), (99, 60), (100, 60), (100, 40)])

    journey2 = jps.JourneyDescription()
    journey2.add_waypoint((60, 40), 1)
    journey2.add_waypoint((40, 40), 1)
    journey2.add_waypoint((40, 60), 1)
    journey2.add_waypoint((60, 60), 1)
    journey2.add_exit([(99, 50), (99, 70), (100, 70), (100, 50)])

    journeys = []
    journeys.append(simulation.add_journey(journey1))
    journeys.append(simulation.add_journey(journey2))

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journeys[0]
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    agent_parameters.position = (10, 50)
    simulation.add_agent(agent_parameters)

    agent_parameters.position = (8, 50)
    simulation.add_agent(agent_parameters)

    agent_parameters.position = (6, 50)
    simulation.add_agent(agent_parameters)

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
                stage_index=0,
            )
            redirect_once = False

        if signal_once and simulation.agents_in_range((60, 60), 1):
            simulation.notify_waiting_set(journeys[0], stage, False)
            signal_once = False
        simulation.iterate()
