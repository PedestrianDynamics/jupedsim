#! /usr/bin/env python3

# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

import pathlib

import jupedsim as jps
from shapely import Polygon


def main():
    width = 2
    length = 10
    geometry = Polygon([(0, 0), (length, 0), (length, width), (0, width)])
    exit_polygon = Polygon(
        [(length - 0.5, 0), (length, 0), (length, width), (length - 0.5, width)]
    )
    trajectory_file = "headon_CSM.sqlite"
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=geometry,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path(trajectory_file)
        ),
    )

    exit_id = simulation.add_exit_stage(exit_polygon)
    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)

    start_positions = [(3, 0.5 * width), (0.5 * length, 0.5 * width)]
    parameters = [
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_id,
            stage_id=exit_id,
            position=start_positions[0],
            radius=0.2,
            v0=1.2,
        ),
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_id,
            stage_id=exit_id,
            position=start_positions[1],
            radius=0.2,
            v0=0.2,
        ),
    ]
    for position, param in zip(start_positions, parameters):
        simulation.add_agent(parameters=param)

    while simulation.agent_count() > 0 and simulation.iteration_count() < 1000:
        simulation.iterate()

    print(
        f"Simulation completed after {simulation.iteration_count()} iterations ({simulation.elapsed_time()} s)"
    )
    print(f">> {trajectory_file = }")


if __name__ == "__main__":
    main()
