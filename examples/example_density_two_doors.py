#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
"""A crowd leaving a large room through two doors, rerouted by the congestion it causes.

Run from the repo root:

    python examples/example_density_two_doors.py

        y
     30 +-----------------+-+---------+
        |                 | |         |
     24 |                 far         |
        |                 | |         |
        |      room       | | outside |
        |                 | |         |
     14 |. . . . . . .    | |         |
        | . crowd . . .   | |         |
      6 |. . . . . . .  near     [X]  |  X = the only target
        | . . . . . . .   | |         |
      0 +-----------------+-+---------+ x
        0                20 21       30

Everyone targets the exit X. The crowd starts next to the near door, which is also on
the target's side, so the far door is a 12 to 21 m longer way for everyone. Without
congestion nobody has a reason to take it. But DensityDependentSpeed lowers the speeds
in the floor field where the jam at the near door grows, and once waiting there costs
more than the detour, agents turn to the far door.

How many agents went through each door so far is printed every 10 s. The trajectory goes to an HDF5 file (``--output``).
"""

import argparse
import pathlib

import jupedsim as jps
import jupedsim.distributions
import numpy as np
import shapely

ROOM = (0.0, 20.0)
WALL = (20.0, 21.0)
OUTSIDE = (21.0, 30.0)
HEIGHT = 30.0
NEAR_DOOR_Y = 6.0
FAR_DOOR_Y = 24.0
EXIT_POLYGON = [(28.5, 5.0), (29.5, 5.0), (29.5, 7.0), (28.5, 7.0)]
CROWD_POLYGON = [(1.0, 1.0), (17.0, 1.0), (17.0, 14.0), (1.0, 14.0)]

# Iterations between two updates of the density-dependent speed fields.
DENSITY_UPDATE_INTERVAL = 50
# Iterations between two lines of the progress report.
REPORT_INTERVAL = 1000
MAX_ITERATIONS = 100000


class DensityDependentSpeed:
    """Slows the exit's speed field where agents crowd, so its floor field routes
    agents around jams. Call update() regularly, e.g. every second of simulated time.

    update() measures the density: every agent is smeared over a Gaussian, corrected
    near walls for the part of the Gaussian that falls on non-walkable cells, and the
    result is averaged over time to damp oscillations between the exits. The density
    is turned into a factor on the speed field the geometry starts out with: 1 up to
    rho_start, then falling linearly towards 0 at rho_congestion, but never below
    min_factor. The exit's floor field then holds the travel time under the current
    congestion.

    Args:
        sim: The simulation; its default speed fields at construction are the base
            the factor applies to.
        sigma: Width (standard deviation) of the Gaussian in m: the area around a
            point that counts as crowded with it.
        time_constant: Time constant in s of the average over time. Larger values
            make the routes change more slowly.
        min_factor: Lowest speed factor, the slowdown of the densest crowd.
        rho_start: Density in persons/m² from which on the speeds are lowered.
        rho_congestion: Density in persons/m² at which the factor would reach 0;
            min_factor is reached somewhat before it.
    """

    def __init__(
        self,
        sim,
        *,
        sigma=1.0,
        time_constant=1.0,
        min_factor=0.11,
        rho_start=1.5,
        rho_congestion=6.4,
    ):
        self._tau, self._min = time_constant, min_factor
        self._rho_start, self._rho_congestion = rho_start, rho_congestion
        # region id -> SpeedGrid, kept fixed: the factor always applies to these
        self._base = sim.default_speed_fields()
        cell_size = next(iter(self._base.values())).cell_size  # one lattice
        half = int(np.ceil(3 * sigma / cell_size))
        k = np.exp(-0.5 * (np.arange(-half, half + 1) * cell_size / sigma) ** 2)
        self._kernel = k / k.sum()
        self._walkable = {r: g.values > 0 for r, g in self._base.items()}
        # Share of the kernel landing on walkable cells: corrects the density
        # near walls.
        self._coverage = {
            r: np.maximum(self._blur(w.astype(float)), 0.25)
            for r, w in self._walkable.items()
        }
        self._density = {
            r: np.zeros_like(g.values) for r, g in self._base.items()
        }
        self._last = None

    def _blur(self, a):
        h = (len(self._kernel) - 1) // 2
        # Full convolution, then cut: also right for kernels wider than the grid.
        for axis in (0, 1):
            a = np.apply_along_axis(
                lambda v: np.convolve(v, self._kernel)[h : h + len(v)], axis, a
            )
        return a

    def density(self, sim):
        """Smoothed density in persons/m² per region, at this instant."""
        counts = {r: np.zeros_like(g.values) for r, g in self._base.items()}
        for agent in sim.agents():
            r = agent.location.region_id
            g = self._base[r]
            x, y = agent.position
            row = int((y - g.origin[1]) / g.cell_size)
            col = int((x - g.origin[0]) / g.cell_size)
            counts[r][row, col] += 1.0
        return {
            r: np.where(
                self._walkable[r],
                self._blur(c / self._base[r].cell_size ** 2)
                / self._coverage[r],
                0.0,
            )
            for r, c in counts.items()
        }

    def update(self, sim, stage_id):
        now = sim.elapsed_time()
        alpha = (
            1.0
            if self._last is None
            else 1 - np.exp(-(now - self._last) / self._tau)
        )
        self._last = now
        for r, rho in self.density(sim).items():
            ema = self._density[r] + alpha * (rho - self._density[r])
            self._density[r] = ema
            factor = 1.0 - (ema - self._rho_start) / (
                self._rho_congestion - self._rho_start
            )
            factor = np.clip(factor, self._min, 1.0)
            sim.set_custom_speed_field(
                stage_id, r, self._base[r].values * factor
            )


def rect(x0, y0, x1, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


def walkable_area(door_width):
    parts = [
        rect(ROOM[0], 0, ROOM[1], HEIGHT),
        rect(OUTSIDE[0], 0, OUTSIDE[1], HEIGHT),
    ]
    for door_y in (NEAR_DOOR_Y, FAR_DOOR_Y):
        parts.append(
            rect(
                WALL[0],
                door_y - door_width / 2,
                WALL[1],
                door_y + door_width / 2,
            )
        )
    return shapely.union_all([shapely.Polygon(p) for p in parts])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=pathlib.Path("density_two_doors.h5"),
        help="HDF5 file to write the trajectory to",
    )
    parser.add_argument(
        "--agents", type=int, default=400, help="number of agents"
    )
    parser.add_argument(
        "--door-width", type=float, default=1.2, help="width of both doors"
    )
    parser.add_argument(
        "--every-nth-frame",
        type=int,
        default=10,
        help="write every nth iteration to the trajectory file",
    )
    args = parser.parse_args()

    surface = jps.WalkableSurface()
    region = surface.add_region(polygon=walkable_area(args.door_width))
    writer = jps.Hdf5TrajectoryWriter(
        output_file=args.output,
        every_nth_frame=args.every_nth_frame,
        write_travel_times=True,
        every_nth_travel_time_field=5,
    )
    ff_options = jps.FloorfieldOptions(wall_influence_radius=1.0)
    simulation = jps.Simulation(
        model=jps.AnticipationVelocityModel(),
        geometry=surface,
        trajectory_writer=writer,
        floorfield_options=ff_options,
    )
    exit_id = simulation.add_exit_stage([(EXIT_POLYGON, region)])
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))

    positions = jps.distributions.distribute_by_number(
        polygon=shapely.Polygon(CROWD_POLYGON),
        number_of_agents=args.agents,
        distance_to_agents=0.4,
        distance_to_polygon=0.2,
        seed=1,
    )
    for position in positions:
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=position,
            state=jps.AnticipationVelocityModelState(radius=0.15),
            region_id=region,
        )

    crowding = DensityDependentSpeed(simulation)
    door_of = {}  # agent id -> door it left the room through

    def report():
        near = sum(door == "near" for door in door_of.values())
        print(
            f"t = {simulation.elapsed_time():6.1f} s  "
            f"in room: {sum(a.position[0] < WALL[1] for a in simulation.agents()):4d}  "
            f"through near door: {near:4d}  "
            f"through far door: {len(door_of) - near:4d}"
        )

    while (
        simulation.agent_count() > 0
        and simulation.iteration_count() < MAX_ITERATIONS
    ):
        iteration = simulation.iteration_count()
        if iteration % DENSITY_UPDATE_INTERVAL == 0:
            crowding.update(simulation, exit_id)
        if iteration % REPORT_INTERVAL == 0:
            report()
        simulation.iterate()
        for agent in simulation.agents():
            if agent.id not in door_of and agent.position[0] > WALL[1]:
                door_of[agent.id] = (
                    "near"
                    if agent.position[1] < (NEAR_DOOR_Y + FAR_DOOR_Y) / 2
                    else "far"
                )
    report()
    writer.close()


if __name__ == "__main__":
    main()
