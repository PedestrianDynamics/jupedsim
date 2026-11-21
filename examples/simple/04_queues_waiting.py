#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Starter example 04: Queuing / timed waiting before release to an exit."""

import jupedsim as jps


def main() -> None:
    builder = jps.GeometryBuilder()
    lobby = builder.add_region(
        boundary=[(0.0, 0.0), (24.0, 0.0), (24.0, 8.0), (0.0, 8.0)],
        height=0.0,
    )

    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    waiting_zone = (10.0, 4.0)
    waiting_wp = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=waiting_zone, radius=1.5), lobby)]
    )
    exit_id = sim.add_exit_stage(
        [([(23.0, 2.5), (24.0, 2.5), (24.0, 5.5), (23.0, 5.5)], lobby)]
    )

    hold_journey_id = sim.add_journey(jps.JourneyDescription([waiting_wp]))
    release_journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    arrival_time: dict[int, float] = {}
    released: set[int] = set()
    hold_duration_s = 3.0
    release_interval_s = 1.0
    next_release_time = 0.0

    for idx in range(5):
        sim.add_agent(
            journey_id=hold_journey_id,
            stage_id=waiting_wp,
            position=(2.0 + idx * 1.1, 4.0),
            state=jps.CollisionFreeSpeedModelV2State(
                desired_speed=1.3, radius=0.2
            ),
            region_id=lobby,
        )

    while sim.agent_count() > 0:
        now = sim.elapsed_time()
        for agent in sim.agents_in_range(waiting_zone, 2.0):
            if agent.id in released:
                continue
            if agent.id not in arrival_time:
                arrival_time[agent.id] = now
                agent.state.desired_speed = 0.0
            elif (
                now - arrival_time[agent.id] >= hold_duration_s
                and now >= next_release_time
            ):
                agent.state.desired_speed = 1.3
                sim.switch_agent_journey(
                    agent_id=agent.id,
                    journey_id=release_journey_id,
                    stage_id=exit_id,
                )
                released.add(agent.id)
                next_release_time = now + release_interval_s
                print(
                    f"Released agent {agent.id} from waiting area at t={now:.2f} s"
                )
        sim.iterate()

    print(
        f"All queued agents released and exited by t={sim.elapsed_time():.2f} s."
    )


if __name__ == "__main__":
    main()
