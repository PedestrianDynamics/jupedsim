#! /usr/bin/env python3

# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import sys

import jupedsim as jps


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    area = [
        (-2, -2),
        (-50, -2),
        (-50, 2),
        (-2, 2),
        (-2, 25),
        (2, 25),
        (2, 2),
        (35, 2),
        (35, -2),
        (2, -2),
        (2, -25),
        (-2, -25),
    ]
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=area,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example5_out.sqlite"),
        ),
    )

    exit_top = simulation.add_exit_stage(
        [(-2, 24), (2, 24), (2, 25), (-2, 25)]
    )
    exit_right = simulation.add_exit_stage(
        [(34, -2), (34, 2), (35, 2), (35, -2)]
    )
    exit_bottom = simulation.add_exit_stage(
        [(-2, -24), (2, -24), (2, -25), (-2, -25)]
    )

    waypoint_middle = simulation.add_queue_stage(
        [
            (0, 0),
            (0, -2),
            (0, -8),
        ]
    )
    queue = simulation.get_stage(waypoint_middle)

    journey = jps.JourneyDescription(
        [waypoint_middle, exit_top, exit_right, exit_bottom]
    )

    journey.set_transition_for_stage(
        waypoint_middle,
        jps.Transition.create_least_targeted_transition(
            [exit_top, exit_right, exit_bottom]
        ),
    )

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.CollisionFreeSpeedModelAgentParameters(
        journey_id=journey_id, stage_id=waypoint_middle, radius=0.3
    )

    for x in range(-49, -9, 1):
        agent_parameters.position = (x, 0)
        simulation.add_agent(agent_parameters)

    while simulation.agent_count() > 0:
        try:
            if simulation.iteration_count() % 200 == 0:
                queue.pop(1)
                print("Next!")
            simulation.iterate()

        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)


if __name__ == "__main__":
    main()
