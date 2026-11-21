# SPDX-License-Identifier: LGPL-3.0-or-later
"""Anisotropic social-force model with optional walking-dyad cohesion.

Combines:
1. Helbing & Molnár (1995) social and obstacle repulsion forces,
2. Johansson, Helbing & Shukla (2007) forward field-of-view anisotropy and
   speed-dependent longitudinal stride personal space,
3. Moussaïd et al. (2010) walking-group centroid attraction and abreast
   alignment for agents sharing a ``group_id``.
"""

import math
from dataclasses import dataclass, replace

from jupedsim import CustomOperationalModel, WallView


@dataclass(kw_only=True, frozen=True)
class AnisotropicSocialForceState:
    """Immutable per-agent state for :class:`AnisotropicSocialForceModel`."""

    velocity: tuple[float, float] = (0.0, 0.0)
    desired_speed: float = 1.34
    reaction_time: float = 0.5
    agent_scale: float = 2000.0
    obstacle_scale: float = 2000.0
    force_distance: float = 0.08
    mass: float = 80.0
    body_force: float = 120000.0
    friction: float = 240000.0
    radius: float = 0.25
    anisotropy_lambda: float = 0.2
    stride_headway: float = 0.25
    group_id: int | None = None
    cohesion_strength: float = 2.5
    abreast_spacing: float = 0.85
    abreast_strength: float = 1.2


class AnisotropicSocialForceModel(CustomOperationalModel):
    """Anisotropic social-force operational model with walking-dyad cohesion.

    References:
        - Helbing, D., & Molnár, P. (1995). Social force model for pedestrian
          dynamics. *Physical Review E*, 51(5), 4282.
        - Johansson, A., Helbing, D., & Shukla, P. K. (2007). Specification of
          the social force pedestrian model by evolutionary adjustment to video
          tracking data. *Advances in Complex Systems*, 10(supp02), 271-288.
        - Moussaïd, M., Perozo, N., Garnier, S., Helbing, D., & Theraulaz, G.
          (2010). The walking behaviour of pedestrian social groups and its
          impact on crowd dynamics. *PLOS ONE*, 5(4), e10047.
    """

    def __init__(
        self,
        *,
        neighbor_range: float = 4.0,
        wall_range: float = 5.0,
    ) -> None:
        super().__init__()
        if neighbor_range <= 0.0 or wall_range <= 0.0:
            raise ValueError(
                "neighbor_range and wall_range must be strictly positive."
            )
        self._neighbor_range = neighbor_range
        self._wall_range = wall_range

    def check_model_constraint(self, state, view) -> None:
        if state.radius <= 0.0:
            raise ValueError(f"radius must be > 0, got {state.radius}.")
        if state.mass <= 0.0:
            raise ValueError(f"mass must be > 0, got {state.mass}.")
        if state.reaction_time <= 0.0:
            raise ValueError(
                f"reaction_time must be > 0, got {state.reaction_time}."
            )
        if state.desired_speed < 0.0:
            raise ValueError(
                f"desired_speed must be >= 0, got {state.desired_speed}."
            )
        if not 0.0 <= state.anisotropy_lambda <= 1.0:
            raise ValueError(
                "anisotropy_lambda must lie in [0, 1], "
                f"got {state.anisotropy_lambda}."
            )
        if state.stride_headway < 0.0:
            raise ValueError(
                f"stride_headway must be >= 0, got {state.stride_headway}."
            )

    @staticmethod
    def _heading_unit(
        velocity: tuple[float, float],
        route_orientation: tuple[float, float],
    ) -> tuple[float, float]:
        speed = math.hypot(velocity[0], velocity[1])
        if speed > 1e-3:
            return (velocity[0] / speed, velocity[1] / speed)
        rn = math.hypot(route_orientation[0], route_orientation[1])
        if rn > 1e-6:
            return (route_orientation[0] / rn, route_orientation[1] / rn)
        return (1.0, 0.0)

    @staticmethod
    def _desired_force(
        *,
        velocity: tuple[float, float],
        target_direction: tuple[float, float],
        desired_speed: float,
        reaction_time: float,
    ) -> tuple[float, float]:
        vx_des = desired_speed * target_direction[0]
        vy_des = desired_speed * target_direction[1]
        return (
            (vx_des - velocity[0]) / reaction_time,
            (vy_des - velocity[1]) / reaction_time,
        )

    @staticmethod
    def _anisotropic_social_force(
        state: AnisotropicSocialForceState,
        heading: tuple[float, float],
        neighbor,
    ) -> tuple[float, float]:
        # rel points from querying agent to neighbor; push_vec points neighbor -> agent
        rel_x, rel_y = neighbor.relative_position
        dist = math.hypot(rel_x, rel_y)
        if dist < 1e-4:
            return (0.0, 0.0)

        # Unit vector pointing from neighbor toward querying agent
        n_x = -rel_x / dist
        n_y = -rel_y / dist

        # Forward angle cos(phi) between agent's heading and vector toward neighbor
        cos_phi = (heading[0] * rel_x + heading[1] * rel_y) / dist
        fov_weight = state.anisotropy_lambda + (
            1.0 - state.anisotropy_lambda
        ) * (0.5 * (1.0 + cos_phi))

        # Heading-aligned directional decay range B(phi): extended ahead by stride_headway * speed
        speed = math.hypot(state.velocity[0], state.velocity[1])
        forward_proj = max(0.0, cos_phi)
        effective_range = max(
            1e-3,
            state.force_distance + state.stride_headway * speed * forward_proj,
        )

        min_body_dist = state.radius + neighbor.state.radius
        exp_arg = max(
            -50.0,
            min(10.0, -(dist - min_body_dist) / effective_range),
        )
        f_psych = fov_weight * state.agent_scale * math.exp(exp_arg)

        # Isotropic physical contact and tangential friction on body overlap
        f_body = 0.0
        f_tangential = 0.0
        t_x = -n_y
        t_y = n_x
        if dist < min_body_dist:
            overlap = min_body_dist - dist
            f_body = state.body_force * overlap
            dvx = neighbor.state.velocity[0] - state.velocity[0]
            dvy = neighbor.state.velocity[1] - state.velocity[1]
            dv_t = dvx * t_x + dvy * t_y
            f_tangential = state.friction * overlap * dv_t

        fx = (f_psych + f_body) * n_x + f_tangential * t_x
        fy = (f_psych + f_body) * n_y + f_tangential * t_y
        return (fx, fy)

    @staticmethod
    def _group_cohesion_acceleration(
        state: AnisotropicSocialForceState,
        route_orientation: tuple[float, float],
        group_offsets: list[tuple[float, float]],
    ) -> tuple[float, float]:
        if not group_offsets:
            return (0.0, 0.0)

        # Centroid offset relative to querying agent
        inv_n = 1.0 / len(group_offsets)
        cx = sum(p[0] for p in group_offsets) * inv_n
        cy = sum(p[1] for p in group_offsets) * inv_n
        c_dist = math.hypot(cx, cy)

        acc_x = 0.0
        acc_y = 0.0

        # Moussaid et al. (2010): bounded attraction toward group centroid when beyond spacing
        half_target = 0.5 * state.abreast_spacing
        if c_dist > half_target + 1e-4:
            pull = state.cohesion_strength * math.tanh(
                (c_dist - half_target) / max(state.abreast_spacing, 0.2)
            )
            acc_x += pull * (cx / c_dist)
            acc_y += pull * (cy / c_dist)

        # Gentle longitudinal alignment penalty so dyad members walk abreast
        rn = math.hypot(route_orientation[0], route_orientation[1])
        if rn > 1e-6:
            rx = route_orientation[0] / rn
            ry = route_orientation[1] / rn
            long_lag = cx * rx + cy * ry
            acc_x += state.abreast_strength * math.tanh(long_lag) * rx
            acc_y += state.abreast_strength * math.tanh(long_lag) * ry

        return (acc_x, acc_y)

    @staticmethod
    def _obstacle_force(
        state: AnisotropicSocialForceState,
        wall: WallView,
    ) -> tuple[float, float]:
        dist = wall.distance
        if dist < 1e-4:
            return (0.0, 0.0)

        n_x, n_y = wall.normal
        exp_arg = max(
            -50.0, min(20.0, -(dist - state.radius) / state.force_distance)
        )
        f_n = state.obstacle_scale * math.exp(exp_arg)
        if dist < state.radius:
            f_n += state.body_force * (state.radius - dist)
        return (f_n * n_x, f_n * n_y)

    def compute_next_state(self, state: AnisotropicSocialForceState, step):
        target_dir = step.route_orientation
        heading = self._heading_unit(state.velocity, target_dir)

        fx_total, fy_total = self._desired_force(
            velocity=state.velocity,
            target_direction=target_dir,
            desired_speed=state.desired_speed,
            reaction_time=state.reaction_time,
        )
        acc_x = fx_total
        acc_y = fy_total

        visible_neighbors = step.other_agents_in_range(
            self._neighbor_range,
            lambda n: step.no_geometry_between(n),
        )
        group_offsets: list[tuple[float, float]] = []

        for neighbor in visible_neighbors:
            fx, fy = self._anisotropic_social_force(state, heading, neighbor)
            acc_x += fx / state.mass
            acc_y += fy / state.mass
            if (
                state.group_id is not None
                and getattr(neighbor.state, "group_id", None) == state.group_id
            ):
                group_offsets.append(neighbor.relative_position)

        if group_offsets:
            g_ax, g_ay = self._group_cohesion_acceleration(
                state, target_dir, group_offsets
            )
            acc_x += g_ax
            acc_y += g_ay

        for wall in step.walls_in_range(self._wall_range):
            w_fx, w_fy = self._obstacle_force(state, wall)
            acc_x += w_fx / state.mass
            acc_y += w_fy / state.mass

        raw_vx = state.velocity[0] + acc_x * step.dt
        raw_vy = state.velocity[1] + acc_y * step.dt
        speed = math.hypot(raw_vx, raw_vy)
        max_speed = max(2.0 * state.desired_speed, 3.0)
        if speed > max_speed:
            scale = max_speed / speed
            raw_vx *= scale
            raw_vy *= scale

        new_velocity = (raw_vx, raw_vy)
        movement = (new_velocity[0] * step.dt, new_velocity[1] * step.dt)
        return replace(state, velocity=new_velocity), movement
