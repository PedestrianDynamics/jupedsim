#! /usr/bin/env python3
# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import logging
import pathlib
import random
import sys
import time

import py_jupedsim as jps
import shapely
from jupedsim.trajectory_writer_sqlite import SqliteTrajectoryWriter
from jupedsim.util import build_jps_geometry
from shapely import to_wkt

from performancetest.geometry import geometries
from performancetest.stats_writer import StatsWriter


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
        dt: int,
        stop_at: int,
        point_a: tuple[float, float],
        point_b: tuple[float, float],
        profile_picker,
        journey_id: int,
        max: int | None = None,
    ):
        self.sim = sim
        self.dt = dt
        self.stop_at = stop_at
        self.point_a = point_a
        self.profile_picker = profile_picker
        self.dir = (point_b[0] - point_a[0], point_b[1] - point_a[1])
        self.agent_parameters = jps.VelocityModelAgentParameters()
        self.agent_parameters.journey_id = journey_id
        self.agent_parameters.profile_id = 0
        self.agent_parameters.orientation = (1.0, 0.0)
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
                self.agent_parameters.position = p
                self.agent_parameters.profile_id = self.profile_picker.pick()
                self.sim.add_agent(self.agent_parameters)
                self._needs_placement -= 1
                self.spawned += 1


class RandomProfilePicker:
    def __init__(self, *, mu_v0, sigma_v0, mu_d, sigma_d):
        # middle bin represents mu, first reprsents mu - 3 sigma, last bin represents mu + 3 sigma
        self._num_bins = 13
        self._profiles = {}
        self._mu_v0 = mu_v0
        self._sigma_v0 = sigma_v0
        self._mu_d = mu_d
        self._sigma_d = sigma_d

    def create_profiles(self, model_builder: jps.VelocityModelBuilder):
        self._profiles = {
            (v0_idx, d_idx): self._num_bins * v0_idx + d_idx
            for v0_idx in range(0, self._num_bins)
            for d_idx in range(0, self._num_bins)
        }
        for k, v in self._profiles.items():
            model_builder.add_parameter_profile(
                id=v,
                time_gap=1,
                tau=0.5,
                v0=self._mu_v0 + 0.5 * self._sigma_v0 * (k[0] - 6),
                radius=(self._mu_d + 0.5 * self._sigma_d * (k[1] - 6)) / 2,
            )

    def pick(self):
        v0_idx = self._to_idx(random.gauss(6, 2))
        d_idx = self._to_idx(random.gauss(6, 2))
        return self._profiles[(v0_idx, d_idx)]

    def _to_idx(self, f: float):
        return min(max(round(f), 0), self._num_bins - 1)


def create_journey(sim: jps.Simulation):
    journey = jps.JourneyDescription()
    first_wait = journey.add_notifiable_waiting_set(
        [
            (1384.33, 635.51),
            (1384.91, 636.33),
            (1385.91, 637.61),
            (1385.60, 634.71),
            (1386.56, 635.85),
            (1387.84, 636.44),
            (1387.56, 634.27),
            (1388.91, 634.85),
        ]
    )
    journey.add_waypoint((1283.35, 510.25), 1.5)
    journey.add_waypoint((1159.81, 693.19), 1.5)
    journey.add_waypoint((1223.74, 768.90), 1.5)
    journey.add_waypoint((1214.52, 766.20), 1.5)
    journey.add_waypoint((962.36, 555.14), 1.5)
    second_wait = journey.add_notifiable_queue(
        [
            (950.56, 538.72),
            (952.46, 538.19),
            (953.89, 537.66),
            (955.61, 536.76),
            (957.04, 536.47),
            (958.46, 536.88),
        ]
    )
    journey.add_exit(
        [(630.01, 25.88), (630.03, 27.63), (625.97, 28.03), (625.92, 26.18)]
    )
    return sim.add_journey(journey), (first_wait, second_wait)


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

    geo = shapely.from_wkt(geometries["large_street_network"])
    geometry = build_jps_geometry(geo)

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_picker = RandomProfilePicker(
        mu_v0=1.34, sigma_v0=0.25, mu_d=0.15, sigma_d=0.015
    )
    profile_picker.create_profiles(model_builder)
    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)
    simulation.set_tracing(True)

    journey, wait_points = create_journey(simulation)
    spawners = [
        Spawner(
            simulation,
            5,
            90000,
            (1455.05, 533.89),
            (1456.38, 534.73),
            profile_picker,
            journey,
            1024,
        ),
    ]

    writer = SqliteTrajectoryWriter(
        pathlib.Path("large_street_network.sqlite")
    )
    stats_writer = StatsWriter(writer.connection())
    writer.begin_writing(2, to_wkt(geo, rounding_precision=-1))

    start_time = time.perf_counter_ns()
    iteration = simulation.iteration_count()
    while args.limit == 0 or iteration < args.limit:
        try:
            for s in spawners:
                s.spawn(iteration)
            if (iteration + 100 * 30) % (100 * 60) == 0:
                simulation.notify_waiting_set(journey, wait_points[0], False)
            if iteration % (100 * 60) == 0:
                simulation.notify_waiting_set(journey, wait_points[0], True)
            if iteration % (100 * 8) == 0:
                simulation.notify_queue(journey, wait_points[1], 1)
            if iteration % 50 == 0:
                writer.write_iteration_state(simulation)
            simulation.iterate()
            iteration = simulation.iteration_count()

            dt = (time.perf_counter_ns() - start_time) / 1000000000
            duration = simulation.get_last_trace().iteration_duration
            op_dur = simulation.get_last_trace().operational_level_duration

            print(
                f"WC-Time: {dt:6.2f}s "
                f"S-Time: {iteration / 100:6.2f}s "
                f"I: {iteration:6d} "
                f"Agents: {simulation.agent_count():4d} "
                f"ItTime: {duration / 1000:6.2f}ms "
                f"[OpLvl {op_dur / 1000:6.2f}ms]",
                end="\r",
            )
            stats_writer.write_stats(
                simulation.iteration_count(),
                simulation.agent_count(),
                simulation.get_last_trace(),
            )
        except KeyboardInterrupt:
            writer.end_writing()
            print("\nCTRL-C Recieved! Shuting down")
            sys.exit(1)

    writer.end_writing()


if __name__ == "__main__":
    main()
