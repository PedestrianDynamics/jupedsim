# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
"""try:
    import .py_jupedsim as py_jps
except ModuleNotFoundError:
    import py_jupedsim as py_jps"""
import py_jupedsim as py_jps


# TODO(kkratz): add typehints for function params
def set_debug_callback(fn) -> None:
    """
    Set reciever for debug messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_debug_callback(fn)


def set_info_callback(fn) -> None:
    """
    Set reciever for info messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_info_callback(fn)


def set_warning_callback(fn) -> None:
    """
    Set reciever for warning messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_warning_callback(fn)


def set_error_callback(fn) -> None:
    """
    Set reciever for error messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_error_callback(fn)


# TODO(kkratz): refactor this into free functions in C-API / bindings
class BuildInfo:
    def __init__(self):
        self.__obj = py_jps.get_build_info()

    @property
    def git_commit_hash(self) -> str:
        return self.__obj.git_commit_hash

    @property
    def git_commit_date(self) -> str:
        return self.__obj.git_commit_date

    @property
    def git_branch(self) -> str:
        return self.__obj.git_branch

    @property
    def compiler(self) -> str:
        return self.__obj.compiler

    @property
    def compiler_version(self) -> str:
        return self.__obj.compiler_version

    @property
    def library_version(self) -> str:
        return self.__obj.library_version


def get_build_info() -> BuildInfo:
    return BuildInfo()


class Trace:
    pass


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
        self.__obj = py_jps.GCFMModelAgentParameters()

    @property
    def speed(self) -> float:
        """
        Current speed

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.speed

    @speed.setter
    def speed(self, value: float) -> None:
        self.__obj.speed = value

    @property
    def e0(self) -> tuple[float, float]:
        """
        e0 (Currently desired orientation)

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.e0

    @e0.setter
    def e0(self, value: tuple[float, float]) -> None:
        self.__obj.e0 = value

    @property
    def position(self) -> tuple[float, float]:
        """
        Current position

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.position

    @position.setter
    def position(self, value: tuple[float, float]) -> None:
        self.__obj.position = value

    @property
    def orientation(self) -> tuple[float, float]:
        """
        Current orientation

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.orientation

    @orientation.setter
    def orientation(self, value: tuple[float, float]) -> None:
        self.__obj.orientation = value

    @property
    def journey_id(self) -> int:
        """
        Id of curently followed journey

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.journey_id

    @journey_id.setter
    def journey_id(self, value: int) -> None:
        self.__obj.journey_id = value

    @property
    def profile_id(self) -> int:
        """
        Id of curently used profile

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.profile_id

    @profile_id.setter
    def profile_id(self, value: int) -> None:
        self.__obj.profile_id = value

    @property
    def id(self) -> int:
        """
        Id of this Agent

        NOTE: Setting this property has no effect on agents that are already part of the simulation
        """
        return self.__obj.id

    @id.setter
    def id(self, value: int) -> None:
        self.__obj.id = value

    def __str__(self) -> str:
        return self.__obj.__repr__()


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
    def e0(self) -> float:
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


class Geometry:
    def __init__(self):
        self._geo = py_jps.Geometry()


class GeometryBuilder:
    def __init__(self):
        self.__obj = py_jps.GeometryBuilder()

    def add_accessible_area(self, polygon: list[tuple[float, float]]) -> None:
        self.__obj.add_accessible_area(polygon)

    # todo typo in bindings_jupedsim
    def exclude_from_accssible_area(self, polygon: list[tuple[float, float]]) -> None:
        self.__obj.exclude_from_accssible_area(polygon)

    def build(self):
        return self.__obj.build()


class VelocityModelBuilder:
    def __init__(self, a_ped: float, d_ped: float, a_wall: float, d_wall: float) -> None:
        self.__obj = py_jps.VelocityModelBuilder(a_ped=a_ped, d_ped=d_ped, a_wall=a_wall, d_wall=d_wall)

    def add_parameter_profile(self, id: int, time_gap: float, tau: float, v0: float, radius: float) -> None:
        self.__obj.add_parameter_profile(id=id, time_gap=time_gap, tau=tau, v0=v0, radius=radius)

    def build(self):
        return self.__obj.build()


class GCFMModelBuilder:
    def __init__(self,
                 nu_Ped: float, nu_Wall: float,
                 dist_eff_Ped: float, dist_eff_Wall: float,
                 intp_width_Ped: float, intp_width_Wall: float,
                 maxf_Ped: float, maxf_Wall: float) -> None:
        self.__obj = py_jps.GCFMModelBuilder(nu_Ped=nu_Ped, nu_Wall=nu_Wall,
                                             dist_eff_Ped=dist_eff_Ped, dist_eff_Wall=dist_eff_Wall,
                                             intp_width_Ped=intp_width_Ped, intp_width_Wall=intp_width_Wall,
                                             maxf_Ped=maxf_Ped, maxf_Wall=maxf_Wall)

    def add_parameter_profile(self, profile_id: int, mass: float, tau: float, v0: float, a_v: float, a_min: float,
                              b_min: float, b_max: float):
        return self.__obj.add_parameter_profile(id=profile_id, mass=mass, tau=tau, v0=v0, a_v=a_v, a_min=a_min,
                                                b_min=b_min, b_max=b_max)

    def build(self):
        return self.__obj.build()


class JourneyDescription:
    def __init__(self, stage_id: list[int] = None):
        if stage_id is None:
            self._obj = py_jps.JourneyDescription()
        else:
            self._obj = py_jps.JourneyDescription(stage_id)

    def append(self, stages: int | list[int]) -> None:
        self._obj.append(stages)


class Simulation:
    def __init__(self, model, geometry: Geometry, dt: float) -> None:
        self._sim = py_jps.Simulation(model=model, geometry=geometry, dt=dt)

    def add_waypoint_stage(self, position: tuple[float, float], distance) -> int:
        return self._sim.add_waypoint_stage(position, distance)

    def add_queue_stage(self, positions: list[tuple[float, float]]) -> int:
        return self._sim.add_queue_stage(positions)

    def add_waiting_set_stage(self, positions: list[tuple[float, float]]) -> int:
        return self._sim.add_waiting_set_stage(positions)

    def add_exit_stage(self, polygon: list[tuple[float, float]]) -> int:
        return self._sim.add_exit_stage(polygon)

    def add_journey(self, journey: JourneyDescription) -> int:
        return self._sim.add_journey(journey._obj)

    def add_agent(self, parameters: GCFMModelAgentParameters | VelocityModelAgentParameters) -> int:
        return self._sim.add_agent(parameters._obj)

    def remove_agent(self, agent_id: int) -> bool:
        return self._sim.remove_agent(agent_id)

    def read_agent(self, agent_id: int) -> GCFMModelAgentParameters | VelocityModelAgentParameters:
        return self._sim.read_agent(agent_id)

    def remove_agents(self) -> None:
        self._sim.remove_agents()

    def iterate(self, count: int = 1) -> None:
        self._sim.iterate(count)

    def switch_agent_profile(self, agentId: int, profileId: int) -> None:
        self._sim.switch_agent_profile(agentId=agentId, profileId=profileId)

    def switch_agent_journey(self, agentId: int,
                             journeyId: int, stageIdx: int) -> None:
        self._sim.switch_agent_journey(agentId=agentId, journeyId=journeyId, stageIdx=stageIdx)

    def agent_count(self) -> int:
        return self._sim.agent_count()

    def elapsed_time(self) -> float:
        return self._sim.elapsed_time()

    def delta_time(self) -> float:
        return self._sim.delta_time()

    def iteration_count(self) -> int:
        return self._sim.iteration_count()

    def agents(self):
        return self._sim.agents()

    def agents_in_range(self, pos: tuple[float, float], distance: float):
        return self._sim.agents_in_range(pos, distance)

    def agents_in_polygon(self, poly: list[tuple[float, float]]):
        return self._sim.agents_in_polygon(poly)

    def notify_waiting_set(self, stageId: int, active: bool) -> None:
        self._sim.notify_waiting_set(stageId, active)

    def notify_queue(self, stageId: int, count: int) -> None:
        self._sim.notify_queue(stageId, count)

    def set_tracing(self, status: bool) -> None:
        self._sim.set_tracing(status)

    def get_last_trace(self) -> Trace:
        return self._sim.get_last_trace()


class RoutingEngine:
    def __init__(self, geo: Geometry) -> None:
        self.__obj = py_jps.RoutingEngine(geo)

    def compute_waypoints(self, frm: tuple[float, float],
                          to: tuple[float, float]) -> list[tuple[float, float]]:
        return self.__obj.compute_waypoints(frm, to)

    def is_routable(self, p: tuple[float, float]) -> bool:
        return self.__obj.is_routable(p)

    def mesh(self) -> list[tuple[tuple[float, float], tuple[float, float], tuple[float, float]]]:
        return self.__obj.mesh()

    def edges_for(self, vertex_id: int):
        return self.__obj.edges_for(vertex_id)
