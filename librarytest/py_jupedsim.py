import py_jupedsim as jps


def main():
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

    agent_parameters = jps.AgentParameters()
    agent_parameters.v0 = 1.0
    agent_parameters.a_min = 0.15
    agent_parameters.b_max = 0.15
    agent_parameters.b_min = 0.15
    agent_parameters.a_v = 0.53
    agent_parameters.t = 1
    agent_parameters.tau = 0.5
    agent_parameters.destination_area_id = destination
    agent_parameters.orientation_x = 1.0
    agent_parameters.orientation_y = 0.0
    agent_parameters.x = 0.0
    agent_parameters.y = 0.0

    for x, y in [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]:
        agent_parameters.x = x
        agent_parameters.y = y
        simulation.add_agent(agent_parameters)

    print("Running simulation")

    while simulation.agent_count() > 0:
        simulation.iterate()
        if simulation.iteration_count() % 100 == 0:
            print(f"Iteration: {simulation.iteration_count()}")
    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )


if __name__ == "__main__":
    main()
