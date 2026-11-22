# SPDX-License-Identifier: LGPL-3.0-or-later
"""Kinematic bicycle micromobility model and shared-space multi-model dispatcher.

Demonstrates how non-holonomic micromobility agents (cyclists, e-scooters) can
share walkable space with pedestrians inside a single ``jupedsim.Simulation``:

1. :class:`KinematicBicycleModel` advances riders under kinematic bicycle
   curvature limits (Rajamani, 2012) while evaluating predictive
   Closest-Point-of-Approach (:math:`\\tau_{\\mathrm{CPA}}, d_{\\mathrm{CPA}}`)
   lateral passing and longitudinal speed yielding over visible neighbors.
2. :class:`SharedSpaceDispatcher` composes :class:`KinematicBicycleModel` with
   pedestrian sub-models (:class:`PythonSocialForceModel` and built-in
   :class:`jupedsim.CollisionFreeSpeedModelV3`) via
   :meth:`jupedsim.AgentStep.with_neighbor_state_mapping`.
"""

import math
from dataclasses import dataclass, replace

import jupedsim as jps
from jupedsim import CustomOperationalModel, WallView

from jupedsim_examples.models.pysocial_force import (
    PythonSocialForceModel,
    PythonSocialForceModelState,
)


def _wrap_angle(angle: float) -> float:
    """Wrap angle to ``[-pi, pi)``."""
    return (angle + math.pi) % (2.0 * math.pi) - math.pi


@dataclass(kw_only=True, frozen=True)
class PerceivedNeighborState:
    """Minimal read-only neighbor view consumed by :class:`KinematicBicycleModel`."""

    velocity: tuple[float, float] = (0.0, 0.0)
    radius: float = 0.25


@dataclass(kw_only=True, frozen=True)
class KinematicBicycleState:
    """Immutable per-agent state for :class:`KinematicBicycleModel`."""

    heading: float = 0.0
    speed: float = 0.0
    desired_speed: float = 3.5
    wheelbase: float = 1.05
    max_steer_angle: float = 0.55
    steer_gain: float = 2.0
    speed_relaxation_time: float = 0.6
    max_accel: float = 1.8
    max_decel: float = 3.5
    safe_clearance: float = 0.85
    anticipation_horizon: float = 3.5
    radius: float = 0.35

    @property
    def velocity(self) -> tuple[float, float]:
        return (
            self.speed * math.cos(self.heading),
            self.speed * math.sin(self.heading),
        )


class KinematicBicycleModel(CustomOperationalModel):
    """Non-holonomic kinematic bicycle model with CPA shared-space anticipation."""

    def __init__(
        self,
        *,
        neighbor_range: float = 10.0,
        wall_range: float = 3.0,
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
        if state.wheelbase <= 0.0:
            raise ValueError(f"wheelbase must be > 0, got {state.wheelbase}.")
        if not 0.0 < state.max_steer_angle < 0.5 * math.pi:
            raise ValueError(
                "max_steer_angle must lie in (0, pi/2), "
                f"got {state.max_steer_angle}."
            )
        if state.desired_speed < 0.0:
            raise ValueError(
                f"desired_speed must be >= 0, got {state.desired_speed}."
            )
        if state.max_accel <= 0.0 or state.max_decel <= 0.0:
            raise ValueError("max_accel and max_decel must be > 0.")
        if state.safe_clearance <= 0.0 or state.anticipation_horizon <= 0.0:
            raise ValueError(
                "safe_clearance and anticipation_horizon must be > 0."
            )

    @staticmethod
    def _evaluate_cpa_avoidance(
        state: KinematicBicycleState,
        desired_dir: tuple[float, float],
        neighbors: list,
    ) -> tuple[tuple[float, float], float]:
        """Compute CPA-adjusted steering vector and speed scale in ``[0, 1]``."""
        steer_x, steer_y = desired_dir
        fwd_x = math.cos(state.heading)
        fwd_y = math.sin(state.heading)

        # Use nominal travel speed when starting from rest so static blockers ahead are anticipated
        probe_speed = max(state.speed, 0.5 * state.desired_speed)
        vw_x = probe_speed * fwd_x
        vw_y = probe_speed * fwd_y

        speed_scale = 1.0

        for neighbor in neighbors:
            dr_x, dr_y = neighbor.relative_position
            dist = math.hypot(dr_x, dr_y)
            if dist < 1e-4:
                continue

            # Ignore neighbors well behind the rider
            if dr_x * fwd_x + dr_y * fwd_y < -state.radius:
                continue

            nv_x, nv_y = getattr(neighbor.state, "velocity", (0.0, 0.0))
            n_radius = float(getattr(neighbor.state, "radius", 0.25))

            dv_x = nv_x - vw_x
            dv_y = nv_y - vw_y
            dv_sq = dv_x * dv_x + dv_y * dv_y

            if dv_sq > 1e-8:
                tau_raw = -(dr_x * dv_x + dr_y * dv_y) / dv_sq
                tau_cpa = max(0.0, min(state.anticipation_horizon, tau_raw))
            else:
                tau_cpa = 0.0

            cpa_x = dr_x + tau_cpa * dv_x
            cpa_y = dr_y + tau_cpa * dv_y
            d_cpa = math.hypot(cpa_x, cpa_y)

            req_clearance = state.safe_clearance + state.radius + n_radius
            if d_cpa >= req_clearance:
                continue

            intrusion = (req_clearance - d_cpa) / req_clearance
            time_weight = 1.0 - (tau_cpa / state.anticipation_horizon) * 0.65
            urgency = intrusion * time_weight

            # Choose lateral passing direction orthogonal to current heading
            # cross > 0 => neighbor is to the left => pass on the right (-lat);
            # cross <= 0 => neighbor is to the right or dead-ahead => pass on the left (+lat).
            cross = fwd_x * cpa_y - fwd_y * cpa_x
            pass_sign = -1.0 if cross > 1e-3 else 1.0
            lat_x = -fwd_y * pass_sign
            lat_y = fwd_x * pass_sign

            steer_x += 2.2 * urgency * lat_x
            steer_y += 2.2 * urgency * lat_y

            # Yield longitudinally when close and nearly dead-ahead
            forward_align = max(0.0, (dr_x * fwd_x + dr_y * fwd_y) / dist)
            if dist < 2.0 * req_clearance and forward_align > 0.75:
                local_scale = max(0.15, 1.0 - 0.85 * intrusion * forward_align)
                speed_scale = min(speed_scale, local_scale)

        return (steer_x, steer_y), speed_scale

    @staticmethod
    def _apply_wall_avoidance(
        state: KinematicBicycleState,
        steer_vec: tuple[float, float],
        walls: list[WallView],
    ) -> tuple[tuple[float, float], float]:
        sx, sy = steer_vec
        wall_speed_scale = 1.0
        buffer_dist = state.radius + 0.55

        for wall in walls:
            if wall.distance <= 1e-4 or wall.distance >= buffer_dist:
                continue
            w = (buffer_dist - wall.distance) / buffer_dist
            sx += 2.5 * w * wall.normal[0]
            sy += 2.5 * w * wall.normal[1]
            if wall.distance < state.radius + 0.15:
                wall_speed_scale = min(wall_speed_scale, 0.4)

        return (sx, sy), wall_speed_scale

    def compute_next_state(self, state: KinematicBicycleState, step):
        route_dir = step.route_orientation
        rn = math.hypot(route_dir[0], route_dir[1])
        if rn > 1e-6:
            desired_dir = (route_dir[0] / rn, route_dir[1] / rn)
        else:
            desired_dir = (math.cos(state.heading), math.sin(state.heading))

        visible_neighbors = step.other_agents_in_range(
            self._neighbor_range,
            lambda n: step.no_geometry_between(n),
        )
        steer_vec, cpa_speed_scale = self._evaluate_cpa_avoidance(
            state, desired_dir, visible_neighbors
        )
        steer_vec, wall_speed_scale = self._apply_wall_avoidance(
            state, steer_vec, step.walls_in_range(self._wall_range)
        )

        sn = math.hypot(steer_vec[0], steer_vec[1])
        if sn > 1e-6:
            desired_heading = math.atan2(steer_vec[1], steer_vec[0])
        else:
            desired_heading = state.heading

        heading_err = _wrap_angle(desired_heading - state.heading)
        steer_angle = max(
            -state.max_steer_angle,
            min(state.max_steer_angle, state.steer_gain * heading_err),
        )

        # Slow slightly during sharp turns in addition to CPA / wall yielding
        turn_scale = max(
            0.35, 1.0 - 0.5 * abs(steer_angle) / state.max_steer_angle
        )
        target_speed = (
            state.desired_speed
            * min(cpa_speed_scale, wall_speed_scale)
            * turn_scale
        )

        raw_acc = (target_speed - state.speed) / max(
            state.speed_relaxation_time, 1e-3
        )
        acc = max(-state.max_decel, min(state.max_accel, raw_acc))
        new_speed = max(0.0, state.speed + acc * step.dt)

        yaw_rate = (new_speed / state.wheelbase) * math.tan(steer_angle)
        new_heading = _wrap_angle(state.heading + yaw_rate * step.dt)

        dx = new_speed * math.cos(new_heading) * step.dt
        dy = new_speed * math.sin(new_heading) * step.dt
        return replace(state, heading=new_heading, speed=new_speed), (dx, dy)


@dataclass(kw_only=True, frozen=True)
class SharedSpaceState:
    """Dispatcher state tagging an agent's modality (``kind``) and sub-state."""

    kind: str
    sub: object


class SharedSpaceDispatcher(CustomOperationalModel):
    """Multi-model dispatcher coupling micromobility riders and pedestrians.

    Follows the dispatcher pattern in ``test_multi_model.py``: each sub-model
    receives a mapped view via :meth:`~jupedsim.AgentStep.with_neighbor_state_mapping`
    encoding how one road-user class perceives another:

    - **Wheeler perceiving a pedestrian:** reads the pedestrian's velocity (if
      available, or reconstructs it from ``orientation * desired_speed``) and
      body radius for predictive CPA passing.
    - **Social-force pedestrian perceiving a wheeler:** sees the wheeler's
      velocity vector and an expanded effective radius proportional to the
      wheeler's speed, eliciting realistic lateral clearance.
    - **Collision-free pedestrian perceiving a wheeler:** sees the wheeler's
      physical + speed-buffered radius as a ``CollisionFreeSpeedModelV3State``.
    """

    def __init__(
        self,
        *,
        wheeler_speed_radius_gain: float = 0.08,
        bicycle_model: KinematicBicycleModel | None = None,
    ) -> None:
        super().__init__()
        self._wheeler_gain = wheeler_speed_radius_gain
        self._bicycle = bicycle_model or KinematicBicycleModel()
        self._social_force = PythonSocialForceModel()
        self._collision_free = jps.CollisionFreeSpeedModelV3()

    @staticmethod
    def _as_wheeler_neighbor(
        neighbor: SharedSpaceState,
    ) -> PerceivedNeighborState:
        sub = neighbor.sub
        if neighbor.kind in ("wheeler", "social_force"):
            return PerceivedNeighborState(
                velocity=sub.velocity,  # type: ignore[attr-defined]
                radius=sub.radius,  # type: ignore[attr-defined]
            )
        # Built-in CollisionFreeSpeedModelV3State stores orientation + desired_speed
        orient = getattr(sub, "orientation", (0.0, 0.0))
        v0 = float(getattr(sub, "desired_speed", 0.0))
        return PerceivedNeighborState(
            velocity=(orient[0] * v0, orient[1] * v0),
            radius=float(getattr(sub, "radius", 0.2)),
        )

    def _as_social_force_neighbor(
        self, neighbor: SharedSpaceState
    ) -> PythonSocialForceModelState:
        sub = neighbor.sub
        if neighbor.kind == "social_force":
            return sub  # type: ignore[return-value]
        if neighbor.kind == "wheeler":
            vel = sub.velocity  # type: ignore[attr-defined]
            eff_r = sub.radius + self._wheeler_gain * sub.speed  # type: ignore[attr-defined]
            return PythonSocialForceModelState(velocity=vel, radius=eff_r)
        orient = getattr(sub, "orientation", (0.0, 0.0))
        v0 = float(getattr(sub, "desired_speed", 0.0))
        return PythonSocialForceModelState(
            velocity=(orient[0] * v0, orient[1] * v0),
            radius=float(getattr(sub, "radius", 0.2)),
        )

    def _as_collision_free_neighbor(
        self, neighbor: SharedSpaceState
    ) -> jps.CollisionFreeSpeedModelV3State:
        sub = neighbor.sub
        if neighbor.kind == "collision_free":
            return sub  # type: ignore[return-value]
        if neighbor.kind == "wheeler":
            eff_r = sub.radius + self._wheeler_gain * sub.speed  # type: ignore[attr-defined]
            return jps.CollisionFreeSpeedModelV3State(radius=eff_r)
        return jps.CollisionFreeSpeedModelV3State(
            radius=float(getattr(sub, "radius", 0.25))
        )

    def check_model_constraint(self, state: SharedSpaceState, view) -> None:
        if state.kind == "wheeler":
            sub_view = view.with_neighbor_state_mapping(
                self._as_wheeler_neighbor
            )
            self._bicycle.check_model_constraint(state.sub, sub_view)
        elif state.kind == "social_force":
            sub_view = view.with_neighbor_state_mapping(
                self._as_social_force_neighbor
            )
            self._social_force.check_model_constraint(state.sub, sub_view)
        elif state.kind == "collision_free":
            if not isinstance(state.sub, jps.CollisionFreeSpeedModelV3State):
                raise TypeError(
                    "Expected CollisionFreeSpeedModelV3State for 'collision_free', "
                    f"got {type(state.sub).__name__}."
                )
        else:
            raise ValueError(f"Unknown SharedSpaceState kind: {state.kind!r}.")

    def seed_agent_state(self, state: SharedSpaceState) -> SharedSpaceState:
        if state.kind == "wheeler":
            return replace(state, sub=self._bicycle.seed_agent_state(state.sub))
        if state.kind == "social_force":
            return replace(
                state, sub=self._social_force.seed_agent_state(state.sub)
            )
        if state.kind == "collision_free":
            return replace(
                state, sub=self._collision_free.seed_state(state.sub)
            )
        raise ValueError(f"Unknown SharedSpaceState kind: {state.kind!r}.")

    def compute_next_state(self, state: SharedSpaceState, step):
        if state.kind == "wheeler":
            sub_step = step.with_neighbor_state_mapping(
                self._as_wheeler_neighbor
            )
            new_sub, movement = self._bicycle.compute_next_state(
                state.sub, sub_step
            )
        elif state.kind == "social_force":
            sub_step = step.with_neighbor_state_mapping(
                self._as_social_force_neighbor
            )
            new_sub, movement = self._social_force.compute_next_state(
                state.sub, sub_step
            )
        elif state.kind == "collision_free":
            sub_step = step.with_neighbor_state_mapping(
                self._as_collision_free_neighbor
            )
            new_sub, movement = self._collision_free.compute_next_state(
                state.sub, sub_step
            )
        else:
            raise ValueError(f"Unknown SharedSpaceState kind: {state.kind!r}.")

        return replace(state, sub=new_sub), movement
