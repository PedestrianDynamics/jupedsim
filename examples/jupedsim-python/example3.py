#! /usr/bin/env python3
# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib

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
        Polygon([(0, 0), (100, 0), (100, 100), (0, 100), (0, 0)])
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
    stage = journey.add_notifiable_queue(
        [
            (60, 50),
            (59, 50),
            (58, 50),
            (57, 50),
            (56, 50),
            (55, 50),
            (54, 50),
        ]
    )
    journey.add_exit([(99, 40), (99, 60), (100, 60), (100, 40)])
    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    for y in range(1, 16):
        agent_parameters.position = (0.5, y)
        simulation.add_agent(agent_parameters)

    writer = SqliteTrajectoryWriter(pathlib.Path("example3_out.sqlite"))
    writer.begin_writing(25, to_wkt(area, rounding_precision=-1))
    while simulation.agent_count() > 0:
        if (
            simulation.iteration_count() > 100 * 52
            and simulation.iteration_count() % 400 == 0
        ):
            simulation.notify_queue(journey_id, stage, 1)
            print("Next!")

        if simulation.iteration_count() % 4 == 0:
            writer.write_iteration_state(simulation)
        simulation.iterate()

    writer.end_writing()


if __name__ == "__main__":
    main()
