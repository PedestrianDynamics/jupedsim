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
        ],
        height=0.0,
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=geo_builder.build(),
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path("example4_out.h5"),
        ),
    )
    exit_left = simulation.add_target(
        jps.Exit([([(-24, -8), (-24, 8), (-23, 8), (-23, -8)], region)])
    )
    exit_top = simulation.add_target(
        jps.Exit([([(-8, 24), (8, 24), (8, 23), (-8, 23)], region)])
    )
    exit_right = simulation.add_target(
        jps.Exit([([(24, -8), (24, 8), (23, 8), (23, -8)], region)])
    )

    waypoint_entrance = simulation.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(0, -4), radius=1), region)]
        )
    )
    waypoint_middle = simulation.add_target(
        jps.Waypoint([(jps.circle_to_polygon(center=(0, 0), radius=1), region)])
    )

    route = simulation.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, waypoint_entrance),
            jps.FixedTransition(waypoint_entrance, waypoint_middle),
            jps.RoundRobinTransition(
                waypoint_middle, {exit_left: 5, exit_top: 1, exit_right: 3}
            ),
        ]
    )

    for y in range(-23, -12, 2):
        for x in range(-7, 8, 2):
            simulation.add_agent(
                region.at(x, y),
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
