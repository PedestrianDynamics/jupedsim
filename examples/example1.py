#! /usr/bin/env python3

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
import sys

from shapely import GeometryCollection, Polygon

import jupedsim as jps


def main():
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_warning_callback(lambda x: logging.debug(x))
    jps.set_error_callback(lambda x: logging.debug(x))

    p1 = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    area = GeometryCollection(p1.union(p2))
    geometry = jps.geometry_from_shapely(area)

    simulation = jps.Simulation(
        model=jps.VelocityModelParameters(),
        geometry=geometry,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example1_out.sqlite"),
        ),
    )

    stage_id = simulation.add_waiting_set_stage([(16, 5), (15, 5), (14, 5)])
    waiting_stage = simulation.get_stage_proxy(stage_id)
    assert isinstance(waiting_stage, jps.WaitingSetProxy)
    exit_id = simulation.add_exit_stage([(18, 4), (20, 4), (20, 6), (18, 6)])

    journey = jps.JourneyDescription()
    journey.add(stage_id)
    journey.add(exit_id)
    journey.set_transition_for_stage(
        stage_id, jps.Transition.create_fixed_transition(exit_id)
    )
    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.stage_id = stage_id
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.time_gap = 1
    agent_parameters.tau = 0.5
    agent_parameters.v0 = 1.2
    agent_parameters.radius = 0.3

    for new_pos in [(7, 7), (1, 3), (1, 5), (1, 7), (2, 7)]:
        agent_parameters.position = new_pos
        simulation.add_agent(agent_parameters)

    print("Running simulation")

    while simulation.agent_count() > 0:
        try:
            simulation.iterate()
            if simulation.iteration_count() == 1300:
                if waiting_stage.state == jps.WaitingSetState.ACTIVE:
                    waiting_stage.state = jps.WaitingSetState.INACTIVE
        except KeyboardInterrupt:
            print("CTRL-C Recieved! Shuting down")
            sys.exit(1)
    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )


if __name__ == "__main__":
    main()
