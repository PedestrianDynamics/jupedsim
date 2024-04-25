#! /usr/bin/env python3

# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import logging
import pathlib
import random
import sys
import time

import jupedsim as jps

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
        start_stage: int,
        max: int | None = None,
    ):
        self.sim = sim
        self.dt = dt
        self.stop_at = stop_at
        self.point_a = point_a
        self.profile_picker = profile_picker
        self.dir = (point_b[0] - point_a[0], point_b[1] - point_a[1])
        self.agent_parameters = jps.CollisionFreeSpeedModelAgentParameters()
        self.agent_parameters.journey_id = journey_id
        self.agent_parameters.stage_id = start_stage
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
                self.sim.add_agent(
                    self.profile_picker.randomise_radius_and_v0(
                        self.agent_parameters
                    )
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

    def randomise_radius_and_v0(
        self, agent: jps.CollisionFreeSpeedModelAgentParameters
    ) -> jps.CollisionFreeSpeedModelAgentParameters:
        new_agent = jps.CollisionFreeSpeedModelAgentParameters()
        new_agent.position = agent.position
        new_agent.orientation = agent.orientation
        new_agent.journey_id = agent.journey_id
        new_agent.stage_id = agent.stage_id
        new_agent.time_gap = agent.time_gap
        new_agent.v0 = self._rnd.gauss(mu=self._mu_v0, sigma=self._sigma_v0)
        new_agent.radius = self._rnd.gauss(
            mu=self._mu_d / 2, sigma=self._sigma_d / 2
        )
        return new_agent


def create_journey(sim: jps.Simulation):
    stages = [
        sim.add_waiting_set_stage(
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
        ),
        sim.add_waypoint_stage((1283.35, 510.25), 1.5),
        sim.add_waypoint_stage((1159.81, 693.19), 1.5),
        sim.add_waypoint_stage((1223.74, 768.90), 1.5),
        sim.add_waypoint_stage((1214.52, 766.20), 1.5),
        sim.add_waypoint_stage((962.36, 555.14), 1.5),
        sim.add_queue_stage(
            [
                (950.56, 538.72),
                (952.46, 538.19),
                (953.89, 537.66),
                (955.61, 536.76),
                (957.04, 536.47),
                (958.46, 536.88),
            ]
        ),
        sim.add_exit_stage(
            [
                (630.01, 25.88),
                (630.03, 27.63),
                (625.97, 28.03),
                (625.92, 26.18),
            ]
        ),
    ]

    journey = jps.JourneyDescription(stages)
    for stages_start, stage_end in zip(stages[0:-1], stages[1:]):
        journey.set_transition_for_stage(
            stages_start,
            jps.Transition.create_fixed_transition(stage_end),
        )

    queue = sim.get_stage(stages[-2])
    waiting_area = sim.get_stage(stages[0])
    return sim.add_journey(journey), (stages[0], waiting_area, queue)


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
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=geometries["large_street_network"],
        trajectory_writer=stats_writer,
    )

    journey, (start_stage, waiting_area, queue) = create_journey(simulation)
    spawners = [
        Spawner(
            simulation,
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
            if (iteration + 100 * 30) % (100 * 60) == 0:
                waiting_area.state = jps.WaitingSetState.INACTIVE
            if iteration % (100 * 60) == 0:
                waiting_area.state = jps.WaitingSetState.ACTIVE
            if iteration % (100 * 8) == 0:
                queue.pop(1)
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
        except KeyboardInterrupt:
            print("\nCTRL-C Received! Shutting down")
            sys.exit(1)


if __name__ == "__main__":
    main()
