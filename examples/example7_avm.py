#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later

import pathlib
import sys

import jupedsim as jps
from shapely import Polygon


def main():
    room1 = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])  # Room 1 (10m x 10m)
    room2 = Polygon([(15, 0), (25, 0), (25, 10), (15, 10)])
    width = 0.6
    num_agents = 10
    corridor = Polygon(
        [
            (10, 5 - width / 2),
            (15, 5 - width / 2),
            (15, 5 + width / 2),
            (10, 5 + width / 2),
        ]
    )
    geometry = room1.union(room2).union(corridor)
    exit_polygon = Polygon([(24, 4.5), (25, 4.5), (25, 5.5), (24, 5.5)])
    trajectory_file = "example_7.sqlite"
    simulation = jps.Simulation(
        model=jps.AnticipationVelocityModel(),
        geometry=geometry,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path(trajectory_file)
        ),
    )

    exit_id = simulation.add_exit_stage(exit_polygon)
    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)

    start_positions = jps.distribute_by_number(
        polygon=Polygon([(0, 0), (5, 0), (5, 10), (0, 10)]),
        number_of_agents=num_agents,
        distance_to_agents=0.5,
        distance_to_polygon=0.2,
    )

    for position in start_positions:
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                journey_id=journey_id,
                stage_id=exit_id,
                position=position,
                anticipation_time=0.5,
                reaction_time=0.3,
                wall_buffer_distance=0.05,
            )
        )

    while simulation.agent_count() > 0 and simulation.iteration_count() < 3000:
        try:
            simulation.iterate()
        except KeyboardInterrupt:
            simulation._writer.close()
            print("CTRL-C Received! Shutting down")
            sys.exit(1)

    simulation._writer.close()
    print(
        f"Simulation completed after {simulation.iteration_count()} iterations ({simulation.elapsed_time()} s)"
    )
    print(f"{trajectory_file = }")


if __name__ == "__main__":
    main()
