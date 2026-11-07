#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later

import pathlib
import sys

import jupedsim as jps
import shapely


def main():
    surface = jps.WalkableSurface()
    region = surface.add_region(polygon=shapely.box(0, -2.5, 50, 2.5))
    exit_polygon = shapely.box(48, -2.5, 50, 2.5)

    trajectory_file = "example_6.h5"
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=surface,
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path(trajectory_file)
        ),
    )

    exit_stage = simulation.add_exit(exit_polygon, region=region)
    route = jps.Route([exit_stage])

    motivated_start_positions = jps.distribute_by_number(
        polygon=shapely.box(0, -2.5, 5, 2.5),
        number_of_agents=30,
        distance_to_agents=0.3,
        distance_to_polygon=0.2,
    )

    for position in motivated_start_positions:
        simulation.add_agent(
            region.at(*position),
            route=route,
            state=jps.CollisionFreeSpeedModelV2State(
                radius=0.12,
                strength_neighbor_repulsion=8,
                range_neighbor_repulsion=0.1,
                strength_geometry_repulsion=5,
                range_geometry_repulsion=0.1,
            ),
        )

    slow_start_positions = jps.distribute_by_number(
        polygon=shapely.box(15, -2.5, 20, 2.5),
        number_of_agents=30,
        distance_to_agents=0.3,
        distance_to_polygon=0.2,
    )

    for position in slow_start_positions:
        simulation.add_agent(
            region.at(*position),
            route=route,
            state=jps.CollisionFreeSpeedModelV2State(
                radius=0.12,
                strength_neighbor_repulsion=30,
                range_neighbor_repulsion=0.1,
                strength_geometry_repulsion=15,
                range_geometry_repulsion=0.1,
                desired_speed=0.3,
            ),
        )

    while simulation.agent_count() > 0:
        try:
            simulation.iterate()
        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            simulation.close()
            sys.exit(1)

    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )
    simulation.close()


if __name__ == "__main__":
    main()
