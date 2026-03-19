# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

import jupedsim.native as py_jps


@dataclass(kw_only=True)
class SocialForceModelIPP:
    r"""Two-level pedestrian model (Inverted Pendulum Paradigm).

    Each agent has an upper body and a ground support (legs) coupled
    via unbalancing/balance-recovery dynamics.  All parameters are
    per-agent; the model itself has no constructor parameters.
    """

    pass


@dataclass(kw_only=True)
class SocialForceModelIPPAgentParameters:
    """Parameters required to create an Agent in the SocialForceModelIPP.

    Attributes:
        position: Position of the agent.
        orientation: Orientation of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        velocity: Current velocity of the upper body.
        ground_support_position: Position of the ground support circle center.
        ground_support_velocity: Velocity of the ground support circle center.
        height: Height of the agent [m].
        desired_speed: Desired speed v0 [m/s].
        reaction_time: Driving force relaxation time tau [s].
        lambda_u: Unbalancing rate [1/s].
        lambda_b: Balancing rate [1/s].
        balance_speed: Coupling speed v_s [m/s].
        damping: Upper body velocity dissipation rate [1/s].
        agent_scale: Exponential repulsion amplitude A vs agents [N].
        obstacle_scale: Exponential repulsion amplitude A_w vs walls [N].
        force_distance: Upper body interaction range B [m].
        obstacle_force_distance: Wall interaction range B_w [m].
        leg_force_distance: Leg interaction range B_leg [m].
        radius: Upper body radius [m].
    """

    position: tuple[float, float] = (0.0, 0.0)
    orientation: tuple[float, float] = (0.0, 0.0)
    journey_id: int = -1
    stage_id: int = -1
    velocity: tuple[float, float] = (0.0, 0.0)
    ground_support_position: tuple[float, float] | None = None
    ground_support_velocity: tuple[float, float] = (0.0, 0.0)
    height: float = 1.75
    desired_speed: float = 1.34
    reaction_time: float = 0.5
    lambda_u: float = 0.5
    lambda_b: float = 1.0
    balance_speed: float = 1.0
    damping: float = 1.0
    agent_scale: float = 5.0
    obstacle_scale: float = 5.0
    force_distance: float = 0.5
    obstacle_force_distance: float = 0.2
    leg_force_distance: float = 0.3
    radius: float = 0.15

    def as_native(
        self,
    ) -> py_jps.SocialForceModelIPPAgentParameters:
        gs_pos = self.ground_support_position if self.ground_support_position is not None else self.position
        return py_jps.SocialForceModelIPPAgentParameters(
            position=self.position,
            orientation=self.orientation,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
            velocity=self.velocity,
            ground_support_position=gs_pos,
            ground_support_velocity=self.ground_support_velocity,
            height=self.height,
            desired_speed=self.desired_speed,
            reaction_time=self.reaction_time,
            lambda_u=self.lambda_u,
            lambda_b=self.lambda_b,
            balance_speed=self.balance_speed,
            damping=self.damping,
            agent_scale=self.agent_scale,
            obstacle_scale=self.obstacle_scale,
            force_distance=self.force_distance,
            obstacle_force_distance=self.obstacle_force_distance,
            leg_force_distance=self.leg_force_distance,
            radius=self.radius,
        )


class SocialForceModelIPPState:
    def __init__(self, backing) -> None:
        self._obj = backing

    @property
    def velocity(self) -> tuple[float, float]:
        """Velocity of this agent."""
        return self._obj.velocity

    @velocity.setter
    def velocity(self, velocity):
        self._obj.velocity = velocity

    @property
    def ground_support_position(self) -> tuple[float, float]:
        """Ground support position of this agent."""
        return self._obj.ground_support_position

    @ground_support_position.setter
    def ground_support_position(self, ground_support_position):
        self._obj.ground_support_position = ground_support_position

    @property
    def ground_support_velocity(self) -> tuple[float, float]:
        """Ground support velocity of this agent."""
        return self._obj.ground_support_velocity

    @ground_support_velocity.setter
    def ground_support_velocity(self, ground_support_velocity):
        self._obj.ground_support_velocity = ground_support_velocity

    @property
    def height(self) -> float:
        """Height of this agent."""
        return self._obj.height

    @height.setter
    def height(self, height):
        self._obj.height = height

    @property
    def desired_speed(self) -> float:
        """Desired speed of this agent."""
        return self._obj.desired_speed

    @desired_speed.setter
    def desired_speed(self, desired_speed):
        self._obj.desired_speed = desired_speed

    @property
    def reaction_time(self) -> float:
        """Reaction time of this agent."""
        return self._obj.reaction_time

    @reaction_time.setter
    def reaction_time(self, reaction_time):
        self._obj.reaction_time = reaction_time

    @property
    def lambda_u(self) -> float:
        """Unbalancing rate."""
        return self._obj.lambda_u

    @lambda_u.setter
    def lambda_u(self, lambda_u):
        self._obj.lambda_u = lambda_u

    @property
    def lambda_b(self) -> float:
        """Balancing rate."""
        return self._obj.lambda_b

    @lambda_b.setter
    def lambda_b(self, lambda_b):
        self._obj.lambda_b = lambda_b

    @property
    def balance_speed(self) -> float:
        """Coupling speed."""
        return self._obj.balance_speed

    @balance_speed.setter
    def balance_speed(self, balance_speed):
        self._obj.balance_speed = balance_speed

    @property
    def damping(self) -> float:
        """Velocity dissipation rate."""
        return self._obj.damping

    @damping.setter
    def damping(self, damping):
        self._obj.damping = damping

    @property
    def agent_scale(self) -> float:
        """Exponential repulsion amplitude."""
        return self._obj.agent_scale

    @agent_scale.setter
    def agent_scale(self, agent_scale):
        self._obj.agent_scale = agent_scale

    @property
    def obstacle_scale(self) -> float:
        """Exponential repulsion amplitude vs walls."""
        return self._obj.obstacle_scale

    @obstacle_scale.setter
    def obstacle_scale(self, obstacle_scale):
        self._obj.obstacle_scale = obstacle_scale

    @property
    def force_distance(self) -> float:
        """Upper body interaction range."""
        return self._obj.force_distance

    @force_distance.setter
    def force_distance(self, force_distance):
        self._obj.force_distance = force_distance

    @property
    def obstacle_force_distance(self) -> float:
        """Wall interaction range."""
        return self._obj.obstacle_force_distance

    @obstacle_force_distance.setter
    def obstacle_force_distance(self, obstacle_force_distance):
        self._obj.obstacle_force_distance = obstacle_force_distance

    @property
    def leg_force_distance(self) -> float:
        """Leg interaction range."""
        return self._obj.leg_force_distance

    @leg_force_distance.setter
    def leg_force_distance(self, leg_force_distance):
        self._obj.leg_force_distance = leg_force_distance

    @property
    def radius(self) -> float:
        """Radius of this agent."""
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius
