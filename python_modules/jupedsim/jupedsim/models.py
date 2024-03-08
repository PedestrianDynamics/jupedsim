# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

import jupedsim.native as py_jps


@dataclass(kw_only=True)
class CollisionFreeSpeedModel:
    """Collision Free Speed Model

    A general description of the Collision Free Speed Model can be found in the originating publication
    https://arxiv.org/abs/1512.05597

    A more detailed description can be found at https://pedestriandynamics.org/models/collision_free_speed_model/

    All attributes are initialized with reasonably good defaults.

    Attributes:
        strength_neighbor_repulsion: Strength of the repulsion from neighbors
        range_neighbor_repulsion: Range of the repulsion from neighbors
        strength_geometry_repulsion: Strength of the repulsion from geometry boundaries
        range_geometry_repulsion: Range of the repulsion from geometry boundaries
    """

    strength_neighbor_repulsion: float = 8.0
    range_neighbor_repulsion: float = 0.1
    strength_geometry_repulsion: float = 5.0
    range_geometry_repulsion: float = 0.02


@dataclass(kw_only=True)
class CollisionFreeSpeedModelV2:
    """Collision Free Speed Model V2

    This is a variation of the Collision Free Speed Model where geometry and neighbor repulsion are individual
    agent parameters instead of global parameters.

    A general description of the Collision Free Speed Model can be found in the originating publication
    https://arxiv.org/abs/1512.05597

    A more detailed description can be found at https://pedestriandynamics.org/models/collision_free_speed_model/
    """

    pass


@dataclass(kw_only=True)
class GeneralizedCentrifugalForceModel:
    """Parameters for Generalized Centrifugal Force Model

    .. warning::
        Unfortunately we broke the model before the 1.0 release and did not notice it in time.
        For the time being please do not use this model.

        We track the defect here: https://github.com/PedestrianDynamics/jupedsim/issues/1337

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
    max_neighbor_repulsion_force: float = 3
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
        e0: Desired direction of the agent.
        position: Position of the agent.
        orientation: Orientation of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        mass: Mass of the agent.
        tau: Time constant that describes how fast the agent accelerates to its desired speed (v0).
        v0: Maximum speed of the agent.
        a_v: Stretch of the ellipsis semi-axis along the movement vector.
        a_min: Minimum length of the ellipsis semi-axis along the movement vector.
        b_min: Minimum length of the ellipsis semi-axis orthogonal to the movement vector.
        b_max: Maximum length of the ellipsis semi-axis orthogonal to the movement vector.
    """

    speed: float = 0.0
    e0: tuple[float, float] = (0.0, 0.0)
    position: tuple[float, float] = (0.0, 0.0)
    orientation: tuple[float, float] = (0.0, 0.0)
    journey_id: int = -1
    stage_id: int = -1
    mass: float = 1
    tau: float = 0.5
    v0: float = 1.2
    a_v: float = 1
    a_min: float = 0.2
    b_min: float = 0.2
    b_max: float = 0.4

    def as_native(
        self,
    ) -> py_jps.GeneralizedCentrifugalForceModelAgentParameters:
        return py_jps.GeneralizedCentrifugalForceModelAgentParameters(
            speed=self.speed,
            e0=self.e0,
            position=self.position,
            orientation=self.orientation,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
            mass=self.mass,
            tau=self.tau,
            v0=self.v0,
            a_v=self.a_v,
            a_min=self.a_min,
            b_min=self.b_min,
            b_max=self.b_max,
        )


@dataclass(kw_only=True)
class CollisionFreeSpeedModelAgentParameters:
    """
    Agent parameters for Collision Free Speed Model.

    See the scientific publication for more details about this model
    https://arxiv.org/abs/1512.05597

    .. note::

        Instances of this type are copied when creating the agent, you can safely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

        .. code:: python

            positions = [...] # List of initial agent positions
            params = CollisionFreeSpeedModelAgentParameters(v0=0.9) # all agents are slower
            for p in positions:
                params.position = p
                sim.add_agent(params)

    Attributes:
        position: Position of the agent.
        time_gap: Time constant that describe how fast pedestrian close gaps.
        v0: Maximum speed of the agent.
        radius: Radius of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
    """

    position: tuple[float, float] = (0.0, 0.0)
    time_gap: float = 1.0
    v0: float = 1.2
    radius: float = 0.2
    journey_id: int = 0
    stage_id: int = 0

    def as_native(self) -> py_jps.CollisionFreeSpeedModelAgentParameters:
        return py_jps.CollisionFreeSpeedModelAgentParameters(
            position=self.position,
            time_gap=self.time_gap,
            v0=self.v0,
            radius=self.radius,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
        )


@dataclass(kw_only=True)
class CollisionFreeSpeedModelV2AgentParameters:
    """
    Agent parameters for Collision Free Speed Model V2.

    See the scientific publication for more details about this model
    https://arxiv.org/abs/1512.05597

    .. note::

        Instances of this type are copied when creating the agent, you can safely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

        .. code:: python

            positions = [...] # List of initial agent positions
            params = CollisionFreeSpeedModelV2AgentParameters(v0=0.9) # all agents are slower
            for p in positions:
                params.position = p
                sim.add_agent(params)

    Attributes:
        position: Position of the agent.
        time_gap: Time constant that describe how fast pedestrian close gaps.
        v0: Maximum speed of the agent.
        radius: Radius of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        strength_neighbor_repulsion: Strength of the repulsion from neighbors
        range_neighbor_repulsion: Range of the repulsion from neighbors
        strength_geometry_repulsion: Strength of the repulsion from geometry boundaries
        range_geometry_repulsion: Range of the repulsion from geometry boundaries
    """

    position: tuple[float, float] = (0.0, 0.0)
    time_gap: float = 1.0
    v0: float = 1.2
    radius: float = 0.2
    journey_id: int = 0
    stage_id: int = 0
    strength_neighbor_repulsion: float = 8.0
    range_neighbor_repulsion: float = 0.1
    strength_geometry_repulsion: float = 5.0
    range_geometry_repulsion: float = 0.02

    def as_native(
        self,
    ) -> py_jps.CollisionFreeSpeedModelV2AgentParameters:
        return py_jps.CollisionFreeSpeedModelV2AgentParameters(
            position=self.position,
            time_gap=self.time_gap,
            v0=self.v0,
            radius=self.radius,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
            strength_neighbor_repulsion=self.strength_neighbor_repulsion,
            range_neighbor_repulsion=self.range_neighbor_repulsion,
            strength_geometry_repulsion=self.strength_geometry_repulsion,
            range_geometry_repulsion=self.range_geometry_repulsion,
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
    def e0(self) -> tuple[float, float]:
        """Desired direction of this agent."""
        return self._obj.e0

    @e0.setter
    def e0(self, e0):
        self._obj.e0 = e0

    @property
    def tau(self) -> float:
        return self._obj.tau

    @tau.setter
    def tau(self, tau):
        self._obj.tau = tau

    @property
    def v0(self) -> float:
        """Maximum speed of this agent."""
        return self._obj.v0

    @v0.setter
    def v0(self, v0):
        self._obj.v0 = v0

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


class CollisionFreeSpeedModelState:
    def __init__(self, backing):
        self._obj = backing

    @property
    def time_gap(self) -> float:
        return self._obj.time_gap

    @time_gap.setter
    def time_gap(self, time_gap):
        self._obj.time_gap = time_gap

    @property
    def v0(self) -> float:
        """Maximum speed of this agent."""
        return self._obj.v0

    @v0.setter
    def v0(self, v0):
        self._obj.v0 = v0

    @property
    def radius(self) -> float:
        """Radius of this agent."""
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius


class CollisionFreeSpeedModelV2State:
    def __init__(self, backing):
        self._obj = backing

    @property
    def time_gap(self) -> float:
        return self._obj.time_gap

    @time_gap.setter
    def time_gap(self, time_gap):
        self._obj.time_gap = time_gap

    @property
    def v0(self) -> float:
        """Maximum speed of this agent."""
        return self._obj.v0

    @v0.setter
    def v0(self, v0):
        self._obj.v0 = v0

    @property
    def radius(self) -> float:
        """Radius of this agent."""
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius

    @property
    def strength_neighbor_repulsion(self) -> float:
        """Strength of the repulsion from neighbors of this agent."""
        return self._obj.strength_neighbor_repulsion

    @strength_neighbor_repulsion.setter
    def strength_neighbor_repulsion(self, strength_neighbor_repulsion):
        self._obj.strength_neighbor_repulsion = strength_neighbor_repulsion

    @property
    def range_neighbor_repulsion(self) -> float:
        """Range of the repulsion from neighbors of this agent."""
        return self._obj.range_neighbor_repulsion

    @range_neighbor_repulsion.setter
    def range_neighbor_repulsion(self, range_neighbor_repulsion):
        self._obj.range_neighbor_repulsion = range_neighbor_repulsion

    @property
    def strength_geometry_repulsion(self) -> float:
        """Strength of the repulsion from geometry boundaries of this agent."""
        return self._obj.strength_geometry_repulsion

    @strength_geometry_repulsion.setter
    def strength_geometry_repulsion(self, strength_geometry_repulsion):
        self._obj.strength_geometry_repulsion = strength_geometry_repulsion

    @property
    def range_geometry_repulsion(self) -> float:
        """Range of the repulsion from geometry boundaries of this agent."""
        return self._obj.range_geometry_repulsion

    @range_geometry_repulsion.setter
    def range_geometry_repulsion(self, range_geometry_repulsion):
        self._obj.range_geometry_repulsion = range_geometry_repulsion
