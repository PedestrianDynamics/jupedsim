# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib

import pytest
import shapely

import jupedsim as jps
from jupedsim.trajectory_writer_sqlite import SqliteTrajectoryWriter


def test_can_share_queue_between_stages(tmp_path):
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    polygon = shapely.union(
        shapely.Polygon([(-10, 2.5), (-10, -2.5), (10, -2.5), (10, 2.5)]),
        shapely.Polygon([(-2.5, 2.5), (-2.5, -10), (2.5, -10), (2.5, 2.5)]),
    )

    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area(polygon.exterior.coords[:-1])
    geometry = geo_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 1
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.2, radius=0.15
    )

    model = model_builder.build()

    simulation = jps.Simulation(model=model, geometry=geometry, dt=0.01)

    common_exit = simulation.add_exit_stage(
        [(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)]
    )

    common_queue = simulation.add_queue_stage(
        [(0, -9), (0, -8), (0, -7), (0, -6), (0, -5), (0, -4)]
    )

    journey1 = jps.JourneyDescription(
        [
            simulation.add_waypoint_stage((-1, 0), 0.5),
            common_queue,
            common_exit,
        ]
    )
    journey2 = jps.JourneyDescription(
        [simulation.add_waypoint_stage((1, 0), 0.5), common_queue, common_exit]
    )

    journeys = [
        simulation.add_journey(journey1),
        simulation.add_journey(journey2),
    ]

    agents = [
        ((-9.5, 0), journeys[0]),
        ((-8.5, 0), journeys[0]),
        ((-7.5, 0), journeys[0]),
        ((-6.5, 0), journeys[0]),
        ((-5.5, 0), journeys[0]),
        ((9.5, 0), journeys[1]),
        ((8.5, 0), journeys[1]),
        ((7.5, 0), journeys[1]),
        ((6.5, 0), journeys[1]),
        ((5.5, 0), journeys[1]),
    ]

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.profile_id = profile_id

    for agent in agents:
        agent_parameters.position = agent[0]
        agent_parameters.journey_id = agent[1]
        simulation.add_agent(agent_parameters)

    # writer = SqliteTrajectoryWriter(
    #    tmp_path / "test_can_share_queue_between_stages.sqlite"
    # )
    # writer.begin_writing(
    #    25,
    #    shapely.to_wkt(
    #        shapely.GeometryCollection(polygon), rounding_precision=-1
    #    ),
    # )
    while simulation.agent_count() > 0:
        try:
            if simulation.iteration_count() % 4 == 0:
                # writer.write_iteration_state(simulation)
                pass
            if (
                max(simulation.iteration_count() - 3000, 0) > 0
                and simulation.iteration_count() % 200 == 0
            ):
                simulation.notify_queue(common_queue, 1)
            simulation.iterate()
        except:
            # writer.write_iteration_state(simulation)
            break
    # writer.end_writing()
