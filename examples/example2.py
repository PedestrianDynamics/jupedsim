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
        polygon=shapely.Polygon([(0, 0), (100, 0), (100, 100), (0, 100)])
    )

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path("example2_out.h5"),
        ),
    )
    stage = simulation.add_waypoint(
        jps.circle_to_polygon(center=(60, 50), radius=1), region=region
    )
    exits = [
        simulation.add_exit(
            [(99, 40), (99, 60), (100, 60), (100, 40)], region=region
        ),
        simulation.add_exit(
            [(99, 50), (99, 70), (100, 70), (100, 50)], region=region
        ),
    ]
    waypoints = [
        simulation.add_waypoint(
            jps.circle_to_polygon(center=center, radius=1), region=region
        )
        for center in [(50, 50), (60, 40), (40, 40), (40, 60), (60, 60)]
    ]

    short_route = jps.Route([waypoints[0], stage, exits[0]])
    long_route = jps.Route([*waypoints[1:], exits[1]])

    for position in [(10, 50), (8, 50), (6, 50)]:
        simulation.add_agent(
            region.at(*position),
            route=short_route,
            state=jps.CollisionFreeSpeedModelState(radius=0.3),
        )

    redirect_once = True
    while simulation.agent_count() > 0:
        try:
            agents_at_waypoint = simulation.agents_in_range((60, 50), 1)
            if redirect_once and len(agents_at_waypoint) == 1:
                agents_at_waypoint[0].follow(long_route, start=waypoints[1])
                redirect_once = False
                print(
                    f"Switched route for agent {agents_at_waypoint[0].id} @{simulation.iteration_count()}"
                )
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
