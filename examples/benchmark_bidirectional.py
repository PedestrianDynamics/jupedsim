# SPDX-License-Identifier: LGPL-3.0-or-later
"""Benchmark: bidirectional corridor flow — WarpDriver vs SocialForce."""

import time

import jupedsim as jps
import shapely


def build_simulation(model, corridor_length=50.0, corridor_width=4.0, trajectory_writer=None):
    area = shapely.Polygon(
        [
            (0, 0),
            (corridor_length, 0),
            (corridor_length, corridor_width),
            (0, corridor_width),
        ]
    )
    sim = jps.Simulation(
        model=model, geometry=area, dt=0.01, trajectory_writer=trajectory_writer
    )

    exit_left = shapely.Polygon(
        [(0, 0), (1, 0), (1, corridor_width), (0, corridor_width)]
    )
    exit_right = shapely.Polygon(
        [
            (corridor_length - 1, 0),
            (corridor_length, 0),
            (corridor_length, corridor_width),
            (corridor_length - 1, corridor_width),
        ]
    )
    exit_left_id = sim.add_exit_stage(exit_left)
    exit_right_id = sim.add_exit_stage(exit_right)

    journey_right = sim.add_journey(jps.JourneyDescription([exit_right_id]))
    journey_left = sim.add_journey(jps.JourneyDescription([exit_left_id]))

    return sim, journey_right, exit_right_id, journey_left, exit_left_id


def add_agents_warp(sim, journey_right, exit_right_id, journey_left, exit_left_id, n=50):
    spacing_y = 3.0 / max(n // 5, 1)
    for i in range(n):
        row = i // 5
        col = i % 5
        x = 3.0 + col * 1.0
        y = 0.5 + row * spacing_y
        sim.add_agent(
            jps.WarpDriverModelAgentParameters(
                position=(x, y),
                orientation=(1, 0),
                journey_id=journey_right,
                stage_id=exit_right_id,
                desired_speed=1.2,
                radius=0.15,
            )
        )
    for i in range(n):
        row = i // 5
        col = i % 5
        x = 47.0 - col * 1.0
        y = 0.5 + row * spacing_y
        sim.add_agent(
            jps.WarpDriverModelAgentParameters(
                position=(x, y),
                orientation=(-1, 0),
                journey_id=journey_left,
                stage_id=exit_left_id,
                desired_speed=1.2,
                radius=0.15,
            )
        )


def add_agents_social(sim, journey_right, exit_right_id, journey_left, exit_left_id, n=50):
    spacing_y = 3.0 / max(n // 5, 1)
    for i in range(n):
        row = i // 5
        col = i % 5
        x = 3.0 + col * 1.0
        y = 0.5 + row * spacing_y
        sim.add_agent(
            jps.SocialForceModelAgentParameters(
                position=(x, y),
                orientation=(1, 0),
                journey_id=journey_right,
                stage_id=exit_right_id,
                desired_speed=1.2,
                radius=0.15,
            )
        )
    for i in range(n):
        row = i // 5
        col = i % 5
        x = 47.0 - col * 1.0
        y = 0.5 + row * spacing_y
        sim.add_agent(
            jps.SocialForceModelAgentParameters(
                position=(x, y),
                orientation=(-1, 0),
                journey_id=journey_left,
                stage_id=exit_left_id,
                desired_speed=1.2,
                radius=0.15,
            )
        )


def run_benchmark(name, model, add_fn, n_agents=50, max_steps=5000):
    sqlite_file = f"{name.lower().replace(' ', '_')}.sqlite"
    writer = jps.SqliteTrajectoryWriter(output_file=sqlite_file)
    sim, jr, erid, jl, elid = build_simulation(model, trajectory_writer=writer)
    add_fn(sim, jr, erid, jl, elid, n=n_agents)
    initial = sim.agent_count()

    t0 = time.perf_counter()
    steps = 0
    for steps in range(1, max_steps + 1):
        sim.iterate()
        if sim.agent_count() == 0:
            break
    elapsed = time.perf_counter() - t0

    remaining = sim.agent_count()
    print(f"\n{'=' * 50}")
    print(f"  {name}")
    print(f"{'=' * 50}")
    print(f"  Agents       : {initial} ({n_agents} per side)")
    print(f"  Steps        : {steps}")
    print(f"  dt           : 0.01 s")
    print(f"  Sim time     : {steps * 0.01:.1f} s")
    print(f"  Wall clock   : {elapsed:.3f} s")
    print(f"  ms/step      : {elapsed / steps * 1000:.2f}")
    print(f"  Steps/s      : {steps / elapsed:.0f}")
    print(f"  Remaining    : {remaining}")
    print(f"  Trajectory   : {sqlite_file}")
    return elapsed, steps


def main():
    n = 50
    print(f"Bidirectional corridor benchmark: {n} agents per side (100 total)")
    print(f"Corridor: 50m x 4m, dt=0.01s, max 5000 steps (50s sim time)")

    t_sf, s_sf = run_benchmark(
        "SocialForceModel",
        jps.SocialForceModel(),
        add_agents_social,
        n_agents=n,
    )

    t_wd, s_wd = run_benchmark(
        "WarpDriverModel",
        jps.WarpDriverModel(),
        add_agents_warp,
        n_agents=n,
    )

    print(f"\n{'=' * 50}")
    print(f"  Summary")
    print(f"{'=' * 50}")
    print(f"  SocialForce  : {t_sf:.3f}s  ({s_sf} steps, {t_sf/s_sf*1000:.2f} ms/step)")
    print(f"  WarpDriver   : {t_wd:.3f}s  ({s_wd} steps, {t_wd/s_wd*1000:.2f} ms/step)")
    ratio = t_wd / t_sf if t_sf > 0 else float("inf")
    print(f"  Ratio (WD/SF): {ratio:.1f}x")


if __name__ == "__main__":
    main()
