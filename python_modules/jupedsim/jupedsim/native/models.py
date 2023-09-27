# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps


@dataclass(kw_only=True)
class VelocityModelParameters:
    """Parameters for Velocity Model

    All attributes are initialized with reasonably good defaults.

    Attributes:
        a_ped (float): TODO
        d_ped (float): TODO
        a_wall (float):
        d_wall (float):
    """

    a_ped: float = 8.0
    d_ped: float = 0.1
    a_wall: float = 5.0
    d_wall: float = 0.02


@dataclass(kw_only=True)
class GeneralizedCentrifugalForceModelParameters:
    """Parameters for Generalized Centrifugal Force Model

    All attributes are initialized with reasonably good defaults.

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


class GeneralizedCentrifugalForceModelAgentParameters:
    """
    Agent parameters for Generalized Centrifugal Force Model.

    See the scientifc publication for more details about this model
    https://arxiv.org/abs/1008.4297

    Objects of this type can be used to add new agents to the simulation and are
    returned by the simulation when inspecting agent state. Setting properties on
    objects returned by the simulation has no effect on the agents as this object
    is a copy of internal state.

    Setting properties on this object is only useful when adding multiple agents
    and they share many properties without reprating them on each 'add_agent'
    call
    """

    def __init__(self):
        self._obj = py_jps.GCFMModelAgentParameters()

    @property
    def speed(self) -> float:
        """
        Current speed

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.speed

    @speed.setter
    def speed(self, value: float) -> None:
        self._obj.speed = value

    @property
    def e0(self) -> tuple[float, float]:
        """
        e0 (Currently desired orientation)

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.e0

    @e0.setter
    def e0(self, value: tuple[float, float]) -> None:
        self._obj.e0 = value

    @property
    def position(self) -> tuple[float, float]:
        """
        Current position

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.position

    @position.setter
    def position(self, value: tuple[float, float]) -> None:
        self._obj.position = value

    @property
    def orientation(self) -> tuple[float, float]:
        """
        Current orientation

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.orientation

    @orientation.setter
    def orientation(self, value: tuple[float, float]) -> None:
        self._obj.orientation = value

    @property
    def journey_id(self) -> int:
        """
        Id of curently followed journey

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.journey_id

    @journey_id.setter
    def journey_id(self, value: int) -> None:
        self._obj.journey_id = value

    @property
    def stage_id(self) -> int:
        """
        Id of curently followed stage

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.stage_id

    @stage_id.setter
    def stage_id(self, value: int) -> None:
        self._obj.stage_id = value

    @property
    def mass(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.mass

    @mass.setter
    def mass(self, value: float) -> None:
        self._obj.mass = value

    @property
    def tau(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.tau

    @tau.setter
    def tau(self, value: float) -> None:
        self._obj.tau = value

    @property
    def v0(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.v0

    @v0.setter
    def v0(self, value: float) -> None:
        self._obj.v0 = value

    @property
    def a_v(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.a_v

    @a_v.setter
    def a_v(self, value: float) -> None:
        self._obj.a_v = value

    @property
    def a_min(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.a_min

    @a_min.setter
    def a_min(self, value: float) -> None:
        self._obj.a_min = value

    @property
    def b_min(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.b_min

    @b_min.setter
    def b_min(self, value: float) -> None:
        self._obj.b_min = value

    @property
    def b_max(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.b_max

    @b_max.setter
    def b_max(self, value: float) -> None:
        self._obj.b_max = value

    @property
    def id(self) -> int:
        """
        Id of this Agent

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.id

    @id.setter
    def id(self, value: int) -> None:
        self._obj.id = value

    def __str__(self) -> str:
        return self._obj.__repr__()


class VelocityModelAgentParameters:
    """
    Agent parameters for Velocity Model.

    See the scientifc publication for more details about this model
    https://arxiv.org/abs/1512.05597

    Objects of this type can be used to add new agents to the simulation and are
    returned by the simulation when inspecting agent state. Setting properties on
    objects returned by the simulation has no effect on the agents as this object
    is a copy of internal state.

    Setting properties on this object is only useful when adding multiple agents
    and they share many properties without reprating them on each 'add_agent'
    call
    """

    def __init__(self):
        self._obj = py_jps.VelocityModelAgentParameters()

    @property
    def position(self) -> tuple[float, float]:
        """
        Current position

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.position

    @position.setter
    def position(self, value: tuple[float, float]) -> None:
        self._obj.position = value

    @property
    def orientation(self) -> tuple[float, float]:
        """
        Current orientation

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.orientation

    @orientation.setter
    def orientation(self, value: tuple[float, float]) -> None:
        self._obj.orientation = value

    @property
    def journey_id(self) -> int:
        """
        Id of curently followed journey

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.journey_id

    @journey_id.setter
    def journey_id(self, value: int) -> None:
        self._obj.journey_id = value

    @property
    def stage_id(self) -> int:
        """
        Id of curently followed stage

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.stage_id

    @stage_id.setter
    def stage_id(self, value: int) -> None:
        self._obj.stage_id = value

    @property
    def time_gap(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.time_gap

    @time_gap.setter
    def time_gap(self, value: float) -> None:
        self._obj.time_gap = value

    @property
    def v0(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.v0

    @v0.setter
    def v0(self, value: float) -> None:
        self._obj.v0 = value

    @property
    def radius(self) -> float:
        """
        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.radius

    @radius.setter
    def radius(self, value: float) -> None:
        self._obj.radius = value

    @property
    def id(self) -> int:
        """
        Id of this Agent

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.id

    @id.setter
    def id(self, value: int) -> None:
        self._obj.id = value

    def __str__(self) -> str:
        return self._obj.__repr__()


class GeneralizedCentrifugalForceModelState:
    def __init__(self, backing):
        self._obj = backing

    @property
    def speed(self) -> float:
        return self._obj.speed

    @speed.setter
    def speed(self, speed):
        self._obj.speed = speed

    @property
    def e0(self) -> tuple[float, float]:
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
        return self._obj.v0

    @v0.setter
    def v0(self, v0):
        self._obj.v0 = v0

    @property
    def a_v(self) -> float:
        return self._obj.a_v

    @a_v.setter
    def a_v(self, a_v):
        self._obj.a_v = a_v

    @property
    def a_min(self) -> float:
        return self._obj.a_min

    @a_min.setter
    def a_min(self, a_min):
        self._obj.a_min = a_min

    @property
    def b_min(self) -> float:
        return self._obj.b_min

    @b_min.setter
    def b_min(self, b_min):
        self._obj.b_min = b_min

    @property
    def b_max(self) -> float:
        return self._obj.b_max

    @b_max.setter
    def b_max(self, b_max):
        self._obj.b_max = b_max


class VelocityModelState:
    def __init__(self, backing):
        self._obj = backing

    @property
    def e0(self) -> tuple[float, float]:
        return self._obj.e0

    @e0.setter
    def e0(self, e0):
        self._obj.e0 = e0

    @property
    def time_gap(self) -> float:
        return self._obj.time_gap

    @time_gap.setter
    def time_gap(self, time_gap):
        self._obj.time_gap = time_gap

    @property
    def tau(self) -> float:
        return self._obj.tau

    @tau.setter
    def tau(self, tau):
        self._obj.tau = tau

    @property
    def v0(self) -> float:
        return self._obj.v0

    @v0.setter
    def v0(self, v0):
        self._obj.v0 = v0

    @property
    def radius(self) -> float:
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius
