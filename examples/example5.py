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

    geo_builder = jps.GeometryBuilder()
    region = geo_builder.add_region(
        boundary=[
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
        ],
        height=0.0,
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=geo_builder.build(),
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path("example5_out.h5"),
        ),
    )

    exit_top = simulation.add_target(
        jps.Exit([([(-2, 24), (2, 24), (2, 25), (-2, 25)], region)])
    )
    exit_right = simulation.add_target(
        jps.Exit([([(34, -2), (34, 2), (35, 2), (35, -2)], region)])
    )
    exit_bottom = simulation.add_target(
        jps.Exit([([(-2, -24), (2, -24), (2, -25), (-2, -25)], region)])
    )

    waypoint_middle = simulation.add_target(
        jps.Waypoint([(jps.circle_to_polygon(center=(0, 0), radius=1), region)])
    )

    route = simulation.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, waypoint_middle),
            jps.LeastTargetedTransition(
                waypoint_middle, [exit_top, exit_right, exit_bottom]
            ),
        ]
    )

    for x in range(-49, -9, 1):
        simulation.add_agent(
            simulation.get_location(x, 0, region_id=region),
            route=route,
            state=jps.CollisionFreeSpeedModelState(radius=0.3),
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
