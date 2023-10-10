# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

import jupedsim.native as py_jps


@dataclass(kw_only=True)
class VelocityModel:
    """Parameters for Velocity Model

    All attributes are initialized with reasonably good defaults.

    Attributes:
        a_ped (float): Strength of the repulsion from neighbors
        d_ped (float): Range of the repulsion from neighbors
        a_wall (float): Strength of the repulsion from geometry boundaries
        d_wall (float): Range of the repulsion from geometry boundaries
    """

    a_ped: float = 8.0
    d_ped: float = 0.1
    a_wall: float = 5.0
    d_wall: float = 0.02


@dataclass(kw_only=True)
class GeneralizedCentrifugalForceModel:
    """Parameters for Generalized Centrifugal Force Model

    # All attributes are initialized with reasonably good defaults.

    Attributes:
        nu_ped (float):
        nu_wall (float):
        dist_eff_ped (float)
        dist_eff_wall (float)
        intp_width_ped (float)
        intp_width_wall (float)
        maxf_ped (float)
        maxf_wall (float)
    """

    nu_ped: float = 0.3
    nu_wall: float = 0.2
    dist_eff_ped: float = 2
    dist_eff_wall: float = 2
    intp_width_ped: float = 0.1
    intp_width_wall: float = 0.1
    maxf_ped: float = 3
    maxf_wall: float = 3


@dataclass(kw_only=True)
class GeneralizedCentrifugalForceModelAgentParameters:
    """
    Parameters required to create an Agent in the Generalized Centrifugal Force
    Model.

    See the scientific publication for more details about this model
    https://arxiv.org/abs/1008.4297

    .. note::
        Insances of this type are copied when creating the agent, you can savely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

        .. code:: python

            positions = [...] # List of initial agent positions
            params = GeneralizedCentrifugalForceModelAgentParameters(speed=0.9) # all agents are slower
            for p in positions:
                params.position = p
                sim.add_agent(params)

    Attributes:
        speed (float):
        e0 (tuple[float, float]):
        position (tuple[float, float]):
        orientation (tuple[float, float]):
        journey_id (int):
        stage_id (int):
        mass (float):
        tau (float):
        v0 (float):
        a_v (float):
        a_min (float):
        b_min (float):
        b_max (float):
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

    def as_native(self) -> py_jps.GCFMModelAgentParameters:
        return py_jps.GCFMModelAgentParameters(
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
class VelocityModelAgentParameters:
    """
    Agent parameters for Velocity Model.

    See the scientific publication for more details about this model
    https://arxiv.org/abs/1512.05597

    .. note::
        Insances of this type are copied when creating the agent, you can savely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

            .. code:: python

            positions = [...] # List of initial agent positions
            params = VelocityModelAgentParameters(speed=0.9) # all agents are slower
            for p in positions:
                params.position = p
                sim.add_agent(params)

    Attributes:
        position (tuple[float, float]):
        time_gap (float):
        v0 (float):
        radius (float):
        journey_id (int):
        stage_id (int):
    """

    position: tuple[float, float] = (0.0, 0.0)
    time_gap: float = 1.0
    v0: float = 1.2
    radius: float = 0.2
    journey_id: int = 0
    stage_id: int = 0

    def as_native(self) -> py_jps.VelocityModelAgentParameters:
        return py_jps.VelocityModelAgentParameters(
            position=self.position,
            time_gap=self.time_gap,
            v0=self.v0,
            radius=self.radius,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
        )


class GeneralizedCentrifugalForceModelState:
    def __init__(self, backing):
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
        """Stretch of the elipsis semi-axis along the movement vector."""
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


class VelocityModelState:
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
