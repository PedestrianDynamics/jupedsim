#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
import sys

from shapely import GeometryCollection, Polygon, to_wkt

import jupedsim as jps


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
        Polygon(
            [
                (-2, -2),
                (-50, -2),
                (-50, 2),
                (-2, 2),
                (-2, 25),
                (2, 25),
                (2, 2),
                (35, 2),
                (35, -2),
                (2, -2),
                (2, -25),
                (-2, -25),
            ]
        )
    )
    geometry = jps.geometry_from_shapely(area)

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.3
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    exit_top = simulation.add_exit_stage(
        [(-2, 24), (2, 24), (2, 25), (-2, 25)]
    )
    exit_right = simulation.add_exit_stage(
        [(34, -2), (34, 2), (35, 2), (35, -2)]
    )
    exit_bottom = simulation.add_exit_stage(
        [(-2, -24), (2, -24), (2, -25), (-2, -25)]
    )

    waypoint_middle = simulation.add_queue_stage(
        [
            (0, 0),
            (0, -2),
            (0, -8),
        ]
    )
    queue = simulation.get_stage_proxy(waypoint_middle)

    journey = jps.JourneyDescription(
        [waypoint_middle, exit_top, exit_right, exit_bottom]
    )

    journey.set_transition_for_stage(
        waypoint_middle,
        jps.Transition.create_least_targeted_transition(
            [exit_top, exit_right, exit_bottom]
        ),
    )

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.stage_id = waypoint_middle
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id

    for x in range(-49, -9, 1):
        agent_parameters.position = (x, 0)
        simulation.add_agent(agent_parameters)

    writer = jps.SqliteTrajectoryWriter(
        pathlib.Path("example5_out_FOO.sqlite")
    )
    writer.begin_writing(100, to_wkt(area, rounding_precision=-1))

    while simulation.agent_count() > 0:
        try:
            if simulation.iteration_count() % 200 == 0:
                queue.pop(1)
                print("Next!")

            writer.write_iteration_state(simulation)

            simulation.iterate()

        except KeyboardInterrupt:
            writer.end_writing()
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)

    writer.end_writing()


if __name__ == "__main__":
    main()
