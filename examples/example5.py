#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import sys

import jupedsim as jps
import shapely


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    surface = jps.WalkableSurface()
    region = surface.add_region(
        polygon=shapely.Polygon(
            [
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
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path("example5_out.h5"),
        ),
    )

    exit_top = simulation.add_exit(
        [(-2, 24), (2, 24), (2, 25), (-2, 25)], region=region
    )
    exit_right = simulation.add_exit(
        [(34, -2), (34, 2), (35, 2), (35, -2)], region=region
    )
    exit_bottom = simulation.add_exit(
        [(-2, -24), (2, -24), (2, -25), (-2, -25)], region=region
    )

    waypoint_middle = simulation.add_waypoint(
        jps.circle_to_polygon(center=(0, 0), radius=1), region=region
    )

    route = jps.Route(
        [
            (
                waypoint_middle,
                jps.LeastTargeted([exit_top, exit_right, exit_bottom]),
            )
        ],
        start=waypoint_middle,
    )

    for x in range(-49, -9, 1):
        simulation.add_agent(
            region.at(x, 0),
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
