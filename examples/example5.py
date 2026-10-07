#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import sys

import jupedsim as jps


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    surface = jps.WalkableSurface()
    region = surface.add_region(
        exterior=[
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
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example5_out.sqlite"),
        ),
    )

    exit_top = simulation.add_exit_stage(
        [(-2, 24), (2, 24), (2, 25), (-2, 25)], region_id=region
    )
    exit_right = simulation.add_exit_stage(
        [(34, -2), (34, 2), (35, 2), (35, -2)],
        region_id=region,
    )
    exit_bottom = simulation.add_exit_stage(
        [(-2, -24), (2, -24), (2, -25), (-2, -25)],
        region_id=region,
    )

    waypoint_middle = simulation.add_waypoint_stage((0, 0), 1, region_id=region)

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

    for x in range(-49, -9, 1):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=waypoint_middle,
            position=(x, 0),
            state=jps.CollisionFreeSpeedModelState(radius=0.3),
            region_id=region,
        )

    while simulation.agent_count() > 0:
        try:
            simulation.iterate()

        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            simulation._writer.close()
            sys.exit(1)

    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )
    simulation._writer.close()


if __name__ == "__main__":
    main()
