# SPDX-License-Identifier: LGPL-3.0-or-later

import warnings
from dataclasses import dataclass

try:
    from warnings import deprecated
except ImportError:
    from deprecated import deprecated


@dataclass(kw_only=True)
class SocialForceModel:
    r"""Parameters for Social Force Model

    All attributes are initialized with reasonably good defaults.

    See the scientific publication for more details about this model
    https://doi.org/10.1038/35035023

    Attributes:
        body_force: describes the strength with which an agent is influenced by pushing forces from obstacles and neighbors in its direct proximity. [in kg s^-2] (is called k)
        friction: describes the strength with which an agent is influenced by frictional forces from obstacles and neighbors in its direct proximity. [in kg m^-1 s^-1] (is called :math:`\kappa`)
    """

    body_force: float = 120000  # [kg s^-2] is called k
    friction: float = 240000  # [kg m^-1 s^-1] is called kappa

    def __init__(
        self,
        *,
        body_force: float = 120000,
        friction: float = 240000,
        bodyForce=None,
    ):
        """
        Init dataclass SocialForceMode to handle deprecated arguments.
        """
        self.body_force = body_force

        if bodyForce is not None:
            warnings.warn(
                "'bodyForce' is deprecated, use 'body_force' instead.",
                DeprecationWarning,
                stacklevel=2,
            )
            self.body_force = bodyForce
        self.friction = friction

    @property
    @deprecated("deprecated, use 'body_force' instead.")
    def bodyForce(self) -> float:
        return self.body_force

    @bodyForce.setter
    @deprecated("deprecated, use 'body_force' instead.")
    def bodyForce(self, bodyForce):
        self.body_force = bodyForce


@dataclass(kw_only=True)
class SocialForceModelAgentParameters:
    """
    Parameters required to create an Agent in the Social Force Model.

    See the scientific publication for more details about this model
    https://doi.org/10.1038/35035023

    Attributes:
        position: Position of the agent.
        orientation: Orientation of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        velocity: current velocity of the agent.
        mass: mass of the agent. [in kg] (is called m)
        desired_speed: desired Speed of the agent. [in m/s] (is called v0)
        reaction_time: reaction Time of the agent. [in s] (is called :math:`\\tau`)
        agent_scale: indicates how strong an agent is influenced by pushing forces from neighbors. [in N] (is called A)
        obstacle_scale: indicates how strong an agent is influenced by pushing forces from obstacles. [in N] (is called A)
        force_distance: indicates how much the distance between an agent and obstacles or neighbors influences social forces. [in m] (is called B)
        radius: radius of the space an agent occupies. [in m] (is called r)
    """

    # todo write force equation from paper
    position: tuple[float, float] = (0.0, 0.0)
    orientation: tuple[float, float] = (0.0, 0.0)
    journey_id: int = -1
    stage_id: int = -1
    velocity: tuple[float, float] = (0.0, 0.0)
    # the values are from paper. doi is in class description
    mass: float = 80.0  # [kg] is called m
    desired_speed: float = (
        0.8  # [m / s] is called v0 can be set higher depending on situation
    )
    reaction_time: float = 0.5  # [s] is called tau
    agent_scale: float = 2000  # [N] is called A
    obstacle_scale: float = 2000  # [N] is called A
    force_distance: float = 0.08  # [m] is called B
    radius: float = (
        0.3  # [m] in paper 2r is uniformy distibuted in interval [0.5 m, 0.7 m]
    )

    def __init__(
        self,
        position: tuple[float, float] = (0.0, 0.0),
        orientation: tuple[float, float] = (0.0, 0.0),
        journey_id: int = -1,
        stage_id: int = -1,
        velocity: tuple[float, float] = (0.0, 0.0),
        mass: float = 80.0,
        desired_speed: float = 0.8,
        reaction_time: float = 0.5,
        agent_scale: float = 2000,
        obstacle_scale: float = 2000,
        force_distance: float = 0.08,
        radius: float = 0.3,
        desiredSpeed=None,
        reactionTime=None,
        agentScale=None,
        obstacleScale=None,
        forceDistance=None,
    ):
        """Init dataclass to handle deprecated arguments."""
        self.position = position
        self.orientation = orientation
        self.journey_id = journey_id
        self.stage_id = stage_id
        self.velocity = velocity
        self.mass = mass
        self.radius = radius

        deprecated_map = {
            "desiredSpeed": "desired_speed",
            "reactionTime": "reaction_time",
            "agentScale": "agent_scale",
            "obstacleScale": "obstacle_scale",
            "forceDistance": "force_distance",
        }
        for old_name, new_name in deprecated_map.items():
            if locals()[old_name]:
                warnings.warn(
                    f"'{old_name}' is deprecated, use '{new_name}' instead.",
                    DeprecationWarning,
                    stacklevel=2,
                )
                setattr(self, new_name, locals()[old_name])
            else:
                setattr(self, new_name, locals()[new_name])

    @property
    @deprecated("deprecated, use 'desired_speed' instead.")
    def desiredSpeed(self) -> float:
        return self.desired_speed

    @desiredSpeed.setter
    @deprecated("deprecated, use 'desired_speed' instead.")
    def desiredSpeed(self, desiredSpeed):
        self.desired_speed = desiredSpeed

    @property
    @deprecated("deprecated, use 'reaction_time' instead.")
    def reactionTime(self) -> float:
        return self.reaction_time

    @reactionTime.setter
    @deprecated("deprecated, use 'reaction_time' instead.")
    def reactionTime(self, reactionTime):
        self.reaction_time = reactionTime

    @property
    @deprecated("deprecated, use 'agent_scale' instead.")
    def agentScale(self) -> float:
        return self.agent_scale

    @agentScale.setter
    @deprecated("deprecated, use 'agent_scale' instead.")
    def agentScale(self, agentScale):
        self.agent_scale = agentScale

    @property
    @deprecated("deprecated, use 'obstacle_scale' instead.")
    def obstacleScale(self) -> float:
        return self.obstacle_scale

    @obstacleScale.setter
    @deprecated("deprecated, use 'obstacle_scale' instead.")
    def obstacleScale(self, obstacleScale):
        self.obstacle_scale = obstacleScale

    @property
    @deprecated("deprecated, use 'force_distance' instead.")
    def forceDistance(self) -> float:
        return self.force_distance

    @forceDistance.setter
    @deprecated("deprecated, use 'force_distance' instead.")
    def forceDistance(self, forceDistance):
        self.force_distance = forceDistance


class SocialForceModelState:
    def __init__(self, backing) -> None:
        self._obj = backing

    @property
    def velocity(self) -> tuple[float, float]:
        """velocity of this agent."""
        return self._obj.velocity

    @velocity.setter
    def velocity(self, velocity):
        self._obj.velocity = velocity

    @property
    def mass(self) -> float:
        """mass of this agent."""
        return self._obj.mass

    @mass.setter
    def mass(self, mass):
        self._obj.mass = mass

    @property
    def desiredSpeed(self) -> float:
        """desired Speed of this agent."""
        warnings.warn(
            "'desiredSpeed' is deprecated, use 'desired_speed' instead.",
            DeprecationWarning,
            stacklevel=2,
        )
        return self._obj.desired_speed

    @desiredSpeed.setter
    def desiredSpeed(self, desiredSpeed):
        warnings.warn(
            "'desiredSpeed' is deprecated, use 'desired_speed' instead.",
            DeprecationWarning,
            stacklevel=2,
        )
        self._obj.desired_speed = desiredSpeed

    @property
    def desired_speed(self) -> float:
        """desired Speed of this agent."""
        return self._obj.desired_speed

    @desired_speed.setter
    def desired_speed(self, desired_speed):
        self._obj.desired_speed = desired_speed

    @property
    @deprecated("deprecated, use 'reaction_time' instead.")
    def reactionTime(self) -> float:
        """reaction Time of this agent."""
        return self._obj.reaction_time

    @reactionTime.setter
    @deprecated("deprecated, use 'reaction_time' instead.")
    def reactionTime(self, reactionTime):
        self._obj.reaction_time = reactionTime

    @property
    def reaction_time(self) -> float:
        return self._obj.reaction_time

    @reaction_time.setter
    def reaction_time(self, reaction_time):
        self._obj.reaction_time = reaction_time

    @property
    @deprecated("deprecated, use 'agent_scale' instead.")
    def agentScale(self) -> float:
        return self._obj.agent_scale

    @agentScale.setter
    @deprecated("deprecated, use 'agent_scale' instead.")
    def agentScale(self, agentScale):
        self._obj.agent_scale = agentScale

    @property
    def agent_scale(self) -> float:
        return self._obj.agent_scale

    @agent_scale.setter
    def agent_scale(self, agent_scale):
        self._obj.agent_scale = agent_scale

    @property
    @deprecated("deprecated, use 'obstacle_scale' instead.")
    def obstacleScale(self) -> float:
        return self._obj.obstacle_scale

    @obstacleScale.setter
    @deprecated("deprecated, use 'obstacle_scale' instead.")
    def obstacleScale(self, obstacleScale):
        self._obj.obstacle_scale = obstacleScale

    @property
    def obstacle_scale(self) -> float:
        return self._obj.obstacle_scale

    @obstacle_scale.setter
    def obstacle_scale(self, obstacle_scale):
        self._obj.obstacle_scale = obstacle_scale

    @property
    @deprecated("deprecated, use 'force_distance' instead.")
    def forceDistance(self) -> float:
        return self._obj.force_distance

    @forceDistance.setter
    @deprecated("deprecated, use 'force_distance' instead.")
    def forceDistance(self, forceDistance):
        self._obj.force_distance = forceDistance

    @property
    def force_distance(self) -> float:
        return self._obj.force_distance

    @force_distance.setter
    def force_distance(self, force_distance):
        self._obj.force_distance = force_distance

    @property
    def radius(self) -> float:
        """radius of this agent."""
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius
