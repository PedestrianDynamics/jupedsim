#! /usr/bin/env python3

# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import sys

import jupedsim as jps
from shapely import GeometryCollection, Polygon


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    p1 = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    p2 = Polygon([(10, 4), (20, 4), (20, 6), (10, 6)])
    area = GeometryCollection(p1.union(p2))

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=area,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("example1_out.sqlite"),
        ),
    )

    stage_id = simulation.add_waiting_set_stage([(16, 5), (15, 5), (14, 5)])
    waiting_stage = simulation.get_stage(stage_id)
    assert isinstance(waiting_stage, jps.WaitingSetStage)
    exit_id = simulation.add_exit_stage([(18, 4), (20, 4), (20, 6), (18, 6)])

    journey = jps.JourneyDescription()
    journey.add(stage_id)
    journey.add(exit_id)
    journey.set_transition_for_stage(
        stage_id, jps.Transition.create_fixed_transition(exit_id)
    )
    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.CollisionFreeSpeedModelAgentParameters(
        journey_id=journey_id, stage_id=stage_id, radius=0.3
    )

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
