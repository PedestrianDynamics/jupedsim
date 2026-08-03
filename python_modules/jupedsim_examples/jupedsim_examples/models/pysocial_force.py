# SPDX-License-Identifier: LGPL-3.0-or-later
from dataclasses import dataclass, replace

import numpy as np
from jupedsim.geometry import LineSegment
from jupedsim.models.custom_model import CustomOperationalModel


@dataclass(kw_only=True, frozen=True)
class PythonSocialForceModelState:
    velocity: tuple[float, float]
    desired_speed: float = 1.34
    reaction_time: float = 0.5
    agent_scale: float = 2000
    obstacle_scale: float = 2000
    force_distance: float = 0.08
    mass: float = 80
    body_force: float = 120000
    friction: float = 240000
    radius: float = 0.3


class PythonSocialForceModel(CustomOperationalModel):
    """
    Implementation of the Helbing Social Force Model as described in:
    "Social Force Model for Pedestrian Dynamics" by Helbing & Molnár (1995)

    The model computes the acceleration of each agent as a superposition of:
    1. Desired force: drives agents toward their goal at desired speed
    2. Social force: repulsive forces between agents
    3. Obstacle force: repulsive forces from obstacles

    Parameters:
        desired_speed: desired walking speed [m/s], default 1.34
        reaction_time: time to adapt to new accelerations [s], default 0.5
        agent_scale: strength of repulsive force between agents [N], default 2000
        obstacle_scale: strength of repulsive force from obstacles [N], default 2000
        force_distance: range of social/obstacle forces [m], default 0.08
        mass: mass of agents [kg], default 80
        body_force: body contact force [kg/s²], default 120000
        friction: friction during contact [kg/m/s], default 240000
        radius: radius of agents [m], default 0.3

    All force helpers are static: math utilities (_normalize, _distance,
    _desired_force) operate on plain values, while _social_force and
    _obstacle_force read per-agent parameters from the state they are given.
    """

    def __init__(
        self,
    ):
        CustomOperationalModel.__init__(self)

    @staticmethod
    def _normalize(vector: tuple[float, float]) -> tuple[float, float]:
        """Normalize a 2D vector."""
        norm = np.sqrt(vector[0] ** 2 + vector[1] ** 2)
        if norm < 1e-10:
            return (0.0, 0.0)
        return (vector[0] / norm, vector[1] / norm)

    @staticmethod
    def _distance(p1: tuple[float, float], p2: tuple[float, float]) -> float:
        """Compute Euclidean distance between two points."""
        dx = p1[0] - p2[0]
        dy = p1[1] - p2[1]
        return np.sqrt(dx**2 + dy**2)

    @staticmethod
    def _desired_force(
        *,
        velocity: tuple[float, float],
        target_direction: tuple[float, float],
        desired_speed: float,
        reaction_time: float,
    ) -> tuple[float, float]:
        """
        Compute the desired force that drives agents toward their goal.

        F_desired = (v_desired - v_actual) / tau
        where v_desired = v0 * e_target and tau is reaction time
        eq2 in paper
        """
        v_desired_x = desired_speed * target_direction[0]
        v_desired_y = desired_speed * target_direction[1]

        fx = (v_desired_x - velocity[0]) / reaction_time
        fy = (v_desired_y - velocity[1]) / reaction_time

        return (fx, fy)

    @staticmethod
    def _social_force(state, neighbor) -> tuple[float, float]:
        """
        Compute repulsive social force between two agents.

        Based on Helbing's model with psychological and body contact forces.
        """
        # Pointing from the neighbor to the agent, i.e. the way it is pushed
        dx = -neighbor.relative_position[0]
        dy = -neighbor.relative_position[1]
        dist = np.sqrt(dx**2 + dy**2)

        # Minimum distance (sum of radii)
        min_dist = state.radius + neighbor.state.radius

        if dist < 1e-3:  # Avoid division by zero
            return (0.0, 0.0)

        # Normal direction (from other to agent)
        n_x = dx / dist
        n_y = dy / dist

        # Tangential direction
        t_x = -n_y
        t_y = n_x

        # Relative velocity
        dvx = state.velocity[0] - neighbor.state.velocity[0]
        dvy = state.velocity[1] - neighbor.state.velocity[1]
        dv_t = dvx * t_x + dvy * t_y  # tangential component

        # Distance-dependent factor
        exp_factor = np.exp(-(dist - min_dist) / state.force_distance)

        # Normal force (repulsive)
        f_n = state.agent_scale * exp_factor

        # Body contact force
        if dist < min_dist:
            f_body = state.body_force * (min_dist - dist)
            f_n = f_n + f_body

        # Friction (tangential)
        f_t = state.friction * exp_factor * dv_t if dist < min_dist else 0

        # Total force components
        fx = (f_n + f_t * t_x) * n_x - f_t * t_x
        fy = (f_n + f_t * t_y) * n_y - f_t * t_y

        return (fx, fy)

    @staticmethod
    def _obstacle_force(
        state,
        obstacle: LineSegment,
    ) -> tuple[float, float]:
        """
        Compute repulsive force from an obstacle (line segment).

        The obstacle is given relative to the agent, so the agent sits at (0, 0).

        Based on Helbing's model with psychological and body contact forces.
        """
        agent = (0.0, 0.0)
        # Get closest point on obstacle to agent
        closest_point = obstacle.closest_point(agent)
        dist = PythonSocialForceModel._distance(agent, closest_point)

        if dist < 1e-3:  # Avoid division by zero
            return (0.0, 0.0)

        # Normal direction (from obstacle to agent)
        n_x = -closest_point[0] / dist
        n_y = -closest_point[1] / dist

        # Distance-dependent factor
        exp_factor = np.exp(-dist / state.force_distance)

        # Normal force (repulsive)
        f_n = state.obstacle_scale * exp_factor

        # Body contact force
        if dist < state.radius:
            f_body = state.body_force * (state.radius - dist)
            f_n = f_n + f_body

        fx = f_n * n_x
        fy = f_n * n_y

        return (fx, fy)

    def compute_next_state(self, state, step):
        """
        Compute the agent's movement using the Social Force Model.

        Args:
            state: PythonSocialForceModelState of the agent being updated
            step: AgentStep, what the agent perceives during this step

        Returns:
            A pair of the updated state (via dataclasses.replace) and the
            movement the agent wants to make.
        """

        # eq 1 in paper
        target_dir = self._normalize(step.to_next_target)

        # Initialize acceleration from desired force
        acc_x, acc_y = self._desired_force(
            velocity=state.velocity,
            target_direction=target_dir,
            desired_speed=state.desired_speed,
            reaction_time=state.reaction_time,
        )

        # Add social forces from neighboring agents
        for neighbor in step.other_agents_in_range(2.0):
            fx, fy = self._social_force(state, neighbor)
            acc_x += fx / state.mass
            acc_y += fy / state.mass

        # Add obstacle forces (from geometry)
        for wall in step.walls_in_range(5.0):
            fx, fy = self._obstacle_force(state, wall)
            acc_x += fx / state.mass
            acc_y += fy / state.mass

        # Update velocity: v_new = v_old + a * dt
        new_velocity = (
            state.velocity[0] + acc_x * step.dt,
            state.velocity[1] + acc_y * step.dt,
        )

        movement = (new_velocity[0] * step.dt, new_velocity[1] * step.dt)

        return replace(state, velocity=new_velocity), movement
