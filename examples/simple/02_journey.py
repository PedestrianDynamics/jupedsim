#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Starter example 02: Exits, waypoints, and journey transitions."""

import jupedsim as jps


def main() -> None:
    builder = jps.GeometryBuilder()
    hall = builder.add_region(
        boundary=[(0.0, 0.0), (30.0, 0.0), (30.0, 16.0), (0.0, 16.0)],
        height=0.0,
    )

    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    entry_wp = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(10.0, 8.0), radius=1.5), hall)]
    )
    split_wp = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(18.0, 8.0), radius=1.5), hall)]
    )
    exit_north = sim.add_exit_stage(
        [([(29.0, 11.0), (30.0, 11.0), (30.0, 15.0), (29.0, 15.0)], hall)]
    )
    exit_south = sim.add_exit_stage(
        [([(29.0, 1.0), (30.0, 1.0), (30.0, 5.0), (29.0, 5.0)], hall)]
    )

    journey = jps.JourneyDescription(
        [entry_wp, split_wp, exit_north, exit_south]
    )
    journey.set_transition_for_stage(
        entry_wp, jps.Transition.create_fixed_transition(split_wp)
    )
    journey.set_transition_for_stage(
        split_wp,
        jps.Transition.create_round_robin_transition(
            [(exit_north, 2), (exit_south, 1)]
        ),
    )
    journey_id = sim.add_journey(journey)

    for idx in range(6):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=entry_wp,
            position=(2.0, 3.0 + idx * 2.0),
            state=jps.CollisionFreeSpeedModelV2State(
                desired_speed=1.3, radius=0.2
            ),
            region_id=hall,
        )

    while sim.agent_count() > 0:
        sim.iterate()

    print(
        f"All 6 agents completed their journeys in {sim.elapsed_time():.2f} s."
    )


if __name__ == "__main__":
    main()
