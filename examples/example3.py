#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import sys

import jupedsim as jps


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    simulation = jps.Simulation(
        model=jps.VelocityModelParameters(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100), (0, 0)],
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example3_out.sqlite"),
        ),
    )
    stage_id = simulation.add_queue_stage(
        [
            (60, 50),
            (59, 50),
            (58, 50),
            (57, 50),
            (56, 50),
            (55, 50),
            (54, 50),
        ]
    )
    queue = simulation.get_stage_proxy(stage_id)
    exit = simulation.add_exit_stage(
        [(99, 40), (99, 60), (100, 60), (100, 40)]
    )

    journey = jps.JourneyDescription([stage_id, exit])
    journey.set_transition_for_stage(
        stage_id, jps.Transition.create_fixed_transition(exit)
    )

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.stage_id = stage_id

    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.time_gap = 1
    agent_parameters.v0 = 1.2
    agent_parameters.radius = 0.3

    for y in range(1, 16):
        agent_parameters.position = (0.5, y)
        simulation.add_agent(agent_parameters)

    while (
        simulation.agent_count() > 0 and simulation.iteration_count() < 20_000
    ):
        try:
            if (
                simulation.iteration_count() > 100 * 52
                and simulation.iteration_count() % 400 == 0
            ):
                queue.pop(1)
                print("Next!")

            simulation.iterate()
        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)


if __name__ == "__main__":
    main()
