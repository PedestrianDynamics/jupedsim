#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import logging
import pathlib
import random
import sys
import time

import jupedsim as jps
import shapely
from performancetest.stats_writer import StatsWriter

from performancetest.geometry import geometries


def log_debug(msg):
    print("\n")
    logging.debug(msg)


def log_info(msg):
    print("\n")
    logging.info(msg)


def log_warn(msg):
    print("\n")
    logging.warning(msg)


def log_error(msg):
    print("\n")
    logging.error(msg)


class Spawner:
    def __init__(
        self,
        sim: jps.Simulation,
        region_id: int,
        dt: int,
        stop_at: int,
        point_a: tuple[float, float],
        point_b: tuple[float, float],
        profile_picker,
        journey_id: int,
        start_stage: int,
        max: int | None = None,
    ):
        self.sim = sim
        self.region_id = region_id
        self.dt = dt
        self.stop_at = stop_at
        self.point_a = point_a
        self.profile_picker = profile_picker
        self.dir = (point_b[0] - point_a[0], point_b[1] - point_a[1])
        self.journey_id = journey_id
        self.start_stage = start_stage
        self._needs_placement = 0
        self.max_agents = max
        self.spawned = 0

    def spawn(self, iteration: int):
        if self.max_agents and self.spawned >= self.max_agents:
            return
        if iteration > self.stop_at:
            return
        if iteration % self.dt == 0:
            self._needs_placement += 1
        if self._needs_placement > 0:
            offset = random.uniform(0, 1)
            p = (
                self.point_a[0] + offset * self.dir[0],
                self.point_a[1] + offset * self.dir[1],
            )
            if len(list(self.sim.agents_in_range(p, 0.6))) == 0:
                self.sim.add_agent(
                    journey_id=self.journey_id,
                    stage_id=self.start_stage,
                    position=p,
                    state=self.profile_picker.random_state(),
                    region_id=self.region_id,
                )
                self._needs_placement -= 1
                self.spawned += 1


class RandomProfilePicker:
    def __init__(self, *, mu_v0, sigma_v0, mu_d, sigma_d, seed=123456):
        self._rnd = random.Random(seed)
        self._mu_v0 = mu_v0
        self._sigma_v0 = sigma_v0
        self._mu_d = mu_d
        self._sigma_d = sigma_d

    def random_state(self) -> jps.CollisionFreeSpeedModelState:
        return jps.CollisionFreeSpeedModelState(
            orientation=(1.0, 0.0),
            desired_speed=self._rnd.gauss(mu=self._mu_v0, sigma=self._sigma_v0),
            radius=self._rnd.gauss(mu=self._mu_d / 2, sigma=self._sigma_d / 2),
        )


def create_journey(sim: jps.Simulation, region_id: int):
    stages = [
        sim.add_waypoint_stage((1384.33, 635.51), 1.5, region_id=region_id),
        sim.add_waypoint_stage((1283.35, 510.25), 1.5, region_id=region_id),
        sim.add_waypoint_stage((1159.81, 693.19), 1.5, region_id=region_id),
        sim.add_waypoint_stage((1223.74, 768.90), 1.5, region_id=region_id),
        sim.add_waypoint_stage((1214.52, 766.20), 1.5, region_id=region_id),
        sim.add_waypoint_stage((962.36, 555.14), 1.5, region_id=region_id),
        sim.add_waypoint_stage((950.56, 538.72), 1.5, region_id=region_id),
        sim.add_exit_stage(
            [
                (630.01, 25.88),
                (630.03, 27.63),
                (625.97, 28.03),
                (625.92, 26.18),
            ],
            region_id=region_id,
        ),
    ]

    journey = jps.JourneyDescription(stages)
    for stages_start, stage_end in zip(stages[0:-1], stages[1:]):
        journey.set_transition_for_stage(
            stages_start,
            jps.Transition.create_fixed_transition(stage_end),
        )

    return sim.add_journey(journey), stages[0]


def parse_args():
    ap = argparse.ArgumentParser(
        description="Runs performace test with 'large_street_network'"
    )
    ap.add_argument(
        "--verbose",
        "-v",
        action="count",
        default=0,
        help="verbosity level, -v, -vv, -vvv are supported",
    )
    ap.add_argument(
        "--limit",
        "-l",
        type=int,
        default=100 * 60 * 15,
        help="number of iterations to run",
    )
    return ap.parse_args()


def main():
    args = parse_args()
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    if args.verbose >= 3:
        jps.set_debug_callback(log_debug)
    if args.verbose >= 2:
        jps.set_info_callback(log_info)
    if args.verbose >= 1:
        jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

    profile_picker = RandomProfilePicker(
        mu_v0=1.34, sigma_v0=0.25, mu_d=0.15, sigma_d=0.015
    )

    stats_writer = StatsWriter(
        jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path(
                f"{jps.get_build_info().git_commit_hash}_large_street_network.sqlite"
            ),
        )
    )
    (street_network,) = shapely.from_wkt(
        geometries["large_street_network"]
    ).geoms
    surface = jps.WalkableSurface()
    region = surface.add_region(polygon=street_network)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        trajectory_writer=stats_writer,
    )

    journey, start_stage = create_journey(simulation, region)
    spawners = [
        Spawner(
            simulation,
            region,
            5,
            90000,
            (1455.05, 533.89),
            (1456.38, 534.73),
            profile_picker,
            journey,
            start_stage,
            1024,
        ),
    ]

    start_time = time.perf_counter_ns()
    iteration = simulation.iteration_count()
    while args.limit == 0 or iteration < args.limit:
        try:
            for s in spawners:
                s.spawn(iteration)
            simulation.iterate()
            iteration = simulation.iteration_count()

            dt = (time.perf_counter_ns() - start_time) / 1000000000
            duration = simulation.timer.iteration_duration_us
            op_dur = simulation.timer.operational_level_duration_us

            print(
                f"WC-Time: {dt:6.2f}s "
                f"S-Time: {iteration / 100:6.2f}s "
                f"I: {iteration:6d} "
                f"Agents: {simulation.agent_count():4d} "
                f"ItTime: {duration / 1000:6.2f}ms "
                f"[OpLvl {op_dur / 1000:6.2f}ms]",
                end="\r",
            )
        except KeyboardInterrupt:
            print("\nCTRL-C Received! Shutting down")
            stats_writer.close()
            sys.exit(1)
    stats_writer.close()


if __name__ == "__main__":
    main()
