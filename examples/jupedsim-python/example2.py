#! /usr/bin/env python3
# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib

import py_jupedsim as jps
import shapely
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
    # jps.set_debug_callback(log_debug)
    jps.set_info_callback(log_info)
    jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

    area = GeometryCollection(
        Polygon([(0, 0), (100, 0), (100, 100), (0, 100)])
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

    journey1 = jps.JourneyDescription()
    journey1.add_waypoint((50, 50), 1)
    stage = journey1.add_notifiable_waiting_set(
        [
            (60, 50),
            (59, 50),
            (58, 50),
        ]
    )
    journey1.add_exit([(99, 40), (99, 60), (100, 60), (100, 40)])

    journey2 = jps.JourneyDescription()
    journey2.add_waypoint((60, 40), 1)
    journey2.add_waypoint((40, 40), 1)
    journey2.add_waypoint((40, 60), 1)
    journey2.add_waypoint((60, 60), 1)
    journey2.add_exit([(99, 50), (99, 70), (100, 70), (100, 50)])

    journeys = []
    journeys.append(simulation.add_journey(journey1))
    journeys.append(simulation.add_journey(journey2))

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journeys[0]
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    agent_parameters.position = (10, 50)
    simulation.add_agent(agent_parameters)

    agent_parameters.position = (8, 50)
    simulation.add_agent(agent_parameters)

    agent_parameters.position = (6, 50)
    simulation.add_agent(agent_parameters)

    writer = SqliteTrajectoryWriter(pathlib.Path("example2_out.sqlite"))
    writer.begin_writing(
        25,
        to_wkt(area, rounding_precision=-1),
    )
    redirect_once = True
    signal_once = True
    while simulation.agent_count() > 0:
        agents_at_head_of_waiting = list(
            simulation.agents_in_range((60, 50), 1)
        )
        if redirect_once and len(agents_at_head_of_waiting) == 1:
            simulation.switch_agent_journey(
                agent_id=agents_at_head_of_waiting[0],
                journey_id=journeys[1],
                stage_index=0,
            )
            redirect_once = False
            print(
                f"Switched journey for agent {agents_at_head_of_waiting[0]} @{simulation.iteration_count()}"
            )

        if signal_once and any(simulation.agents_in_range((60, 60), 1)):
            simulation.notify_waiting_set(journeys[0], stage, False)
            print(f"Stop Waiting @{simulation.iteration_count()}")
            signal_once = False
        if simulation.iteration_count() % 4 == 0:
            writer.write_iteration_state(simulation)
        simulation.iterate()

    writer.end_writing()


if __name__ == "__main__":
    main()
