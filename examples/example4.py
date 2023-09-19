#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
import sys

from shapely import GeometryCollection, Polygon

import jupedsim as jps


def main():
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_warning_callback(lambda x: logging.debug(x))
    jps.set_error_callback(lambda x: logging.debug(x))

    area = GeometryCollection(
        Polygon(
            shell=[
                (-8, -8),
                (-24, -8),
                (-24, 8),
                (-8, 8),
                (-8, 24),
                (8, 24),
                (8, 8),
                (24, 8),
                (24, -8),
                (8, -8),
                (8, -24),
                (-8, -24),
            ]
        )
    )
    geometry = jps.geometry_from_shapely(area)

    simulation = jps.Simulation(
        model=jps.VelocityModelParameters(),
        geometry=geometry,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example4_out.sqlite"),
        ),
    )
    exit_left = simulation.add_exit_stage(
        [(-24, -8), (-24, 8), (-23, 8), (-23, -8)]
    )
    exit_top = simulation.add_exit_stage(
        [(-8, 24), (8, 24), (8, 23), (-8, 23)]
    )
    exit_right = simulation.add_exit_stage(
        [(24, -8), (24, 8), (23, 8), (23, -8)]
    )

    queue_id = simulation.add_queue_stage(
        [
            (0, -4),
            (0, -8),
        ]
    )
    queue = simulation.get_stage_proxy(queue_id)

    waypoint_middle = simulation.add_waypoint_stage((0, 0), 1)

    journey = jps.JourneyDescription(
        [queue_id, waypoint_middle, exit_left, exit_top, exit_right]
    )

    journey.set_transition_for_stage(
        queue_id,
        jps.Transition.create_fixed_transition(waypoint_middle),
    )
    journey.set_transition_for_stage(
        waypoint_middle,
        jps.Transition.create_round_robin_transition(
            [(exit_left, 5), (exit_top, 1), (exit_right, 3)]
        ),
    )
    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.stage_id = queue_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.time_gap = 1
    agent_parameters.tau = 0.5
    agent_parameters.v0 = 1.2
    agent_parameters.radius = 0.3

    for y in range(-23, -12, 2):
        for x in range(-7, 8, 2):
            agent_parameters.position = (x, y)
            simulation.add_agent(agent_parameters)

    while simulation.agent_count() > 0:
        try:
            if (
                simulation.iteration_count() > 100 * 5
                and simulation.iteration_count() % 100 == 0
            ):
                queue.pop(1)
                print("Next!")

            simulation.iterate()
        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)


if __name__ == "__main__":
    main()
