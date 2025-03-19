# SPDX-License-Identifier: LGPL-3.0-or-later

import warnings
from dataclasses import dataclass

try:
    from warnings import deprecated
except ImportError:
    from deprecated import deprecated

import jupedsim.native as py_jps


@dataclass(kw_only=True)
class GeneralizedCentrifugalForceModel:
    """Parameters for Generalized Centrifugal Force Model

    All attributes are initialized with reasonably good defaults.

    Attributes:
        strength_neighbor_repulsion: Strength of the repulsion from neighbors
        strength_geometry_repulsion: Strength of the repulsion from geometry boundaries
        max_neighbor_interaction_distance: cut-off-radius for ped-ped repulsion (r_c in FIG. 7)
        max_geometry_interaction_distance: cut-off-radius for ped-wall repulsion (r_c in FIG. 7)
        max_neighbor_interpolation_distance: distance of interpolation of repulsive force for ped-ped interaction (r_eps in FIG. 7)
        max_geometry_interpolation_distance: distance of interpolation of repulsive force for ped-wall interaction (r_eps in FIG. 7)
        max_neighbor_repulsion_force: maximum of the repulsion force for ped-ped interaction by contact of ellipses (f_m in FIG. 7)
        max_geometry_repulsion_force: maximum of the repulsion force for ped-wall interaction by contact of ellipses (f_m in FIG. 7)
    """

    strength_neighbor_repulsion: float = 0.3
    strength_geometry_repulsion: float = 0.2
    max_neighbor_interaction_distance: float = 2
    max_geometry_interaction_distance: float = 2
    max_neighbor_interpolation_distance: float = 0.1
    max_geometry_interpolation_distance: float = 0.1
    max_neighbor_repulsion_force: float = 9
    max_geometry_repulsion_force: float = 3


@dataclass(kw_only=True)
class GeneralizedCentrifugalForceModelAgentParameters:
    """
    Parameters required to create an Agent in the Generalized Centrifugal Force
    Model.

    See the scientific publication for more details about this model
    https://arxiv.org/abs/1008.4297

    .. note::
        Instances of this type are copied when creating the agent, you can safely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

        .. code:: python

            positions = [...] # List of initial agent positions
            params = GeneralizedCentrifugalForceModelAgentParameters(speed=0.9) # all agents are slower
            for p in positions:
                params.position = p
                sim.add_agent(params)

    Attributes:
        speed: Speed of the agent.
        desired_direction: Desired direction of the agent.
        position: Position of the agent.
        orientation: Orientation of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        mass: Mass of the agent.
        tau: Time constant that describes how fast the agent accelerates to its desired speed (v0).
        desired_speed: Maximum speed of the agent.
        a_v: Stretch of the ellipsis semi-axis along the movement vector.
        a_min: Minimum length of the ellipsis semi-axis along the movement vector.
        b_min: Minimum length of the ellipsis semi-axis orthogonal to the movement vector.
        b_max: Maximum length of the ellipsis semi-axis orthogonal to the movement vector.
    """

    speed: float = 0.0
    desired_direction: tuple[float, float] = (0.0, 0.0)
    position: tuple[float, float] = (0.0, 0.0)
    orientation: tuple[float, float] = (1.0, 0.0)
    journey_id: int = -1
    stage_id: int = -1
    mass: float = 1
    tau: float = 0.5
    desired_speed: float = 1.2
    a_v: float = 1
    a_min: float = 0.2
    b_min: float = 0.2
    b_max: float = 0.4

    def __init__(
        self,
        *,
        speed: float = 0.0,
        desired_direction: tuple[float, float] = (0.0, 0.0),
        position: tuple[float, float] = (0.0, 0.0),
        orientation: tuple[float, float] = (1.0, 0.0),
        journey_id: int = -1,
        stage_id: int = -1,
        mass: float = 1,
        tau: float = 0.5,
        desired_speed: float = 1.2,
        a_v: float = 1,
        a_min: float = 0.2,
        b_min: float = 0.2,
        b_max: float = 0.4,
        v0=None,
        e0=None,
    ):
        """Init dataclass to handle deprecated argument."""
        self.speed = speed
        self.desired_direction = desired_direction
        self.position = position
        self.orientation = orientation
        self.journey_id = journey_id
        self.stage_id = stage_id
        self.mass = mass
        self.tau = tau
        self.a_v = a_v
        self.a_min = a_min
        self.b_min = b_min
        self.b_max = b_max

        if v0 is not None:
            warnings.warn(
                "'v0' is deprecated, use 'desired_speed' instead.",
                DeprecationWarning,
                stacklevel=2,
            )
            self.desired_speed = v0
        else:
            self.desired_speed = desired_speed

        if e0 is not None:
            warnings.warn(
                "'e0' is deprecated, use 'desired_direction' instead.",
                DeprecationWarning,
                stacklevel=2,
            )
            self.desired_direction = e0
        else:
            self.desired_direction = desired_direction

    @property
    @deprecated("deprecated, use 'desired_direction' instead.")
    def e0(self) -> tuple[float, float]:
        return self.desired_direction

    @e0.setter
    @deprecated("deprecated, use 'desired_direction' instead.")
    def e0(self, e0):
        self.desired_direction = e0

    def as_native(
        self,
    ) -> py_jps.GeneralizedCentrifugalForceModelAgentParameters:
        return py_jps.GeneralizedCentrifugalForceModelAgentParameters(
            speed=self.speed,
            desired_direction=self.desired_direction,
            position=self.position,
            orientation=self.orientation,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
            mass=self.mass,
            tau=self.tau,
            desired_speed=self.desired_speed,
            a_v=self.a_v,
            a_min=self.a_min,
            b_min=self.b_min,
            b_max=self.b_max,
        )


class GeneralizedCentrifugalForceModelState:
    def __init__(self, backing) -> None:
        self._obj = backing

    @property
    def speed(self) -> float:
        """Speed of this agent."""
        return self._obj.speed

    @speed.setter
    def speed(self, speed):
        self._obj.speed = speed

    @property
    def desired_direction(self) -> float:
        """desired direction of this agent."""
        return self._obj.desired_direction

    @desired_direction.setter
    def desired_direction(self, desired_direction):
        self._obj.desired_direction = desired_direction

    @property
    @deprecated("deprecated, use 'desired_direction' instead.")
    def e0(self) -> tuple[float, float]:
        """Desired direction of this agent."""
        return self._obj.desired_speed

    @e0.setter
    @deprecated("deprecated, use 'desired_direction' instead.")
    def e0(self, e0):
        self._obj.desired_speed = e0

    @property
    def tau(self) -> float:
        return self._obj.tau

    @tau.setter
    def tau(self, tau):
        self._obj.tau = tau

    @property
    def desired_speed(self) -> float:
        """desired Speed of this agent."""
        return self._obj.desired_speed

    @desired_speed.setter
    def desired_speed(self, desired_speed):
        self._obj.desired_speed = desired_speed

    @property
    @deprecated("deprecated, use 'desired_speed' instead.")
    def v0(self) -> float:
        """Maximum speed of this agent."""
        return self._obj.desired_speed

    @v0.setter
    @deprecated("deprecated, use 'desired_speed' instead.")
    def v0(self, v0):
        self._obj.desired_speed = v0

    @property
    def a_v(self) -> float:
        """Stretch of the ellipsis semi-axis along the movement vector."""
        return self._obj.a_v

    @a_v.setter
    def a_v(self, a_v):
        self._obj.a_v = a_v

    @property
    def a_min(self) -> float:
        """Minimum length of the ellipsis semi-axis along the movement vector."""
        return self._obj.a_min

    @a_min.setter
    def a_min(self, a_min):
        self._obj.a_min = a_min

    @property
    def b_min(self) -> float:
        """Minimum length of the ellipsis semi-axis orthogonal to the movement vector."""
        return self._obj.b_min

    @b_min.setter
    def b_min(self, b_min):
        self._obj.b_min = b_min

    @property
    def b_max(self) -> float:
        """Maximum length of the ellipsis semi-axis orthogonal to the movement vector."""
        return self._obj.b_max

    @b_max.setter
    def b_max(self, b_max):
        self._obj.b_max = b_max
