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
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example4_out.sqlite"),
        ),
    )
    exit_left = simulation.add_exit_stage(
        [(-24, -8), (-24, 8), (-23, 8), (-23, -8)],
        region_id=region,
    )
    exit_top = simulation.add_exit_stage(
        [(-8, 24), (8, 24), (8, 23), (-8, 23)], region_id=region
    )
    exit_right = simulation.add_exit_stage(
        [(24, -8), (24, 8), (23, 8), (23, -8)],
        region_id=region,
    )

    waypoint_entrance = simulation.add_waypoint_stage(
        (0, -4), 1, region_id=region
    )
    waypoint_middle = simulation.add_waypoint_stage((0, 0), 1, region_id=region)

    journey = jps.JourneyDescription(
        [waypoint_entrance, waypoint_middle, exit_left, exit_top, exit_right]
    )

    journey.set_transition_for_stage(
        waypoint_entrance,
        jps.Transition.create_fixed_transition(waypoint_middle),
    )
    journey.set_transition_for_stage(
        waypoint_middle,
        jps.Transition.create_round_robin_transition(
            [(exit_left, 5), (exit_top, 1), (exit_right, 3)]
        ),
    )
    journey_id = simulation.add_journey(journey)

    for y in range(-23, -12, 2):
        for x in range(-7, 8, 2):
            simulation.add_agent(
                journey_id=journey_id,
                stage_id=waypoint_entrance,
                position=(x, y),
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
