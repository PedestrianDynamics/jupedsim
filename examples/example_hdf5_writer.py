# SPDX-License-Identifier: LGPL-3.0-or-later

"""Example: HDF5 trajectory writer.

Runs a short simulation, writes the trajectory to an HDF5 file in a
layout compatible with the loaders provided by `PedPy
<https://github.com/PedestrianDynamics/PedPy>`_ for the Pedestrian
Dynamics Data Archive format, then reads the file back with
PedPy and plots the trajectories on top of the walkable area.

Extra dependencies for this example (beyond JuPedSim itself):

    pip install h5py pedpy matplotlib

The script writes ``example_traj.h5`` and ``example_traj.png`` to the
current working directory.
"""

import pathlib
import sys

import jupedsim as jps
import matplotlib.pyplot as plt
import pedpy
from shapely import GeometryCollection, Polygon


def main() -> None:
    if jps.Hdf5TrajectoryWriter is None:
        sys.exit("h5py is not installed. Install with: pip install h5py")

    room1 = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    room2 = Polygon([(15, 0), (25, 0), (25, 10), (15, 10)])
    corridor = Polygon([(10, 4.5), (28, 4.5), (28, 5.5), (10, 5.5)])

    area = GeometryCollection(corridor.union(room1.union(room2)))
    walkable_area = pedpy.WalkableArea(area.geoms[0])
    out = pathlib.Path("example_traj.h5")
    writer = jps.Hdf5TrajectoryWriter(
        output_file=out,
        every_nth_frame=4,
        compression_level=1,
    )

    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=area,
        trajectory_writer=writer,
        dt=0.01,
    )
    exit_area = Polygon([(27, 4.5), (28, 4.5), (28, 5.5), (27, 5.5)])
    exit_id = sim.add_exit_stage(exit_area)
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    spawning_area = Polygon([(0, 0), (5, 0), (5, 10), (0, 10)])
    num_agents = 150
    pos_in_spawning_area = jps.distributions.distribute_by_number(
        polygon=spawning_area,
        number_of_agents=num_agents,
        distance_to_agents=0.4,
        distance_to_polygon=0.2,
        seed=1,
    )
    for position in pos_in_spawning_area:
        sim.add_agent(
            jps.CollisionFreeSpeedModelV2AgentParameters(
                position=position, journey_id=journey_id, stage_id=exit_id
            )
        )

    while sim.agent_count() > 0 and sim.iteration_count() < 2000:
        sim.iterate()

    writer.close()
    print(
        f"Wrote {out.resolve()} ({sim.iteration_count()} iterations, "
        f"every_nth_frame=4)"
    )

    traj = pedpy.load_trajectory_from_ped_data_archive_hdf5(trajectory_file=out)
    walkable_area = pedpy.load_walkable_area_from_ped_data_archive_hdf5(
        trajectory_file=out
    )
    print(
        f"Loaded {len(traj.data)} rows, {traj.data['id'].nunique()} agents, "
        f"fps={traj.frame_rate}"
    )

    fig, ax = plt.subplots(figsize=(8, 6))
    pedpy.plot_trajectories(traj=traj, walkable_area=walkable_area, axes=ax)
    ax.set_aspect("equal")
    ax.set_title("Hdf5TrajectoryWriter -> pedpy.plot_trajectories")
    png = out.with_suffix(".png")
    fig.savefig(png, dpi=120, bbox_inches="tight")
    print(f"Saved plot to {png.resolve()}")


if __name__ == "__main__":
    main()
