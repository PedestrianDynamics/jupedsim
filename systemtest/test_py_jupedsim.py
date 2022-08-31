import pytest
import py_jupedsim as jps


def test_can_run_simulation():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    # jps.set_debug_callback(log_msg_handler)
    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([0, 0, 10, 0, 10, 10, 0, 10])
    geo_builder.add_accessible_area([10, 4, 20, 4, 20, 6, 10, 6])
    geometry = geo_builder.build()

    destination = 1
    areas_builder = jps.AreasBuilder()
    areas_builder.add_area(
        destination, [18, 4, 20, 4, 20, 6, 18, 6], ["exit", "other-label"]
    )
    areas = areas_builder.build()

    model = jps.OperationalModel.make_velocity_model(8, 0.1, 5, 0.02)

    simulation = jps.Simulation(model, geometry, areas, 0.01)

    journey = jps.Journey.make_waypoint_journey([((19, 5), 0.5)])

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.AgentParameters()
    agent_parameters.v0 = 1.0
    agent_parameters.a_min = 0.15
    agent_parameters.b_max = 0.15
    agent_parameters.b_min = 0.15
    agent_parameters.a_v = 0.53
    agent_parameters.t = 1
    agent_parameters.tau = 0.5
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation_x = 1.0
    agent_parameters.orientation_y = 0.0
    agent_parameters.x = 0.0
    agent_parameters.y = 0.0

    initial_agent_positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]

    for x, y in initial_agent_positions:
        agent_parameters.x = x
        agent_parameters.y = y
        simulation.add_agent(agent_parameters)

    agent_id = simulation.add_agent(agent_parameters)
    assert simulation.remove_agent(agent_id)
    with pytest.raises(RuntimeError, match=r"Unknown agent id \d+"):
        assert simulation.remove_agent(agent_id)

    for actual, expected in zip(simulation.agents(), initial_agent_positions):
        assert actual.x == expected[0]
        assert actual.y == expected[1]

    while simulation.agent_count() > 0:
        simulation.iterate()
        assert simulation.iteration_count() < 2000
