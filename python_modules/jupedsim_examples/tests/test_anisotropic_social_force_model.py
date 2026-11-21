# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for AnisotropicSocialForceModel and walking-dyad cohesion."""

import math
from dataclasses import FrozenInstanceError

import jupedsim as jps
import pytest
import shapely
from jupedsim_examples.models.anisotropic_social_force import (
    AnisotropicSocialForceModel,
    AnisotropicSocialForceState,
)


def _make_corridor_sim(model: AnisotropicSocialForceModel, dt: float = 0.02):
    builder = jps.GeometryBuilder()
    room = builder.add_region(
        polygon=shapely.Polygon([(0, 0), (24, 0), (24, 10), (0, 10)]),
        height=0.0,
    )
    sim = jps.Simulation(model=model, geometry=builder.build(), dt=dt)
    exit_id = sim.add_exit_stage([([(23, 3), (23, 7), (24, 7), (24, 3)], room)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return sim, exit_id, journey_id, room


def test_state_is_frozen_dataclass():
    state = AnisotropicSocialForceState()
    with pytest.raises(FrozenInstanceError):
        state.desired_speed = 2.0  # type: ignore[misc]


@pytest.mark.parametrize(
    "bad_kwargs",
    [
        {"radius": 0.0},
        {"mass": -1.0},
        {"reaction_time": 0.0},
        {"desired_speed": -0.5},
        {"anisotropy_lambda": 1.5},
        {"stride_headway": -0.1},
    ],
)
def test_invalid_state_rejected_on_add_agent(bad_kwargs):
    sim, exit_id, journey_id, room = _make_corridor_sim(
        AnisotropicSocialForceModel()
    )
    with pytest.raises(Exception):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(2.0, 5.0),
            state=AnisotropicSocialForceState(**bad_kwargs),
            region_id=room,
        )


def test_single_agent_reaches_exit():
    sim, exit_id, journey_id, room = _make_corridor_sim(
        AnisotropicSocialForceModel()
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(2.0, 5.0),
        state=AnisotropicSocialForceState(),
        region_id=room,
    )
    for _ in range(1500):
        if sim.agent_count() == 0:
            break
        sim.iterate()

    assert sim.agent_count() == 0


def test_forward_neighbor_induces_stronger_slowdown_than_rear_neighbor():
    # Case A: blocker ahead of probe agent moving in +x
    sim_ahead, exit_a, j_a, room_a = _make_corridor_sim(
        AnisotropicSocialForceModel()
    )
    probe_ahead = sim_ahead.add_agent(
        journey_id=j_a,
        stage_id=exit_a,
        position=(5.0, 5.0),
        state=AnisotropicSocialForceState(
            velocity=(1.3, 0.0),
            anisotropy_lambda=0.05,
            stride_headway=0.4,
        ),
        region_id=room_a,
    )
    sim_ahead.add_agent(
        journey_id=j_a,
        stage_id=exit_a,
        position=(5.75, 5.0),
        state=AnisotropicSocialForceState(
            velocity=(0.0, 0.0), desired_speed=0.0
        ),
        region_id=room_a,
    )
    sim_ahead.iterate()
    vx_with_ahead = sim_ahead.agent(probe_ahead).state.velocity[0]

    # Case B: identical blocker at the same distance behind probe agent
    sim_behind, exit_b, j_b, room_b = _make_corridor_sim(
        AnisotropicSocialForceModel()
    )
    probe_behind = sim_behind.add_agent(
        journey_id=j_b,
        stage_id=exit_b,
        position=(5.0, 5.0),
        state=AnisotropicSocialForceState(
            velocity=(1.3, 0.0),
            anisotropy_lambda=0.05,
            stride_headway=0.4,
        ),
        region_id=room_b,
    )
    sim_behind.add_agent(
        journey_id=j_b,
        stage_id=exit_b,
        position=(4.25, 5.0),
        state=AnisotropicSocialForceState(
            velocity=(0.0, 0.0), desired_speed=0.0
        ),
        region_id=room_b,
    )
    sim_behind.iterate()
    vx_with_behind = sim_behind.agent(probe_behind).state.velocity[0]

    assert vx_with_ahead < vx_with_behind


def test_dyad_cohesion_draws_group_members_closer_than_ungrouped():
    # Grouped dyad starting 2.4 m apart laterally
    sim_grp, exit_g, j_g, room_g = _make_corridor_sim(
        AnisotropicSocialForceModel()
    )
    g1 = sim_grp.add_agent(
        journey_id=j_g,
        stage_id=exit_g,
        position=(2.0, 3.8),
        state=AnisotropicSocialForceState(group_id=1),
        region_id=room_g,
    )
    g2 = sim_grp.add_agent(
        journey_id=j_g,
        stage_id=exit_g,
        position=(2.0, 6.2),
        state=AnisotropicSocialForceState(group_id=1),
        region_id=room_g,
    )

    # Ungrouped pair at identical initial positions
    sim_ind, exit_i, j_i, room_i = _make_corridor_sim(
        AnisotropicSocialForceModel()
    )
    u1 = sim_ind.add_agent(
        journey_id=j_i,
        stage_id=exit_i,
        position=(2.0, 3.8),
        state=AnisotropicSocialForceState(group_id=None),
        region_id=room_i,
    )
    u2 = sim_ind.add_agent(
        journey_id=j_i,
        stage_id=exit_i,
        position=(2.0, 6.2),
        state=AnisotropicSocialForceState(group_id=None),
        region_id=room_i,
    )

    for _ in range(200):
        sim_grp.iterate()
        sim_ind.iterate()

    p_g1 = sim_grp.agent(g1).position
    p_g2 = sim_grp.agent(g2).position
    dist_grouped = math.hypot(p_g1[0] - p_g2[0], p_g1[1] - p_g2[1])

    p_u1 = sim_ind.agent(u1).position
    p_u2 = sim_ind.agent(u2).position
    dist_ungrouped = math.hypot(p_u1[0] - p_u2[0], p_u1[1] - p_u2[1])

    assert dist_grouped < dist_ungrouped - 0.3


def test_dyad_splits_around_pillar_and_reaches_exit():
    outer = shapely.box(0.0, 0.0, 20.0, 8.0)
    pillar = shapely.box(8.0, 3.2, 10.0, 4.8)
    walkable = outer.difference(pillar)

    builder = jps.GeometryBuilder()
    room = builder.add_region(polygon=walkable, height=0.0)
    sim = jps.Simulation(
        model=AnisotropicSocialForceModel(),
        geometry=builder.build(),
        dt=0.02,
    )
    exit_id = sim.add_exit_stage(
        [([(19.0, 2.5), (19.0, 5.5), (20.0, 5.5), (20.0, 2.5)], room)]
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(2.0, 3.5),
        state=AnisotropicSocialForceState(group_id=7),
        region_id=room,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(2.0, 4.5),
        state=AnisotropicSocialForceState(group_id=7),
        region_id=room,
    )

    for _ in range(1500):
        if sim.agent_count() == 0:
            break
        sim.iterate()

    assert sim.agent_count() == 0
