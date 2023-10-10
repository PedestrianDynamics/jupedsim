#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import sys

from shapely import GeometryCollection, Polygon

import jupedsim as jps


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    area = GeometryCollection(
        Polygon([(0, 0), (100, 0), (100, 100), (0, 100)])
    )

    simulation = jps.Simulation(
        model=jps.VelocityModel(),
        geometry=area,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example2_out.sqlite"),
        ),
    )
    stage_id = simulation.add_waiting_set_stage(
        [
            (60, 50),
            (59, 50),
            (58, 50),
        ]
    )
    stage = simulation.get_stage_proxy(stage_id)
    exits = [
        simulation.add_exit_stage([(99, 40), (99, 60), (100, 60), (100, 40)]),
        simulation.add_exit_stage([(99, 50), (99, 70), (100, 70), (100, 50)]),
    ]
    waypoints = [
        simulation.add_waypoint_stage((50, 50), 1),
        simulation.add_waypoint_stage((60, 40), 1),
        simulation.add_waypoint_stage((40, 40), 1),
        simulation.add_waypoint_stage((40, 60), 1),
        simulation.add_waypoint_stage((60, 60), 1),
    ]

    short_journey = jps.JourneyDescription([waypoints[0], stage_id, exits[0]])
    long_journey = jps.JourneyDescription([*waypoints[1:], exits[1]])

    short_journey.set_transition_for_stage(
        waypoints[0], jps.Transition.create_fixed_transition(stage_id)
    )
    short_journey.set_transition_for_stage(
        stage_id, jps.Transition.create_fixed_transition(exits[0])
    )

    for waypoint_start, waypoint_end in zip(waypoints[1:-1], waypoints[2:]):
        long_journey.set_transition_for_stage(
            waypoint_start,
            jps.Transition.create_fixed_transition(waypoint_end),
        )
    long_journey.set_transition_for_stage(
        waypoints[-1],
        jps.Transition.create_fixed_transition(exits[1]),
    )

    journeys = [
        simulation.add_journey(short_journey),
        simulation.add_journey(long_journey),
    ]

    agent_parameters = jps.VelocityModelAgentParameters(
        journey_id=journeys[0], stage_id=waypoints[0], radius=0.3
    )

    agent_parameters.position = (10, 50)
    simulation.add_agent(agent_parameters)

    agent_parameters.position = (8, 50)
    simulation.add_agent(agent_parameters)

    agent_parameters.position = (6, 50)
    simulation.add_agent(agent_parameters)

    redirect_once = True
    signal_once = True
    while simulation.agent_count() > 0:
        try:
            agents_at_head_of_waiting = list(
                simulation.agents_in_range((60, 50), 1)
            )
            if redirect_once and len(agents_at_head_of_waiting) == 1:
                simulation.switch_agent_journey(
                    agent_id=agents_at_head_of_waiting[0],
                    journey_id=journeys[1],
                    stage_id=waypoints[1],
                )
                redirect_once = False
                print(
                    f"Switched journey for agent {agents_at_head_of_waiting[0]} @{simulation.iteration_count()}"
                )

            if signal_once and any(simulation.agents_in_range((60, 60), 1)):
                stage.state = jps.WaitingSetState.INACTIVE
                print(f"Stop Waiting @{simulation.iteration_count()}")
                signal_once = False
            simulation.iterate()
        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)


if __name__ == "__main__":
    main()
