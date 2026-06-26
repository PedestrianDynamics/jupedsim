#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import math
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


def distance(point1, point2):
    return math.sqrt(
        (point1[0] - point2[0]) ** 2 + (point1[1] - point2[1]) ** 2
    )


def get_travelled_distance(path_waypoints):
    d = 0
    for i in range(len(path_waypoints) - 1):
        d = d + distance(path_waypoints[i], path_waypoints[i + 1])
    return d


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
        simulation = jps.Simulation(
            model=jps.AnticipationVelocityModel(),
            geometry=geo,
            timer_log_level=3,
        )

        # jps.enable_tracing()
        # spawning areas
        # Source coordinates
        source_coordinates = []  # only one spawning area
        for source in sources.geoms:
            source_coordinates.append(list(source.exterior.coords))

        # exit areas
        exit_polygons = []
        exit_ids = []  # sequence: A, B, C, D, E

        for exit in exits.geoms:
            exit_polygons.append(shapely.Polygon(list(exit.exterior.coords)))
            exit_ids.append(
                simulation.add_exit_stage(list(exit.exterior.coords))
            )

        journey = jps.JourneyDescription(exit_ids)
        journey_id = simulation.add_journey(journey)

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

        agent_parameters = jps.AnticipationVelocityModelAgentParameters()
        agent_parameters.radius = 0.15
        v_distribution = normal(1.34, 0.05, number_agents)

        routing = jps.RoutingEngine(geo.geoms[0])

        for i in range(len(agent_positions)):
            agent_parameters.position = (
                agent_positions[i][0],
                agent_positions[i][1],
            )
            agent_parameters.desired_speed = v_distribution[i]
            agent_parameters.journey_id = journey_id

            # choose shortest path
            min_distance = float("inf")
            min_exit_id = -1

            for j in range(len(exit_polygons)):
                # calc path to exit
                polygon = exit_polygons[j]
                exit_point = (
                    polygon.centroid.xy[0][0],
                    polygon.centroid.xy[1][0],
                )
                path = routing.compute_waypoints(agent_positions[i], exit_point)
                distance_to_exit = get_travelled_distance(path)
                if distance_to_exit < min_distance:
                    min_distance = distance_to_exit
                    min_exit_id = j

            agent_parameters.stage_id = exit_ids[min_exit_id]
            simulation.add_agent(agent_parameters)

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
