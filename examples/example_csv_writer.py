#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later

"""Example: Custom CSV trajectory writer.

Demonstrates how to implement a custom TrajectoryWriter that outputs
agent trajectory data to a CSV file instead of the built-in SQLite format.
"""

import csv
import pathlib
import sys

import jupedsim as jps
from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation
from shapely import GeometryCollection, Polygon


class CsvTrajectoryWriter(TrajectoryWriter):
    """Write trajectory data to a CSV file."""

    def __init__(
        self,
        output_file: pathlib.Path,
        every_nth_frame: int = 1,
    ) -> None:
        if every_nth_frame < 1:
            raise TrajectoryWriter.Exception("'every_nth_frame' must be > 0")

        self._output_file = output_file
        self._every_nth_frame = every_nth_frame
        self._file = None
        self._csv_writer = None

    def begin_writing(self, simulation: Simulation) -> None:
        self._file = open(self._output_file, "w", newline="")
        self._csv_writer = csv.writer(self._file)
        self._csv_writer.writerow(
            ["frame", "id", "pos_x", "pos_y", "ori_x", "ori_y"]
        )

    def write_iteration_state(self, simulation: Simulation) -> None:
        iteration = simulation.iteration_count()
        if iteration % self._every_nth_frame != 0:
            return

        frame = iteration // self._every_nth_frame
        for agent in simulation.agents():
            self._csv_writer.writerow(
                [
                    frame,
                    agent.id,
                    agent.position[0],
                    agent.position[1],
                    agent.orientation[0],
                    agent.orientation[1],
                ]
            )

    def every_nth_frame(self) -> int:
        return self._every_nth_frame

    def close(self) -> None:
        """Flush and close the CSV file."""
        if self._file:
            self._file.close()
            self._file = None


def main():
    p1 = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    area = GeometryCollection(p1.union(p2))

    output_file = pathlib.Path("example_csv_out.csv")

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=area,
        trajectory_writer=CsvTrajectoryWriter(
            output_file=output_file,
            every_nth_frame=5,
        ),
    )

    exit_id = simulation.add_exit_stage([(18, 4), (20, 4), (20, 6), (18, 6)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.CollisionFreeSpeedModelAgentParameters(
        journey_id=journey_id, stage_id=exit_id, radius=0.3
    )

    for pos in [(1, 2), (3, 8), (5, 5)]:
        agent_parameters.position = pos
        simulation.add_agent(agent_parameters)

    print(f"Running simulation with CsvTrajectoryWriter -> {output_file}")

    while simulation.agent_count() > 0:
        try:
            simulation.iterate()
        except KeyboardInterrupt:
            print("CTRL-C Received! Shutting down")
            sys.exit(1)

    simulation._writer.close()

    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )

    # Read back CSV and plot trajectories over geometry
    import matplotlib.pyplot as plt
    import numpy as np

    data = np.genfromtxt(output_file, delimiter=",", skip_header=1)
    agent_ids = np.unique(data[:, 1]).astype(int)

    fig, ax = plt.subplots(figsize=(10, 5))

    # Draw geometry
    geom = p1.union(p2)
    x_geom, y_geom = geom.exterior.xy
    ax.plot(x_geom, y_geom, "k-", linewidth=2)
    ax.fill(x_geom, y_geom, alpha=0.1, color="gray")

    # Draw exit
    exit_rect = Polygon([(18, 4), (20, 4), (20, 6), (18, 6)])
    x_exit, y_exit = exit_rect.exterior.xy
    ax.fill(x_exit, y_exit, alpha=0.3, color="green", label="Exit")

    # Plot each agent's trajectory
    for agent_id in agent_ids:
        mask = data[:, 1] == agent_id
        ax.plot(
            data[mask, 2],
            data[mask, 3],
            ".-",
            markersize=2,
            label=f"Agent {agent_id}",
        )

    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_title("Trajectories from CSV output")
    ax.set_aspect("equal")
    ax.legend()
    plt.tight_layout()
    plt.savefig("example_csv_trajectories.png", dpi=150)
    print("Plot saved to example_csv_trajectories.png")
    plt.show()


if __name__ == "__main__":
    main()
