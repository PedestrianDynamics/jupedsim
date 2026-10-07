# SPDX-License-Identifier: LGPL-3.0-or-later

"""Example: HDF5 trajectory writer.

Runs a short simulation, writes the trajectory to an HDF5 file, then reads
the file back with h5py and plots the trajectories on top of the walkable
area.

Extra dependencies for this example (beyond JuPedSim itself):

    pip install matplotlib

The script writes ``example_traj.h5`` and ``example_traj.png`` to the
current working directory.
"""

import pathlib

import h5py
import hdf5plugin  # noqa: F401  registers the Zstd filter used by the writer
import jupedsim as jps
import matplotlib.pyplot as plt
import shapely
from shapely import GeometryCollection, Polygon


def main() -> None:
    room1 = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    room2 = Polygon([(15, 0), (25, 0), (25, 10), (15, 10)])
    corridor = Polygon([(10, 4.5), (28, 4.5), (28, 5.5), (10, 5.5)])

    area = GeometryCollection(corridor.union(room1.union(room2)))
    out = pathlib.Path("example_traj.h5")
    with jps.Hdf5TrajectoryWriter(output_file=out, every_nth_frame=4) as writer:
        sim = jps.Simulation(
            model=jps.CollisionFreeSpeedModelV2(),
            geometry=area,
            trajectory_writer=writer,
            dt=0.01,
        )
        exit_area = Polygon([(27, 4.5), (28, 4.5), (28, 5.5), (27, 5.5)])
        exit_id = sim.add_exit_stage(exit_area, region_id=0)
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
                journey_id=journey_id,
                stage_id=exit_id,
                position=position,
                state=jps.CollisionFreeSpeedModelV2State(),
                region_id=0,
            )

        while sim.agent_count() > 0 and sim.iteration_count() < 2000:
            sim.iterate()
    print(
        f"Wrote {out.resolve()} ({sim.iteration_count()} iterations, "
        f"every_nth_frame=4)"
    )

    with h5py.File(out, "r") as hf:
        traj = hf["trajectory"][:]
        regions = [shapely.from_wkt(wkt) for wkt in hf["regions"]["wkt"]]
    print(
        f"Loaded {len(traj)} rows, {len(set(traj['id']))} agents, "
        f"{traj['frame'].max() + 1} frames"
    )

    fig, ax = plt.subplots(figsize=(8, 6))
    for region in regions:
        for ring in [region.exterior, *region.interiors]:
            ax.plot(*ring.xy, color="black", linewidth=1)
    for agent_id in sorted(set(traj["id"])):
        rows = traj[traj["id"] == agent_id]
        ax.plot(rows["x"], rows["y"], linewidth=0.5)
    ax.set_aspect("equal")
    ax.set_title("Hdf5TrajectoryWriter output")
    png = out.with_suffix(".png")
    fig.savefig(png, dpi=120, bbox_inches="tight")
    print(f"Saved plot to {png.resolve()}")


if __name__ == "__main__":
    main()
