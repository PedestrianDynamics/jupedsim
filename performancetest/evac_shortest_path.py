#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
import sys
import time

import jupedsim as jps
import jupedsim.distributions
import pandas as pd
import shapely
from numpy.random import normal
from shapely import affinity

timer_key_list = [
    "Add Agent",
    "Agent Removal System",
    "Neighborhood Search",
    "Operational Decision System",
    "Strategical Decision System",
    "Tactical Decision System",
    "Total Simulation Time",
]


def log_debug(msg):
    logging.debug(msg)


def log_info(msg):
    logging.info(msg)


def log_warn(msg):
    logging.warning(msg)


def log_error(msg):
    logging.error(msg)


def main(
    df_res: pd.DataFrame,
    number_of_runs: int = 1,
    number_of_iterations: int = 100,
    number_of_agents: int = 15000,
):
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_debug_callback(log_debug)
    jps.set_info_callback(log_info)
    jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

    geo_collections = shapely.from_wkt(
        pathlib.Path("./geometry/Geometrie_verwinkelt_v1.wkt").read_text()
    )
    # shift to center to make visualizer work
    cp = geo_collections.centroid
    offset = (-cp.x, -cp.y)

    geo_collections = affinity.translate(geo_collections, offset[0], offset[1])

    geo = geo_collections.geoms[0]
    exits = geo_collections.geoms[1]
    sources = geo_collections.geoms[2]
    # waypoints = geo_collections.geoms[3]
    for run in range(0, number_of_runs):
        surface = jps.WalkableSurface()
        region = surface.add_region(polygon=geo.geoms[0])
        simulation = jps.Simulation(
            model=jps.AnticipationVelocityModel(),
            geometry=surface,
            timer_log_level=3,
        )

        # jps.enable_tracing()
        # spawning areas
        # Source coordinates
        source_coordinates = []  # only one spawning area
        for source in sources.geoms:
            source_coordinates.append(list(source.exterior.coords))

        exit_id = simulation.add_exit_stage(
            [(exit, region) for exit in exits.geoms]
        )
        journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))

        number_agents_stage = int(number_of_agents * 2 / 5)
        number_agents_remaining = number_of_agents - number_agents_stage
        number_agents = number_agents_stage + number_agents_remaining

        spawning_area_stage = shapely.Polygon(source_coordinates[0])
        spawning_area_rest = shapely.difference(geo, spawning_area_stage)

        pos_stage = jps.distributions.distribute_by_number(
            polygon=spawning_area_stage,
            number_of_agents=number_agents_stage,
            distance_to_agents=0.3,
            distance_to_polygon=0.3,
            seed=234567,
        )

        pos_rest = jps.distributions.distribute_by_number(
            polygon=spawning_area_rest,
            number_of_agents=number_agents_remaining,
            distance_to_agents=0.3,
            distance_to_polygon=0.3,
            seed=234567,
        )
        agent_positions = pos_stage + pos_rest

        agent_parameters = jps.AnticipationVelocityModelState()
        agent_parameters.radius = 0.15
        v_distribution = normal(1.34, 0.05, number_agents)

        for position, desired_speed in zip(agent_positions, v_distribution):
            agent_parameters.desired_speed = desired_speed
            simulation.add_agent(
                journey_id=journey_id,
                stage_id=exit_id,
                position=position,
                state=agent_parameters,
                region_id=region,
            )

        start_time = time.perf_counter_ns()
        while (
            simulation.agent_count() > 0
            and simulation.iteration_count() < number_of_iterations
        ):
            try:
                simulation.iterate()
                dt = (time.perf_counter_ns() - start_time) / 1000000000
                duration = simulation.timer.iteration_duration_us
                iteration = simulation.iteration_count()
                print(
                    f"Run: {run + 1:2d}/{number_of_runs} "
                    f"WC-Time: {dt:6.2f}s "
                    f"S-Time: {iteration / 100:6.2f}s "
                    f"I: {iteration:6d} "
                    f"Agents: {simulation.agent_count():4d} "
                    f"ItTime: {duration / 1000:6.2f}ms ",
                    f"writeTime: {duration / 1000:6.2f}ms ",
                    end="\r",
                )

            except KeyboardInterrupt:
                print("CTRL-C Received! Shutting down")
                jps.dump_traces("evac_shortest_path_v1.prof")
                sys.exit(1)

        res_dict = {}

        for key in timer_key_list:
            res_dict[key] = simulation.timer.elapsed_time_us(key)
        df_res = pd.concat(
            [df_res, pd.DataFrame(res_dict, index=[0])], ignore_index=True
        )
    return df_res


if __name__ == "__main__":
    res = pd.DataFrame(columns=timer_key_list)
    res = main(
        df_res=res,
        number_of_runs=50,
        number_of_iterations=200,
        number_of_agents=15000,
    )
    res.to_csv("evac_shortest_path_deque_ptr_15k_flt.csv", index=False)
