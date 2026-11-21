#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Starter example 05: Manual step-by-step agent control via DirectSteering."""

import math

import jupedsim as jps


def main() -> None:
    builder = jps.GeometryBuilder()
    arena = builder.add_region(
        boundary=[(-10.0, -10.0), (10.0, -10.0), (10.0, 10.0), (-10.0, 10.0)],
        height=0.0,
    )

    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    steering_stage = sim.add_direct_steering_stage()
    journey_id = sim.add_journey(jps.JourneyDescription([steering_stage]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=steering_stage,
        position=(5.0, 0.0),
        state=jps.CollisionFreeSpeedModelV2State(desired_speed=1.4, radius=0.2),
        region_id=arena,
    )

    # Steer the agent along a circular path of radius 5 m for 10 seconds
    orbit_radius = 5.0
    angular_rate = 0.35
    while sim.elapsed_time() < 10.0:
        angle = angular_rate * sim.elapsed_time() + 0.3
        tx = orbit_radius * math.cos(angle)
        ty = orbit_radius * math.sin(angle)
        sim.agent(agent_id).final_target = sim.get_location(
            tx, ty, region_id=arena
        )
        sim.iterate()

    final_x, final_y = sim.agent(agent_id).position
    print(
        f"Direct-steered agent reached ({final_x:.2f}, {final_y:.2f}) "
        f"after {sim.iteration_count()} iterations."
    )


if __name__ == "__main__":
    main()
