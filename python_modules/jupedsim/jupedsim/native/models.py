# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps


class VelocityModelBuilder:
    def __init__(
        self, a_ped: float, d_ped: float, a_wall: float, d_wall: float
    ) -> None:
        self._obj = py_jps.VelocityModelBuilder(
            a_ped=a_ped, d_ped=d_ped, a_wall=a_wall, d_wall=d_wall
        )

    def add_parameter_profile(
        self, id: int, time_gap: float, tau: float, v0: float, radius: float
    ) -> None:
        self._obj.add_parameter_profile(
            id=id, time_gap=time_gap, tau=tau, v0=v0, radius=radius
        )

    def build(self):
        return self._obj.build()


class GCFMModelBuilder:
    def __init__(
        self,
        nu_ped: float,
        nu_wall: float,
        dist_eff_ped: float,
        dist_eff_wall: float,
        intp_width_ped: float,
        intp_width_wall: float,
        maxf_ped: float,
        maxf_wall: float,
    ) -> None:
        self._obj = py_jps.GCFMModelBuilder(
            nu_ped=nu_ped,
            nu_wall=nu_wall,
            dist_eff_ped=dist_eff_ped,
            dist_eff_wall=dist_eff_wall,
            intp_width_ped=intp_width_ped,
            intp_width_wall=intp_width_wall,
            maxf_ped=maxf_ped,
            maxf_wall=maxf_wall,
        )

    def add_parameter_profile(
        self,
        profile_id: int,
        mass: float,
        tau: float,
        v0: float,
        a_v: float,
        a_min: float,
        b_min: float,
        b_max: float,
    ):
        return self._obj.add_parameter_profile(
            id=profile_id,
            mass=mass,
            tau=tau,
            v0=v0,
            a_v=a_v,
            a_min=a_min,
            b_min=b_min,
            b_max=b_max,
        )

    def build(self):
        return self._obj.build()


class GCFMModelAgentParameters:
    """
    Agent parameters for Generalized Centrifugal Model.

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
    def profile_id(self) -> int:
        """
        Id of curently used profile

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.profile_id

    @profile_id.setter
    def profile_id(self, value: int) -> None:
        self._obj.profile_id = value

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
    def e0(self) -> tuple[float, float]:
        """
        e0 (Currently desired direction)

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
    def profile_id(self) -> int:
        """
        Id of curently used profile

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self._obj.profile_id

    @profile_id.setter
    def profile_id(self, value: int) -> None:
        self._obj.profile_id = value

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

    @property
    def e0(self) -> tuple[float, float]:
        return self._obj.e0


class VelocityModelState:
    def __init__(self, backing):
        self._obj = backing

    @property
    def e0(self) -> tuple[float, float]:
        return self._obj.e0
