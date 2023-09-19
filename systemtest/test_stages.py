# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pytest
import shapely

import jupedsim as jps
from jupedsim import Transition


def test_can_share_queue_between_stages():
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
    simulation = jps.Simulation(
        model=jps.VelocityModelParameters(), geometry=geometry, dt=0.01
    )

    wp_j1 = simulation.add_waypoint_stage((-1, 0), 0.5)

    common_exit = simulation.add_exit_stage(
        [(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)]
    )

    common_queue = simulation.add_queue_stage(
        [(0, -9), (0, -8), (0, -7), (0, -6), (0, -5), (0, -4)]
    )
    queue = simulation.get_stage_proxy(common_queue)

    journey1 = jps.JourneyDescription(
        [
            wp_j1,
            common_queue,
            common_exit,
        ]
    )
    journey1.set_transition_for_stage(
        wp_j1, Transition.create_fixed_transition(common_queue)
    )
    journey1.set_transition_for_stage(
        common_queue, Transition.create_fixed_transition(common_exit)
    )
    wp_j2 = simulation.add_waypoint_stage((1, 0), 0.5)
    journey2 = jps.JourneyDescription([wp_j2, common_queue, common_exit])
    journey2.set_transition_for_stage(
        wp_j2, Transition.create_fixed_transition(common_queue)
    )
    journey2.set_transition_for_stage(
        common_queue, Transition.create_fixed_transition(common_exit)
    )

    journeys = [
        (simulation.add_journey(journey1), wp_j1),
        (simulation.add_journey(journey2), wp_j2),
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
    agent_parameters.time_gap = 1
    agent_parameters.tau = 0.5
    agent_parameters.v0 = 1.2
    agent_parameters.radius = 0.15

    for pos, (journey_id, stage_id) in agents:
        agent_parameters.position = pos
        agent_parameters.journey_id = journey_id
        agent_parameters.stage_id = stage_id
        simulation.add_agent(agent_parameters)

    while simulation.agent_count() > 0:
        if simulation.iteration_count() % 4 == 0:
            pass
        if (
            max(simulation.iteration_count() - 3000, 0) > 0
            and simulation.iteration_count() % 200 == 0
        ):
            queue.pop(1)
        simulation.iterate()


def test_can_use_stage_proxy():
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
    simulation = jps.Simulation(
        model=jps.VelocityModelParameters(), geometry=geometry, dt=0.01
    )

    exit_id = simulation.add_exit_stage(
        [(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)]
    )

    waypoint_id = simulation.add_waypoint_stage((9.5, 0), 1)

    exit_journey_id = simulation.add_journey(
        jps.JourneyDescription(
            [
                exit_id,
            ]
        )
    )

    entry_journey_id = simulation.add_journey(
        jps.JourneyDescription(
            [
                waypoint_id,
            ]
        )
    )

    exit = simulation.get_stage_proxy(exit_id)
    waypoint = simulation.get_stage_proxy(waypoint_id)

    assert exit.count_targeting() == 0
    assert waypoint.count_targeting() == 0

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (-9.5, 0)
    agent_parameters.journey_id = exit_journey_id
    agent_parameters.stage_id = exit_id
    agent_parameters.time_gap = 1
    agent_parameters.tau = 0.5
    agent_parameters.v0 = 1.2
    agent_parameters.radius = 0.15
    agent_id = simulation.add_agent(agent_parameters)

    assert exit.count_targeting() == 1
    assert waypoint.count_targeting() == 0

    simulation.iterate()

    assert exit.count_targeting() == 1
    assert waypoint.count_targeting() == 0

    simulation.switch_agent_journey(
        agent_id=agent_id, journey_id=entry_journey_id, stage_id=waypoint_id
    )

    assert exit.count_targeting() == 0
    assert waypoint.count_targeting() == 1

    simulation.iterate()

    assert exit.count_targeting() == 0
    assert waypoint.count_targeting() == 1
