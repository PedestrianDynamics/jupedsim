#! /usr/bin/env python3
# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
import sys
import time

import py_jupedsim as jps
from jupedsim.trajectory_writer_sqlite import SqliteTrajectoryWriter
from jupedsim.util import build_jps_geometry
from shapely import GeometryCollection, Polygon, to_wkt


def log_debug(msg):
    logging.debug(msg)


def log_info(msg):
    logging.info(msg)


def log_warn(msg):
    logging.warning(msg)


def log_error(msg):
    logging.error(msg)


def main():
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_debug_callback(log_debug)
    jps.set_info_callback(log_info)
    jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

    area = GeometryCollection(
        Polygon(shell=[(0, 0), (1000, 0), (1000, 5000), (0, 5000), (0, 0)])
    )
    geometry = build_jps_geometry(area)

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.3
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    journey = jps.JourneyDescription()
    journey.add_exit([(999, 2000), (999, 3000), (1000, 3000), (1000, 2000)])
    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    for y in range(0, 5000):
        for x in range(0, 12):
            agent_parameters.position = (0.5 + x, y + 0.5)
            simulation.add_agent(agent_parameters)

    writer = SqliteTrajectoryWriter(pathlib.Path("example4_out.sqlite"))
    writer.begin_writing(5, to_wkt(area, rounding_precision=-1))
    while simulation.agent_count() > 0:
        try:
            if simulation.iteration_count() % 20 == 0:
                writer.write_iteration_state(simulation)
            before = time.perf_counter_ns()
            simulation.iterate()
            duration = time.perf_counter_ns() - before
            print(
                f"Iteration: {simulation.iteration_count():3.0f} / Time taken: {duration / 1000000}ms",
                end="\r",
            )
        except KeyboardInterrupt:
            writer.end_writing()
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)

    writer.end_writing()


if __name__ == "__main__":
    main()
