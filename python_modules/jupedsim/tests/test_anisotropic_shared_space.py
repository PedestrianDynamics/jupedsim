# SPDX-License-Identifier: LGPL-3.0-or-later
"""Unit tests for anisotropic personal space and shared-space micro-mobility."""

from __future__ import annotations

import numpy as np
from jupedsim.routing.anisotropic_shared_space import (
    MicroMobilityAgent,
    SharedSpaceMultimodalSolver,
    compute_anisotropic_social_forces,
    ellipse_semi_axes,
    fov_attenuation,
)


def test_ellipse_semi_axes_speed_and_attention() -> None:
    a_attentive, b_attentive = ellipse_semi_axes(speed_mps=1.3, distracted=0.0)
    a_distracted, b_distracted = ellipse_semi_axes(
        speed_mps=1.3, distracted=1.0
    )
    assert a_attentive > b_attentive
    assert a_distracted > a_attentive
    assert b_distracted > b_attentive


def test_fov_attenuation_cone_and_phone_range() -> None:
    w_front = fov_attenuation([1.0, 0.0], [2.0, 0.0], phone_distracted=False)
    w_rear = fov_attenuation([1.0, 0.0], [-2.0, 0.0], phone_distracted=False)
    w_phone = fov_attenuation([1.0, 0.0], [2.0, 0.0], phone_distracted=True)
    assert w_front == 1.0
    assert w_rear < 0.40
    assert w_phone < w_front


def test_anisotropic_forces_and_dyad_single_file_transition() -> None:
    pos = np.array([[0.0, 0.0], [0.0, 0.8]], dtype=float)
    vel = np.array([[1.2, 0.0], [1.2, 0.0]], dtype=float)
    dirs = np.array([[1.0, 0.0], [1.0, 0.0]], dtype=float)

    f_low, diag_low = compute_anisotropic_social_forces(
        positions=pos,
        velocities=vel,
        desired_directions=dirs,
        dyads=[(0, 1)],
        local_densities_m2=np.array([0.3, 0.3]),
    )
    f_high, diag_high = compute_anisotropic_social_forces(
        positions=pos,
        velocities=vel,
        desired_directions=dirs,
        dyads=[(0, 1)],
        local_densities_m2=np.array([1.6, 1.6]),
    )
    assert f_low.shape == (2, 2)
    assert not np.allclose(f_low, f_high)
    assert diag_low["dyad_formations"][(0, 1)] == "v_abreast"
    assert diag_high["dyad_formations"][(0, 1)] == "single_file"


def test_shared_space_dynamic_speed_cap_and_cpa() -> None:
    solver = SharedSpaceMultimodalSolver(
        crowd_density_threshold_m2=0.35,
        dense_speed_cap_kmh=8.0,
    )
    assert solver.compute_effective_speed_cap(
        0.15, 5.0
    ) > solver.compute_effective_speed_cap(0.55, 5.0)

    wheelers = [
        MicroMobilityAgent(
            agent_id="cyc_1",
            vehicle_type="cyclist",
            x=-6.0,
            y=0.0,
            yaw=0.0,
            speed=4.5,
            desired_speed_mps=5.0,
            goal_xy=(12.0, 0.0),
        )
    ]
    _, _, next_w, info = solver.step(
        ped_positions=np.array([[0.0, 0.2], [0.5, -0.3]], dtype=float),
        ped_velocities=np.array([[0.0, 1.0], [0.0, 1.0]], dtype=float),
        ped_desired_dirs=np.array([[0.0, 1.0], [0.0, 1.0]], dtype=float),
        wheelers=wheelers,
        dt=0.05,
    )
    assert len(next_w) == 1
    assert "min_d_cpa_m" in info
