#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Starter example 01: Geometry, operational model, agents, and iteration loop."""

import jupedsim as jps


def main() -> None:
    builder = jps.GeometryBuilder()
    corridor = builder.add_region(
        boundary=[(0.0, 0.0), (20.0, 0.0), (20.0, 4.0), (0.0, 4.0)],
        height=0.0,
    )

    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    exit_id = sim.add_exit_stage(
        [([(19.0, 0.5), (20.0, 0.5), (20.0, 3.5), (19.0, 3.5)], corridor)]
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    for y in (1.0, 2.0, 3.0):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(1.5, y),
            state=jps.CollisionFreeSpeedModelV2State(
                desired_speed=1.3, radius=0.2
            ),
            region_id=corridor,
        )

    while sim.agent_count() > 0:
        sim.iterate()

    print(
        f"Evacuated all agents in {sim.iteration_count()} iterations "
        f"({sim.elapsed_time():.2f} s)."
    )


if __name__ == "__main__":
    main()
