# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib

import py_jupedsim as jps
from configs import init_logger, log_error, log_info
from jupedsim.serialization import JpsCoreStyleTrajectoryWriter


def build_geometry() -> jps.GeometryBuilder:
    """build geometry object

    All points should be defined CCW
    :returns: a geometry builder

    """
    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([0, 0, 10, 0, 10, 10, 0, 10])
    geo_builder.add_accessible_area([10, 4, 20, 4, 20, 6, 10, 6])
    geometry = geo_builder.build()
    return geometry


def build_areas() -> jps.AreasBuilder:
    """Build destination areas

    all Points should be CCW
    :returns: Area builder

    """
    destination_id = 1
    areas_builder = jps.AreasBuilder()
    areas_builder.add_area(
        destination_id,
        # x  y  CCW polygon
        [18, 4, 20, 4, 20, 6, 18, 6],
        ["exit", "other-label"],
    )
    areas = areas_builder.build()
    return areas


def build_model(
    a_ped: float,
    D_ped: float,
    a_wall: float,
    D_wall: float,
    parameter_profiles: dict,
) -> jps.OperationalModel:
    """Initialize velocity model with parameter values

    :param a_ped:
    :param D_ped:
    :param a_wall:
    :param D_wall:
    :returns: velocity model

    """
    model_builder = jps.VelocityModelBuilder(a_ped, D_ped, a_wall, D_wall)
    # define two different profiles
    for key, params in parameter_profiles.items():
        model_builder.add_parameter_profile(
            key, params[0], params[1], params[2]
        )
    model = model_builder.build()
    return model


def init_journey(simulation: jps.Simulation) -> int:
    """Init goals of agents to follow

    :param simulation:
    :returns:

    """
    points = [((19, 5), 0.5)]  # defined as a list of (point, distance)
    journey = jps.Journey.make_waypoint_journey(points)
    journey_id = simulation.add_journey(journey)
    return journey_id


def init_agent_parameters(
    radius: float,
    phi_x: float,
    phi_y: float,
    journey: jps.Journey,
    profile: int,
) -> jps.AgentParameters:
    """Init agent shape and parameters

    :param radius: radius of the circle
    :param phi_x: direcion in x-axis
    :param phi_y: direction in y-axis
    :param journey: waypoints for agents to pass through
    :param profile: profile id
    :returns:

    """
    agent_parameters = jps.AgentParameters()
    # Shape is a circle
    agent_parameters.a_min = radius
    agent_parameters.b_max = radius
    agent_parameters.b_min = radius
    agent_parameters.a_v = 0.0
    # ----- Profile
    agent_parameters.journey_id = journey
    agent_parameters.orientation_x = phi_x
    agent_parameters.orientation_y = phi_y
    agent_parameters.profile_id = profile
    return agent_parameters


def distribute_and_add_agents(
    simulation: jps.Simulation, agent_parameters: jps.AgentParameters
) -> list:
    """Initialize positions of agents and insert them into the simulation

    :param simulation:
    :param agent_parameters:
    :returns:

    """

    ped_ids = []
    positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]
    for x, y in positions:
        agent_parameters.x = x
        agent_parameters.y = y
        ped_id = simulation.add_agent(agent_parameters)
        ped_ids.append(ped_id)

    return ped_ids


def main(fps: int, dt: float, trajectory_path: pathlib.Path):
    """Main simulation loop

    :param fps:
    :param dt:
    :param trajectory_file:
    :returns:

    """
    init_logger(jps)
    log_info("Init geometry")
    geometry = build_geometry()
    log_info("Init areas")
    areas = build_areas()
    log_info("Init velocity model")
    parameter_profiles = {
        # id:  (timeGap, tau, v0)
        1: (1, 0.5, 1.0),
        2: (1, 0.5, 0.1),
    }
    model = build_model(8, 0.1, 5, 0.02, parameter_profiles)
    log_info(f"Init simulation with dt={dt} [s] and fps={fps}")
    simulation = jps.Simulation(model, geometry, areas, dt)
    log_info("Init simulation done")
    log_info("Init journey")
    journey_id = init_journey(simulation)
    log_info("Create agents")
    agent_parameters = init_agent_parameters(
        radius=0.15, phi_x=1, phi_y=0, journey=journey_id, profile=1
    )
    ped_ids = distribute_and_add_agents(simulation, agent_parameters)
    log_info(f"Running simulation for {len(ped_ids)} agents:")
    writer = JpsCoreStyleTrajectoryWriter(trajectory_path)
    writer.begin_writing(fps)
    test_id = ped_ids[0]
    actual_profile = 1
    while simulation.agent_count() > 0:
        simulation.iterate()
        if simulation.iteration_count() % fps == 0:
            writer.write_iteration_state(simulation)

        if (
            simulation.iteration_count() > 0
            and simulation.iteration_count() < 500
        ):
            actual_profile = 2

        if (
            simulation.iteration_count() > 500
            and simulation.iteration_count() < 700
        ):
            actual_profile = 1

        try:
            simulation.switch_agent_profile(test_id, actual_profile)
        except RuntimeError:
            log_error(
                f"Can not change Profile of Agent {test_id} to Profile={actual_profile} at Iteration={simulation.iteration_count()}"
            )
            # end the simulation
            break

    writer.end_writing()
    log_info(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )
    log_info(f"Trajectory: {trajectory_path}")


if __name__ == "__main__":
    main(fps=10, dt=0.01, trajectory_path=pathlib.Path("out.txt"))
