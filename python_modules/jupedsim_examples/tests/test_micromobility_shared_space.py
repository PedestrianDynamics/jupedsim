# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for KinematicBicycleModel and SharedSpaceDispatcher."""

import math
from dataclasses import FrozenInstanceError

import jupedsim as jps
import pytest
import shapely
from jupedsim_examples.models.micromobility import (
    KinematicBicycleModel,
    KinematicBicycleState,
    SharedSpaceDispatcher,
    SharedSpaceState,
    _wrap_angle,
)
from jupedsim_examples.models.pysocial_force import PythonSocialForceModelState


def _make_shared_corridor(model, dt: float = 0.02):
    builder = jps.GeometryBuilder()
    corridor = builder.add_region(
        polygon=shapely.Polygon([(0, 0), (35, 0), (35, 8), (0, 8)]),
        height=0.0,
    )
    sim = jps.Simulation(model=model, geometry=builder.build(), dt=dt)
    exit_id = sim.add_exit_stage(
        [([(34, 2), (34, 6), (35, 6), (35, 2)], corridor)]
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return sim, exit_id, journey_id, corridor


def test_bicycle_state_is_frozen():
    state = KinematicBicycleState()
    with pytest.raises(FrozenInstanceError):
        state.speed = 5.0  # type: ignore[misc]


@pytest.mark.parametrize(
    "bad_kwargs",
    [
        {"radius": 0.0},
        {"wheelbase": -1.0},
        {"max_steer_angle": 0.0},
        {"max_steer_angle": 1.7},
        {"desired_speed": -1.0},
        {"max_accel": 0.0},
        {"safe_clearance": 0.0},
    ],
)
def test_bicycle_constraint_rejects_invalid_state(bad_kwargs):
    sim, exit_id, journey_id, corridor = _make_shared_corridor(
        KinematicBicycleModel()
    )
    with pytest.raises(Exception):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(2.0, 4.0),
            state=KinematicBicycleState(**bad_kwargs),
            region_id=corridor,
        )


def test_lone_wheeler_respects_curvature_bound_and_reaches_exit():
    dt = 0.02
    sim, exit_id, journey_id, corridor = _make_shared_corridor(
        KinematicBicycleModel(), dt=dt
    )
    # Start facing 90 deg away from the +x exit to exercise bounded turning radius
    init_state = KinematicBicycleState(
        heading=0.5 * math.pi,
        speed=2.0,
        desired_speed=3.5,
        wheelbase=1.1,
        max_steer_angle=0.5,
    )
    rider = sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(3.0, 2.5),
        state=init_state,
        region_id=corridor,
    )

    prev_heading = init_state.heading
    for _ in range(1500):
        if sim.agent_count() == 0:
            break
        sim.iterate()
        if sim.agent_count() > 0:
            st = sim.agent(rider).state
            dpsi = abs(_wrap_angle(st.heading - prev_heading))
            max_dpsi = (st.speed / st.wheelbase) * math.tan(
                st.max_steer_angle
            ) * dt + 1e-6
            assert dpsi <= max_dpsi
            prev_heading = st.heading

    assert sim.agent_count() == 0


def test_wheeler_overtakes_slower_pedestrians_in_shared_space():
    dispatcher = SharedSpaceDispatcher()
    sim, exit_id, journey_id, corridor = _make_shared_corridor(
        dispatcher, dt=0.02
    )

    rider_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(2.0, 4.0),
        state=SharedSpaceState(
            kind="wheeler",
            sub=KinematicBicycleState(
                heading=0.0,
                speed=2.5,
                desired_speed=3.6,
                safe_clearance=0.9,
            ),
        ),
        region_id=corridor,
    )
    ped_sf_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(9.0, 4.0),
        state=SharedSpaceState(
            kind="social_force",
            sub=PythonSocialForceModelState(
                velocity=(1.1, 0.0), desired_speed=1.2, radius=0.25
            ),
        ),
        region_id=corridor,
    )
    ped_cfs_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(15.0, 4.1),
        state=SharedSpaceState(
            kind="collision_free",
            sub=jps.CollisionFreeSpeedModelV3State(
                desired_speed=1.2, radius=0.25
            ),
        ),
        region_id=corridor,
    )

    min_clearance = float("inf")
    max_lateral_dev = 0.0

    for _ in range(600):
        sim.iterate()
        active_ids = {a.id for a in sim.agents()}
        if rider_id not in active_ids:
            break
        rx, ry = sim.agent(rider_id).position
        max_lateral_dev = max(max_lateral_dev, abs(ry - 4.0))
        for pid in (ped_sf_id, ped_cfs_id):
            if pid in active_ids:
                px, py = sim.agent(pid).position
                min_clearance = min(min_clearance, math.hypot(rx - px, ry - py))

    # Rider deflects laterally around collinear pedestrians and avoids body collision
    assert max_lateral_dev > 0.35
    assert min_clearance > 0.65
