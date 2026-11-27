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
        boundary=[(0, 0), (100, 0), (100, 100), (0, 100)], height=0.0
    )

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=geo_builder.build(),
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path("example2_out.h5"),
        ),
    )
    waypoint = simulation.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(60, 50), radius=1), region)]
        )
    )
    exits = [
        simulation.add_target(
            jps.Exit([([(99, 40), (99, 60), (100, 60), (100, 40)], region)])
        ),
        simulation.add_target(
            jps.Exit([([(99, 50), (99, 70), (100, 70), (100, 50)], region)])
        ),
    ]
    waypoints = [
        simulation.add_target(
            jps.Waypoint(
                [(jps.circle_to_polygon(center=(50, 50), radius=1), region)]
            )
        ),
        simulation.add_target(
            jps.Waypoint(
                [(jps.circle_to_polygon(center=(60, 40), radius=1), region)]
            )
        ),
        simulation.add_target(
            jps.Waypoint(
                [(jps.circle_to_polygon(center=(40, 40), radius=1), region)]
            )
        ),
        simulation.add_target(
            jps.Waypoint(
                [(jps.circle_to_polygon(center=(40, 60), radius=1), region)]
            )
        ),
        simulation.add_target(
            jps.Waypoint(
                [(jps.circle_to_polygon(center=(60, 60), radius=1), region)]
            )
        ),
    ]

    short_route = simulation.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, waypoints[0]),
            jps.FixedTransition(waypoints[0], waypoint),
            jps.FixedTransition(waypoint, exits[0]),
        ]
    )
    long_targets = [*waypoints[1:], exits[1]]
    long_route = simulation.add_route(
        [jps.FixedTransition(jps.AGENT_LOCATION, long_targets[0])]
        + [
            jps.FixedTransition(source, target)
            for source, target in zip(long_targets, long_targets[1:])
        ]
    )

    for position in [(10, 50), (8, 50), (6, 50)]:
        simulation.add_agent(
            region.at(*position),
            route=short_route,
            state=jps.CollisionFreeSpeedModelState(radius=0.3),
        )

    redirect_once = True
    while simulation.agent_count() > 0:
        try:
            agents_at_waypoint = simulation.agents_in_range(
                region.at(60, 50), 1
            )
            if redirect_once and len(agents_at_waypoint) == 1:
                agents_at_waypoint[0].reroute(route=long_route)
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
