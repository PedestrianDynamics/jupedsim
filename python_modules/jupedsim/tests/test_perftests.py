# SPDX-License-Identifier: LGPL-3.0-or-later
"""Subsystem performance & smoke stress tests for JuPedSim.

By default (during normal ``pytest`` runs), each scenario executes ``1``
iteration with a compact agent count so that setup, routing, spatial grid
updates, stage transitions, and all operational models are smoke-tested
without slowing down CI.

To run full multi-step profiling from the command line or via pytest::

    JPS_PERF_ITERATIONS=500 JPS_PERF_SCALE=5 pytest python_modules/jupedsim/tests/test_perftests.py -s
    python python_modules/jupedsim/tests/test_perftests.py --iterations 500 --scale 5
"""

from __future__ import annotations

import argparse
import os
import time
from collections.abc import Callable

import jupedsim as jps
import pytest
import shapely


def _perf_iterations() -> int:
    return max(1, int(os.environ.get("JPS_PERF_ITERATIONS", "1")))


def _perf_scale() -> int:
    return max(1, int(os.environ.get("JPS_PERF_SCALE", "1")))


def _make_agent_state(
    state_cls: type, orientation: tuple[float, float] = (1.0, 0.0)
):
    if state_cls is jps.GeneralizedCentrifugalForceModelState:
        return state_cls(
            orientation=orientation,
            desired_speed=1.2,
            a_min=0.18,
            b_min=0.18,
            b_max=0.35,
        )
    if state_cls is jps.SocialForceModelState:
        return state_cls(desired_speed=1.2, radius=0.2)
    return state_cls(orientation=orientation, desired_speed=1.2, radius=0.18)


MODEL_FACTORIES: list[tuple[str, Callable[[], object], type, float]] = [
    (
        "CFS",
        jps.CollisionFreeSpeedModel,
        jps.CollisionFreeSpeedModelState,
        0.01,
    ),
    (
        "CFSV2",
        jps.CollisionFreeSpeedModelV2,
        jps.CollisionFreeSpeedModelV2State,
        0.01,
    ),
    (
        "CFSV3",
        jps.CollisionFreeSpeedModelV3,
        jps.CollisionFreeSpeedModelV3State,
        0.01,
    ),
    (
        "AVM",
        jps.AnticipationVelocityModel,
        jps.AnticipationVelocityModelState,
        0.01,
    ),
    (
        "GCFM",
        jps.GeneralizedCentrifugalForceModel,
        jps.GeneralizedCentrifugalForceModelState,
        0.01,
    ),
    (
        "SocialForce",
        jps.SocialForceModel,
        jps.SocialForceModelState,
        0.005,
    ),
    (
        "WarpDriver",
        jps.WarpDriverModel,
        jps.WarpDriverModelState,
        0.02,
    ),
]


def run_routing_obstacle_hall(iterations: int, scale: int) -> float:
    """Stress RoutingEngine and TacticalDecisionSystem in a multi-pillar hall."""
    hall = shapely.box(0.0, 0.0, 40.0, 20.0)
    pillars = [
        shapely.box(cx - 0.8, cy - 0.8, cx + 0.8, cy + 0.8)
        for cx in (12.0, 20.0, 28.0)
        for cy in (5.0, 10.0, 15.0)
    ]
    walkable = hall.difference(shapely.union_all(pillars))

    builder = jps.GeometryBuilder()
    region = builder.add_region(polygon=walkable, height=0.0)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    exit_north = sim.add_exit_stage(
        [([(39.0, 12.0), (40.0, 12.0), (40.0, 18.0), (39.0, 18.0)], region)]
    )
    exit_south = sim.add_exit_stage(
        [([(39.0, 2.0), (40.0, 2.0), (40.0, 8.0), (39.0, 8.0)], region)]
    )
    gate_wp = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(8.0, 10.0), radius=2.0), region)]
    )

    journey = jps.JourneyDescription([gate_wp, exit_north, exit_south])
    journey.set_transition_for_stage(
        gate_wp,
        jps.Transition.create_least_targeted_transition(
            [exit_north, exit_south]
        ),
    )
    journey_id = sim.add_journey(journey)

    spawn = shapely.box(1.0, 2.0, 6.0, 18.0)
    n_agents = min(16 * scale, 120)
    positions = jps.distribute_by_number(
        polygon=spawn,
        number_of_agents=n_agents,
        distance_to_agents=0.55,
        distance_to_polygon=0.3,
        seed=42,
    )
    for pos in positions:
        sim.add_agent(
            journey_id=journey_id,
            stage_id=gate_wp,
            position=pos,
            state=jps.CollisionFreeSpeedModelV2State(radius=0.18),
            region_id=region,
        )

    t0 = time.perf_counter()
    sim.iterate(count=iterations)
    elapsed = time.perf_counter() - t0
    assert sim.iteration_count() == iterations
    assert "Total Simulation Time" in str(sim.timer)
    return elapsed


def run_dense_bottleneck_neighborhood(iterations: int, scale: int) -> float:
    """Stress NeighborhoodSearch grid updates and short-range repulsion."""
    left_room = shapely.box(0.0, 0.0, 16.0, 14.0)
    corridor = shapely.box(16.0, 6.0, 20.0, 8.0)
    right_room = shapely.box(20.0, 0.0, 28.0, 14.0)
    walkable = left_room.union(corridor).union(right_room)

    builder = jps.GeometryBuilder()
    region = builder.add_region(polygon=walkable, height=0.0)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=builder.build(),
        dt=0.01,
    )

    exit_id = sim.add_exit_stage(
        [([(27.0, 4.0), (28.0, 4.0), (28.0, 10.0), (27.0, 10.0)], region)]
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    spawn = shapely.box(1.0, 1.0, 15.0, 13.0)
    n_agents = min(24 * scale, 240)
    positions = jps.distribute_by_number(
        polygon=spawn,
        number_of_agents=n_agents,
        distance_to_agents=0.5,
        distance_to_polygon=0.3,
        seed=7,
    )
    for pos in positions:
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=pos,
            state=jps.CollisionFreeSpeedModelV2State(radius=0.18),
            region_id=region,
        )

    t0 = time.perf_counter()
    sim.iterate(count=iterations)
    elapsed = time.perf_counter() - t0
    assert sim.iteration_count() == iterations
    return elapsed


def run_strategical_waypoint_ring(iterations: int, scale: int) -> float:
    """Stress StrategicalDecisionSystem and StageManager transitions."""
    builder = jps.GeometryBuilder()
    region = builder.add_region(
        boundary=[(0.0, 0.0), (20.0, 0.0), (20.0, 20.0), (0.0, 20.0)],
        height=0.0,
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=builder.build(),
        dt=0.01,
    )

    centers = [(5.0, 5.0), (15.0, 5.0), (15.0, 15.0), (5.0, 15.0)]
    waypoints = [
        sim.add_waypoint_stage(
            [(jps.circle_to_polygon(center=c, radius=1.5), region)]
        )
        for c in centers
    ]
    exit_id = sim.add_exit_stage(
        [([(19.0, 9.0), (20.0, 9.0), (20.0, 11.0), (19.0, 11.0)], region)]
    )

    journey = jps.JourneyDescription([*waypoints, exit_id])
    for idx, wp in enumerate(waypoints[:-1]):
        journey.set_transition_for_stage(
            wp, jps.Transition.create_fixed_transition(waypoints[idx + 1])
        )
    journey.set_transition_for_stage(
        waypoints[-1],
        jps.Transition.create_round_robin_transition(
            [(waypoints[0], 3), (exit_id, 1)]
        ),
    )
    journey_id = sim.add_journey(journey)

    spawn = shapely.box(2.0, 2.0, 12.0, 12.0)
    n_agents = min(16 * scale, 120)
    positions = jps.distribute_by_number(
        polygon=spawn,
        number_of_agents=n_agents,
        distance_to_agents=0.55,
        distance_to_polygon=0.3,
        seed=99,
    )
    for pos in positions:
        sim.add_agent(
            journey_id=journey_id,
            stage_id=waypoints[0],
            position=pos,
            state=jps.CollisionFreeSpeedModelState(radius=0.18),
            region_id=region,
        )

    t0 = time.perf_counter()
    sim.iterate(count=iterations)
    elapsed = time.perf_counter() - t0
    assert sim.iteration_count() == iterations
    return elapsed


def run_operational_model_corridor(
    model_factory: Callable[[], object],
    state_cls: type,
    dt: float,
    iterations: int,
    scale: int,
) -> float:
    """Stress an operational model in a bidirectional corridor."""
    builder = jps.GeometryBuilder()
    region = builder.add_region(
        boundary=[(0.0, 0.0), (30.0, 0.0), (30.0, 6.0), (0.0, 6.0)],
        height=0.0,
    )
    sim = jps.Simulation(model=model_factory(), geometry=builder.build(), dt=dt)

    exit_left = sim.add_exit_stage(
        [([(0.0, 0.5), (1.0, 0.5), (1.0, 5.5), (0.0, 5.5)], region)]
    )
    exit_right = sim.add_exit_stage(
        [([(29.0, 0.5), (30.0, 0.5), (30.0, 5.5), (29.0, 5.5)], region)]
    )
    j_right = sim.add_journey(jps.JourneyDescription([exit_right]))
    j_left = sim.add_journey(jps.JourneyDescription([exit_left]))

    n_per_side = min(6 * scale, 50)
    left_positions = jps.distribute_by_number(
        polygon=shapely.box(2.0, 0.6, 10.0, 5.4),
        number_of_agents=n_per_side,
        distance_to_agents=0.7,
        distance_to_polygon=0.35,
        seed=11,
    )
    right_positions = jps.distribute_by_number(
        polygon=shapely.box(20.0, 0.6, 28.0, 5.4),
        number_of_agents=n_per_side,
        distance_to_agents=0.7,
        distance_to_polygon=0.35,
        seed=22,
    )
    for pos in left_positions:
        sim.add_agent(
            journey_id=j_right,
            stage_id=exit_right,
            position=pos,
            state=_make_agent_state(state_cls, orientation=(1.0, 0.0)),
            region_id=region,
        )
    for pos in right_positions:
        sim.add_agent(
            journey_id=j_left,
            stage_id=exit_left,
            position=pos,
            state=_make_agent_state(state_cls, orientation=(-1.0, 0.0)),
            region_id=region,
        )

    t0 = time.perf_counter()
    sim.iterate(count=iterations)
    elapsed = time.perf_counter() - t0
    assert sim.iteration_count() == iterations
    return elapsed


def test_perf_routing_obstacle_hall():
    elapsed = run_routing_obstacle_hall(_perf_iterations(), _perf_scale())
    assert elapsed >= 0.0


def test_perf_dense_bottleneck_neighborhood():
    elapsed = run_dense_bottleneck_neighborhood(
        _perf_iterations(), _perf_scale()
    )
    assert elapsed >= 0.0


def test_perf_strategical_waypoint_ring():
    elapsed = run_strategical_waypoint_ring(_perf_iterations(), _perf_scale())
    assert elapsed >= 0.0


@pytest.mark.parametrize(
    "name, model_factory, state_cls, dt",
    MODEL_FACTORIES,
    ids=[entry[0] for entry in MODEL_FACTORIES],
)
def test_perf_operational_models(name, model_factory, state_cls, dt):
    elapsed = run_operational_model_corridor(
        model_factory, state_cls, dt, _perf_iterations(), _perf_scale()
    )
    assert elapsed >= 0.0


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--iterations", type=int, default=200)
    parser.add_argument("--scale", type=int, default=4)
    args = parser.parse_args()

    suites = [
        (
            "routing_obstacle_hall",
            lambda: run_routing_obstacle_hall(args.iterations, args.scale),
        ),
        (
            "dense_bottleneck_neighborhood",
            lambda: run_dense_bottleneck_neighborhood(
                args.iterations, args.scale
            ),
        ),
        (
            "strategical_waypoint_ring",
            lambda: run_strategical_waypoint_ring(args.iterations, args.scale),
        ),
    ]
    for name, factory, state_cls, dt in MODEL_FACTORIES:
        suites.append(
            (
                f"operational_{name}",
                lambda f=factory, s=state_cls, d=dt: (
                    run_operational_model_corridor(
                        f, s, d, args.iterations, args.scale
                    )
                ),
            )
        )

    print(
        f"{'Scenario':<32} {'Total (s)':>10} {'ms / step':>12} "
        f"(iterations={args.iterations}, scale={args.scale})"
    )
    for label, fn in suites:
        total = fn()
        ms_per_step = (total / args.iterations) * 1000.0
        print(f"{label:<32} {total:>10.4f} {ms_per_step:>12.3f}")


if __name__ == "__main__":
    main()
