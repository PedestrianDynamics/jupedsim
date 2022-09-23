# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib

import py_jupedsim as jps
from configs import init_logger, log_error, log_info
from jupedsim.serialization import JpsCoreStyleTrajectoryWriter
from utilities import (
    build_areas,
    build_geometry,
    build_gcfm_model,
    distribute_and_add_agents,
    init_journey,
    init_gcfm_agent_parameters,
)


def main(fps: int, dt: float, trajectory_path: pathlib.Path):
    """Main simulation loop

    :param fps:
    :param dt:
    :param trajectory_file:
    :returns:

    """
    accessible_areas = [
        # x, y list in CCW
        [0, 0, 10, 0, 10, 10, 0, 10],
        [10, 4, 20, 4, 20, 6, 10, 6],
    ]
    geometry = build_geometry(accessible_areas)
    destinations = {
        # id, list(x, y)
        1: [18, 4, 20, 4, 20, 6, 18, 6]
    }
    labels = ["exit", "other-label"]
    areas = build_areas(destinations, labels)
    parameter_profiles = {
        # id:  (mass, tau, v0)
        1: (1, 0.5, 1.2),
        2: (1, 0.5, 0.1),
    }
    model = build_gcfm_model(
        0.3, 0.21, 2, 2.1, 0.1, 0.11, 3, 3.1, parameter_profiles
    )

    log_info(f"Init simulation with dt={dt} [s] and fps={fps}")
    simulation = jps.Simulation(model, geometry, areas, dt)
    log_info("Init simulation done")
    way_points = [((19, 5), 0.5)]
    journey_id = init_journey(simulation, way_points)
    agent_parameters = init_gcfm_agent_parameters(
        a_min=0.15,
        b_min=0.15,
        b_max=0.30,
        a_v=0.2,
        phi_x=1,
        phi_y=0,
        journey=journey_id,
        profile=1,
    )
    positions = [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]
    ped_ids = distribute_and_add_agents(
        simulation, agent_parameters, positions
    )
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
    init_logger()
    main(fps=10, dt=0.01, trajectory_path=pathlib.Path("out.txt"))
