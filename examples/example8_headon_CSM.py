#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later

import pathlib

import jupedsim as jps
from shapely import Polygon


def main():
    width = 2
    length = 10
    surface = jps.WalkableSurface()
    region = surface.add_region(
        polygon=Polygon([(0, 0), (length, 0), (length, width), (0, width)])
    )
    exit_polygon = Polygon(
        [(length - 0.5, 0), (length, 0), (length, width), (length - 0.5, width)]
    )
    trajectory_file = "headon_CSM.h5"
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path(trajectory_file)
        ),
    )

    exit_stage = simulation.add_exit(exit_polygon, region=region)
    route = jps.Route([exit_stage])

    start_positions = [(3, 0.5 * width), (0.5 * length, 0.5 * width)]
    states = [
        jps.CollisionFreeSpeedModelState(radius=0.2, desired_speed=1.2),
        jps.CollisionFreeSpeedModelState(radius=0.2, desired_speed=0.2),
    ]
    for position, state in zip(start_positions, states):
        simulation.add_agent(
            region.at(*position),
            route=route,
            state=state,
        )

    while simulation.agent_count() > 0 and simulation.iteration_count() < 1000:
        simulation.iterate()

    print(
        f"Simulation completed after {simulation.iteration_count()} iterations ({simulation.elapsed_time()} s)"
    )
    simulation._writer.close()


if __name__ == "__main__":
    main()
