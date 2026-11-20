# SPDX-License-Identifier: LGPL-3.0-or-later
"""Anisotropic personal-space social forces and multimodal shared-space solver.

Implements:
- Velocity- and gaze-anisotropic elliptical personal-space
  repulsive acceleration with field-of-view attenuation and density-adaptive
  dyadic V-formation cohesion transitioning to single-file above ``1.2 ped/m^2``.
- Continuous kinematic-bicycle cyclist / e-scooter shared-space
  ``tau_CPA`` predictive avoidance solver with dynamic ``8 km/h`` speed capping
  above ``0.35 ped/m^2`` local pedestrian density.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, replace
from typing import Dict, List, Optional, Sequence, Tuple

import numpy as np
import numpy.typing as npt


def ellipse_semi_axes(
    speed_mps: float,
    distracted: float | bool = 0.0,
    a0: float = 0.42,
    tau_stride: float = 0.36,
    delta_a_distracted: float = 0.22,
    b0: float = 0.28,
    delta_b_distracted: float = 0.16,
) -> Tuple[float, float]:
    """Return longitudinal and lateral elliptical personal-space semi-axes ``(a_m, b_m)``."""
    if a0 <= 0.0 or b0 <= 0.0:
        raise ValueError(
            "Baseline semi-axes a0 and b0 must be strictly positive."
        )
    v = max(0.0, float(speed_mps))
    u = float(np.clip(float(distracted), 0.0, 1.0))
    a_m = float(a0) + float(tau_stride) * v + float(delta_a_distracted) * u
    b_m = float(b0) + float(delta_b_distracted) * u
    return a_m, b_m


def fov_attenuation(
    gaze_dir: npt.ArrayLike,
    rel_pos: npt.ArrayLike,
    lambda_aniso: float = 0.35,
    phone_distracted: bool = False,
) -> float:
    """Compute anisotropic visual field-of-view weight ``w(phi) in [lambda_aniso, 1]``.

    Follows Johansson, Helbing & Shukla (2007) with optional smartphone gaze
    aversion attenuation reducing forward visual anticipation.
    """
    if not (0.0 <= lambda_aniso <= 1.0):
        raise ValueError("lambda_aniso must lie in [0, 1].")
    g = np.asarray(gaze_dir, dtype=float).reshape(2)
    r = np.asarray(rel_pos, dtype=float).reshape(2)
    g_norm = float(np.linalg.norm(g))
    r_norm = float(np.linalg.norm(r))
    if g_norm < 1e-9 or r_norm < 1e-9:
        return 1.0

    cos_phi = float(np.clip(np.dot(g, r) / (g_norm * r_norm), -1.0, 1.0))
    w = float(lambda_aniso) + (1.0 - float(lambda_aniso)) * (
        0.5 * (1.0 + cos_phi)
    )
    if phone_distracted:
        w = float(lambda_aniso) + 0.35 * (w - float(lambda_aniso))
    return float(np.clip(w, lambda_aniso, 1.0))


def estimate_local_crowd_density(
    positions: npt.NDArray[np.float64],
    query_points: Optional[npt.NDArray[np.float64]] = None,
    kernel_radius_m: float = 1.0,
    exclude_self: bool = False,
) -> npt.NDArray[np.float64]:
    """Estimate smooth local pedestrian density (``ped / m^2``) via 2D Gaussian kernel."""
    pos = np.asarray(positions, dtype=float).reshape(-1, 2)
    pts = (
        pos
        if query_points is None
        else np.asarray(query_points, dtype=float).reshape(-1, 2)
    )
    if len(pos) == 0 or len(pts) == 0:
        return np.zeros(len(pts), dtype=float)

    diff = pts[:, np.newaxis, :] - pos[np.newaxis, :, :]
    dist_sq = np.sum(diff**2, axis=-1)
    sigma_sq = float(kernel_radius_m) ** 2
    weights = np.exp(-dist_sq / (2.0 * sigma_sq)) / (2.0 * math.pi * sigma_sq)
    if (
        exclude_self
        and query_points is None
        and weights.shape[0] == weights.shape[1]
    ):
        np.fill_diagonal(weights, 0.0)
    return np.sum(weights, axis=1)


def compute_anisotropic_social_forces(
    positions: npt.ArrayLike,
    velocities: npt.ArrayLike,
    desired_directions: npt.ArrayLike,
    desired_speeds: npt.ArrayLike | float = 1.35,
    gaze_directions: Optional[npt.ArrayLike] = None,
    distracted_mask: Optional[npt.ArrayLike] = None,
    dyads: Optional[Sequence[Tuple[int, int]]] = None,
    local_densities_m2: Optional[npt.ArrayLike] = None,
    relaxation_time_s: float = 0.50,
    repulsion_strength: float = 2.40,
    repulsion_range_ell: float = 0.45,
    lambda_aniso: float = 0.35,
    dyad_cohesion_gain: float = 1.60,
    dyad_damping_gain: float = 0.65,
    dyad_single_file_density_m2: float = 1.20,
) -> Tuple[
    npt.NDArray[np.float64],
    Dict[str, npt.NDArray[np.float64] | Dict[Tuple[int, int], str]],
]:
    """Compute velocity- and gaze-anisotropic pedestrian accelerations with
    density-adaptive dyadic V-formation / single-file transitions.

    Above ``dyad_single_file_density_m2`` (``1.2 ped/m^2``), walking dyads
    reconfigure from abreast V-formation (lateral spacing ``0.75 m``, slight
    ``0.12 m`` forward conversational V-cupping) into leader-follower single-file
    alignment (longitudinal spacing ``0.85 m``, zero lateral offset).
    """
    pos = np.asarray(positions, dtype=float).reshape(-1, 2)
    vel = np.asarray(velocities, dtype=float).reshape(-1, 2)
    e0 = np.asarray(desired_directions, dtype=float).reshape(-1, 2)
    n_ped = len(pos)
    if vel.shape != (n_ped, 2) or e0.shape != (n_ped, 2):
        raise ValueError(
            "positions, velocities, and desired_directions must share shape (N, 2)."
        )

    if np.ndim(desired_speeds) == 0:
        v0 = np.full(n_ped, float(desired_speeds), dtype=float)
    else:
        v0 = np.asarray(desired_speeds, dtype=float).reshape(n_ped)

    # Normalize desired direction vectors safely
    e0_norm = np.linalg.norm(e0, axis=1, keepdims=True)
    e0_unit = np.where(
        e0_norm > 1e-9, e0 / np.maximum(e0_norm, 1e-9), np.array([[1.0, 0.0]])
    )

    speeds = np.linalg.norm(vel, axis=1)
    heading_vecs = np.where(
        speeds[:, np.newaxis] > 1e-4,
        vel / np.maximum(speeds[:, np.newaxis], 1e-9),
        e0_unit,
    )

    if gaze_directions is None:
        gaze_vecs = heading_vecs.copy()
    else:
        g_raw = np.asarray(gaze_directions, dtype=float).reshape(n_ped, 2)
        g_norm = np.linalg.norm(g_raw, axis=1, keepdims=True)
        gaze_vecs = np.where(
            g_norm > 1e-9, g_raw / np.maximum(g_norm, 1e-9), heading_vecs
        )

    if distracted_mask is None:
        distracted = np.zeros(n_ped, dtype=float)
    else:
        distracted = np.clip(
            np.asarray(distracted_mask, dtype=float).reshape(n_ped), 0.0, 1.0
        )

    if local_densities_m2 is None:
        densities = estimate_local_crowd_density(pos)
    else:
        densities = np.asarray(local_densities_m2, dtype=float).reshape(n_ped)

    # 1. Goal-seeking relaxation term
    acc_drive = (e0_unit * v0[:, np.newaxis] - vel) / max(
        float(relaxation_time_s), 1e-3
    )

    # 2. Velocity- & gaze-anisotropic elliptical repulsive acceleration
    acc_rep = np.zeros((n_ped, 2), dtype=float)
    semi_a = np.zeros(n_ped, dtype=float)
    semi_b = np.zeros(n_ped, dtype=float)
    for i in range(n_ped):
        semi_a[i], semi_b[i] = ellipse_semi_axes(speeds[i], distracted[i])

    for i in range(n_ped):
        hx, hy = heading_vecs[i]
        nx, ny = -hy, hx
        ai, bi = semi_a[i], semi_b[i]
        is_phone = bool(distracted[i] >= 0.5)
        for j in range(n_ped):
            if i == j:
                continue
            rel = pos[j] - pos[i]
            dist = float(np.linalg.norm(rel))
            if dist < 1e-6:
                acc_rep[i] -= repulsion_strength * heading_vecs[i]
                continue

            d_par = rel[0] * hx + rel[1] * hy
            d_perp = rel[0] * nx + rel[1] * ny
            d_eff = math.hypot(d_par / ai, d_perp / bi)

            # Gradient of d_eff w.r.t. pos[i] points away from neighbor j
            grad_local_par = (d_par / (ai**2)) / max(d_eff, 1e-6)
            grad_local_perp = (d_perp / (bi**2)) / max(d_eff, 1e-6)
            away_vec = -np.array(
                [
                    grad_local_par * hx + grad_local_perp * nx,
                    grad_local_par * hy + grad_local_perp * ny,
                ]
            )
            away_norm = float(np.linalg.norm(away_vec))
            away_unit = away_vec / max(away_norm, 1e-9)

            w_fov = fov_attenuation(
                gaze_vecs[i],
                rel,
                lambda_aniso=lambda_aniso,
                phone_distracted=is_phone,
            )
            mag = (
                float(repulsion_strength)
                * w_fov
                * math.exp(
                    (1.0 - d_eff) / max(float(repulsion_range_ell), 1e-3)
                )
            )
            acc_rep[i] += mag * away_unit

    # 3. Dyadic V-formation social cohesion -> single-file transition above 1.2 ped/m^2
    acc_dyad = np.zeros((n_ped, 2), dtype=float)
    dyad_formations: Dict[Tuple[int, int], str] = {}
    if dyads:
        for i, j in dyads:
            u_grp = 0.5 * (e0_unit[i] + e0_unit[j])
            u_norm = float(np.linalg.norm(u_grp))
            u_hat = u_grp / u_norm if u_norm > 1e-6 else e0_unit[i]
            n_perp = np.array([-u_hat[1], u_hat[0]])

            rho_pair = 0.5 * (densities[i] + densities[j])
            # Smooth transition centered at 1.2 ped/m^2
            alpha_sf = 1.0 / (
                1.0
                + math.exp(
                    -10.0 * (rho_pair - float(dyad_single_file_density_m2))
                )
            )
            dyad_formations[(i, j)] = (
                "single_file"
                if rho_pair > float(dyad_single_file_density_m2)
                else "v_abreast"
            )

            # Determine consistent lateral side / leader sign from current geometry
            rel_ij = pos[j] - pos[i]
            lat_sign = 1.0 if float(np.dot(rel_ij, n_perp)) >= 0.0 else -1.0
            long_sign = 1.0 if float(np.dot(rel_ij, u_hat)) >= 0.0 else -1.0

            target_v_abreast = -0.12 * u_hat + (0.75 * lat_sign) * n_perp
            target_single_file = (0.85 * long_sign) * u_hat
            target_rel = (
                1.0 - alpha_sf
            ) * target_v_abreast + alpha_sf * target_single_file

            pos_err = rel_ij - target_rel
            vel_err = vel[j] - vel[i]
            f_cohesion = (
                float(dyad_cohesion_gain) * pos_err
                + float(dyad_damping_gain) * vel_err
            )
            acc_dyad[i] += f_cohesion
            acc_dyad[j] -= f_cohesion

    total_acc = acc_drive + acc_rep + acc_dyad
    diagnostics: Dict[
        str, npt.NDArray[np.float64] | Dict[Tuple[int, int], str]
    ] = {
        "semi_a_m": semi_a,
        "semi_b_m": semi_b,
        "local_densities_m2": densities,
        "acc_drive": acc_drive,
        "acc_repulsion": acc_rep,
        "acc_dyad": acc_dyad,
        "dyad_formations": dyad_formations,
    }
    return total_acc, diagnostics


@dataclass(frozen=True)
class MicroMobilityAgent:
    """Continuous kinematic-bicycle state for a shared-space cyclist or e-scooter."""

    agent_id: str
    vehicle_type: str = "cyclist"
    x: float = 0.0
    y: float = 0.0
    yaw: float = 0.0
    speed: float = 3.5
    wheelbase_m: float = 1.05
    desired_speed_mps: float = 4.5
    goal_xy: Tuple[float, float] = (30.0, 0.0)
    max_steer_rad: float = 0.52
    max_accel_mps2: float = 2.0
    max_decel_mps2: float = 3.8

    @property
    def velocity(self) -> npt.NDArray[np.float64]:
        return np.array(
            [self.speed * math.cos(self.yaw), self.speed * math.sin(self.yaw)],
            dtype=float,
        )

    def step_kinematic_bicycle(
        self,
        accel_mps2: float,
        steer_rad: float,
        dt: float,
        speed_cap_mps: Optional[float] = None,
    ) -> MicroMobilityAgent:
        """Advance immutable state by ``dt`` under exact kinematic-bicycle kinematics."""
        delta = float(
            np.clip(steer_rad, -self.max_steer_rad, self.max_steer_rad)
        )
        acc = float(
            np.clip(accel_mps2, -self.max_decel_mps2, self.max_accel_mps2)
        )
        cap = (
            float(speed_cap_mps)
            if speed_cap_mps is not None
            else max(self.desired_speed_mps * 1.2, self.speed)
        )

        beta = math.atan(0.5 * math.tan(delta))
        v_next = float(np.clip(self.speed + acc * dt, 0.0, cap))
        v_mid = 0.5 * (self.speed + v_next)

        x_next = self.x + v_mid * math.cos(self.yaw + beta) * dt
        y_next = self.y + v_mid * math.sin(self.yaw + beta) * dt
        yaw_rate = (
            (v_mid / max(self.wheelbase_m, 0.3))
            * math.cos(beta)
            * math.tan(delta)
        )
        yaw_next = math.atan2(
            math.sin(self.yaw + yaw_rate * dt),
            math.cos(self.yaw + yaw_rate * dt),
        )
        return replace(self, x=x_next, y=y_next, yaw=yaw_next, speed=v_next)


class SharedSpaceMultimodalSolver:
    """Coupled pedestrian + cyclist/e-scooter shared-space solver .

    Enforces a dynamic ``8.0 km/h`` (``2.222 m/s``) micro-mobility speed cap
    whenever local pedestrian crowd density exceeds ``0.35 ped/m^2`` and selects
    predictive ``tau_CPA`` / ``d_CPA`` collision-avoidance steering & braking.
    """

    def __init__(
        self,
        crowd_density_threshold_m2: float = 0.35,
        dense_speed_cap_kmh: float = 8.0,
        safe_passing_clearance_m: float = 1.25,
        cpa_horizon_s: float = 3.5,
        n_steer_candidates: int = 11,
    ) -> None:
        self.crowd_density_threshold_m2 = float(crowd_density_threshold_m2)
        self.dense_speed_cap_kmh = float(dense_speed_cap_kmh)
        self.dense_speed_cap_mps = float(dense_speed_cap_kmh) / 3.6
        self.safe_passing_clearance_m = float(safe_passing_clearance_m)
        self.cpa_horizon_s = float(cpa_horizon_s)
        self.n_steer_candidates = max(3, int(n_steer_candidates))

    def compute_effective_speed_cap(
        self, local_density_m2: float, free_speed_mps: float
    ) -> float:
        """Apply ``8 km/h`` speed cap when ``local_density_m2 > 0.35 ped/m^2``."""
        if float(local_density_m2) > self.crowd_density_threshold_m2:
            return min(float(free_speed_mps), self.dense_speed_cap_mps)
        return float(free_speed_mps)

    @staticmethod
    def compute_pair_cpa(
        p_pos: npt.NDArray[np.float64],
        p_vel: npt.NDArray[np.float64],
        w_pos: npt.NDArray[np.float64],
        w_vel: npt.NDArray[np.float64],
    ) -> Tuple[float, float]:
        """Return ``(tau_cpa_s, d_cpa_m)`` between pedestrian and wheeler."""
        dr = w_pos - p_pos
        dv = w_vel - p_vel
        dv_sq = float(np.dot(dv, dv))
        if dv_sq < 1e-12:
            return 0.0, float(np.linalg.norm(dr))
        tau = max(0.0, -float(np.dot(dr, dv)) / dv_sq)
        d_cpa = float(np.linalg.norm(dr + tau * dv))
        return tau, d_cpa

    def select_wheeler_control(
        self,
        agent: MicroMobilityAgent,
        ped_positions: npt.NDArray[np.float64],
        ped_velocities: npt.NDArray[np.float64],
        dt: float,
        local_density_m2: Optional[float] = None,
    ) -> Tuple[float, float, float]:
        """Return ``(accel_mps2, steer_rad, speed_cap_mps)`` minimizing goal
        heading error plus ``tau_CPA`` pedestrian encroachment risk.
        """
        w_pos = np.array([[agent.x, agent.y]], dtype=float)
        if local_density_m2 is None:
            rho = float(
                estimate_local_crowd_density(ped_positions, query_points=w_pos)[
                    0
                ]
            )
        else:
            rho = float(local_density_m2)

        speed_cap = self.compute_effective_speed_cap(
            rho, agent.desired_speed_mps
        )

        to_goal = np.array(agent.goal_xy, dtype=float) - np.array(
            [agent.x, agent.y]
        )
        goal_yaw = math.atan2(to_goal[1], to_goal[0])

        steer_grid = np.linspace(
            -agent.max_steer_rad, agent.max_steer_rad, self.n_steer_candidates
        )
        nom_acc = float(
            np.clip(
                (speed_cap - agent.speed) / max(dt, 0.25),
                -agent.max_decel_mps2,
                agent.max_accel_mps2,
            )
        )
        acc_candidates = [
            nom_acc,
            0.0,
            -0.6 * agent.max_decel_mps2,
            -agent.max_decel_mps2,
        ]

        best_cost = math.inf
        best_acc = nom_acc
        best_steer = 0.0

        for steer in steer_grid:
            for acc in acc_candidates:
                cand = agent.step_kinematic_bicycle(
                    accel_mps2=acc,
                    steer_rad=float(steer),
                    dt=dt,
                    speed_cap_mps=speed_cap,
                )
                yaw_err = math.atan2(
                    math.sin(goal_yaw - cand.yaw), math.cos(goal_yaw - cand.yaw)
                )
                cost = (
                    2.2 * (yaw_err**2)
                    + 0.35 * ((speed_cap - cand.speed) ** 2)
                    + 0.15 * (steer**2)
                )

                cand_pos = np.array([cand.x, cand.y], dtype=float)
                cand_vel = cand.velocity
                for p_idx in range(len(ped_positions)):
                    tau_cpa, d_cpa = self.compute_pair_cpa(
                        ped_positions[p_idx],
                        ped_velocities[p_idx],
                        cand_pos,
                        cand_vel,
                    )
                    if (
                        tau_cpa <= self.cpa_horizon_s
                        and d_cpa < self.safe_passing_clearance_m
                    ):
                        urgency = 1.0 / (0.25 + tau_cpa)
                        intrusion = self.safe_passing_clearance_m - d_cpa
                        cost += 28.0 * urgency * (intrusion**2)

                if cost < best_cost:
                    best_cost = cost
                    best_acc = float(acc)
                    best_steer = float(steer)

        return best_acc, best_steer, speed_cap

    def step(
        self,
        ped_positions: npt.ArrayLike,
        ped_velocities: npt.ArrayLike,
        ped_desired_dirs: npt.ArrayLike,
        wheelers: Sequence[MicroMobilityAgent],
        dt: float = 0.05,
        ped_desired_speeds: npt.ArrayLike | float = 1.35,
        distracted_mask: Optional[npt.ArrayLike] = None,
        dyads: Optional[Sequence[Tuple[int, int]]] = None,
    ) -> Tuple[
        npt.NDArray[np.float64],
        npt.NDArray[np.float64],
        List[MicroMobilityAgent],
        Dict[str, object],
    ]:
        """Advance coupled pedestrians and micro-mobility wheelers by ``dt``."""
        p_pos = np.asarray(ped_positions, dtype=float).reshape(-1, 2)
        p_vel = np.asarray(ped_velocities, dtype=float).reshape(-1, 2)

        ped_acc, diag = compute_anisotropic_social_forces(
            positions=p_pos,
            velocities=p_vel,
            desired_directions=ped_desired_dirs,
            desired_speeds=ped_desired_speeds,
            distracted_mask=distracted_mask,
            dyads=dyads,
        )

        # Add pedestrian evasive repulsion from approaching micro-mobility agents
        for whl in wheelers:
            w_xy = np.array([whl.x, whl.y], dtype=float)
            for i in range(len(p_pos)):
                away = p_pos[i] - w_xy
                d = float(np.linalg.norm(away))
                if 1e-6 < d < 3.5:
                    ped_acc[i] += 2.2 * math.exp((0.9 - d) / 0.65) * (away / d)

        next_wheelers: List[MicroMobilityAgent] = []
        speed_caps: List[float] = []
        min_d_cpa = math.inf

        for whl in wheelers:
            acc, steer, cap = self.select_wheeler_control(
                whl, p_pos, p_vel, dt=dt
            )
            nxt = whl.step_kinematic_bicycle(
                accel_mps2=acc, steer_rad=steer, dt=dt, speed_cap_mps=cap
            )
            next_wheelers.append(nxt)
            speed_caps.append(cap)
            nxt_pos = np.array([nxt.x, nxt.y], dtype=float)
            for i in range(len(p_pos)):
                _, dcpa = self.compute_pair_cpa(
                    p_pos[i], p_vel[i], nxt_pos, nxt.velocity
                )
                min_d_cpa = min(min_d_cpa, dcpa)

        next_p_vel = p_vel + ped_acc * dt
        # Cap pedestrian speeds softly at 2.4 m/s
        p_spds = np.linalg.norm(next_p_vel, axis=1, keepdims=True)
        next_p_vel = np.where(
            p_spds > 2.4,
            next_p_vel * (2.4 / np.maximum(p_spds, 1e-9)),
            next_p_vel,
        )
        next_p_pos = p_pos + 0.5 * (p_vel + next_p_vel) * dt

        step_info: Dict[str, object] = {
            "ped_diagnostics": diag,
            "wheeler_speed_caps_mps": np.asarray(speed_caps, dtype=float),
            "min_d_cpa_m": min_d_cpa,
        }
        return next_p_pos, next_p_vel, next_wheelers, step_info
