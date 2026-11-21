#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Starter example 03: Multi-stage routing around interior obstacles."""

import jupedsim as jps
import shapely


def main() -> None:
    outer = shapely.box(0.0, 0.0, 26.0, 14.0)
    obstacles = shapely.union_all(
        [
            shapely.box(8.0, 0.0, 9.0, 9.5),
            shapely.box(17.0, 4.5, 18.0, 14.0),
        ]
    )
    walkable = outer.difference(obstacles)

    builder = jps.GeometryBuilder()
    floor = builder.add_region(polygon=walkable, height=0.0)

    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    upper_pass = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(8.5, 11.8), radius=1.0), floor)]
    )
    lower_pass = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(17.5, 2.2), radius=1.0), floor)]
    )
    exit_top = sim.add_exit_stage(
        [([(25.0, 9.5), (26.0, 9.5), (26.0, 13.0), (25.0, 13.0)], floor)]
    )
    exit_bottom = sim.add_exit_stage(
        [([(25.0, 1.0), (26.0, 1.0), (26.0, 4.5), (25.0, 4.5)], floor)]
    )

    journey = jps.JourneyDescription(
        [upper_pass, lower_pass, exit_top, exit_bottom]
    )
    journey.set_transition_for_stage(
        upper_pass, jps.Transition.create_fixed_transition(lower_pass)
    )
    journey.set_transition_for_stage(
        lower_pass,
        jps.Transition.create_least_targeted_transition(
            [exit_top, exit_bottom]
        ),
    )
    journey_id = sim.add_journey(journey)

    spawn_box = shapely.box(1.0, 1.5, 5.5, 12.5)
    for pos in jps.distribute_by_number(
        polygon=spawn_box,
        number_of_agents=8,
        distance_to_agents=0.8,
        distance_to_polygon=0.4,
        seed=42,
    ):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=upper_pass,
            position=pos,
            state=jps.CollisionFreeSpeedModelV2State(
                desired_speed=1.35, radius=0.2
            ),
            region_id=floor,
        )

    while sim.agent_count() > 0:
        sim.iterate()

    print(
        f"Multi-stage obstacle routing finished in {sim.elapsed_time():.2f} s."
    )


if __name__ == "__main__":
    main()
