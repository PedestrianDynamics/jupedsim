# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import simulation.py_jupedsim as py_jps


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
    py_jps.set_warnign_callback(fn)


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
        self.__obj = py_jps.VelocityModelAgentParameters()

    @property
    def e0(self) -> float:
        """
        e0 (Currently desired direction)

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


class JPS_Simulation_Wrapper:
    def __init__(self, model: py_jps.JPS_OperationalModel_Wrapper, geometry: py_jps.JPS_Geometry_Wrapper,
                 dT: float) -> None:
        self.__obj = py_jps.JPS_Simulation_Wrapper(model, geometry, dT)

    def add_waypoint_stage(self, w: py_jps.JPS_Simulation_Wrapper, position: py_jps.JPS_Point,
                           distance) -> py_jps.JPS_StageId:
        return self.__obj.add_waypoint_stage(w, position, distance)

    def add_queue_stage(self, w: py_jps.JPS_Simulation_Wrapper, positions: list) -> py_jps.JPS_StageId:
        # todo position should be std::vector<JPS_Point> | is a list fine?
        return self.__obj.add_queue_stage(w, positions)

    def add_waiting_set_stage(self, w: py_jps.JPS_Simulation_Wrapper, positions: list) -> py_jps.JPS_StageId:
        return self.__obj.add_waiting_set_stage(w, positions)

    def add_exit_stage(self, w: py_jps.JPS_Simulation_Wrapper, polygon: list) -> py_jps.JPS_StageId:
        return self.__obj.add_exit_stage(w, polygon)

    def add_journey(self, simulation: py_jps.JPS_Simulation_Wrapper,
                    journey: py_jps.JPS_JourneyDescription_Wrapper) -> py_jps.JPS_JourneyId:
        return self.__obj.add_journey(simulation, journey)

    def add_agent(self, simulation: py_jps.JPS_Simulation_Wrapper,
                  parameters: py_jps.JPS_GCFMModelAgentParameters | py_jps.JPS_VelocityModelAgentParameters) -> py_jps.JPS_AgentId:
        return self.__obj.add_agent(simulation, parameters)

    def remove_agent(self, simulation: py_jps.JPS_Simulation_Wrapper, id: py_jps.JPS_AgentId) -> bool:
        return self.__obj.remove_agent(simulation, id)

    def read_agent(self, simulation: py_jps.JPS_Simulation_Wrapper,
                   id: py_jps.JPS_AgentId) -> py_jps.JPS_GCFMModelAgentParameters | py_jps.JPS_VelocityModelAgentParameters:
        return self.__obj.read_agent(simulation, id)

    def remove_agents(self, simulation: py_jps.JPS_Simulation_Wrapper) -> None:
        self.__obj.remove_agents(simulation)

    def iterate(self, simulation: py_jps.JPS_Simulation_Wrapper, count: int) -> None:
        # todo actually size_t | still fine?
        self.__obj.iterate(simulation, count)

    def switch_agent_profile(self, w: py_jps.JPS_Simulation_Wrapper, agentId: py_jps.JPS_AgentId,
                             profileId: py_jps.JPS_ModelParameterProfileId) -> None:
        self.__obj.switch_agent_profile(w, agentId, profileId)

    def switch_agent_journey(self, w: py_jps.JPS_Simulation_Wrapper, agentId: py_jps.JPS_AgentId,
                             journeyId: py_jps.JPS_JourneyId, stageIdx: py_jps.JPS_StageIndex) -> None:
        self.__obj.switch_agent_journey(w, agentId, journeyId, stageIdx)

    def agent_count(self, simulation: py_jps.JPS_Simulation_Wrapper) -> int:
        return self.__obj.agent_count(simulation)

    def elapsed_time(self, simulation: py_jps.JPS_Simulation_Wrapper) -> float:
        return self.__obj.elapsed_time(simulation)

    def delta_time(self, simulation: py_jps.JPS_Simulation_Wrapper) -> float:
        return self.__obj.delta_time(simulation)

    def iteration_count(self, simulation: py_jps.JPS_Simulation_Wrapper) -> int:
        return self.__obj.iteration_count(simulation)

    def agents(self, simulation: py_jps.JPS_Simulation_Wrapper):
        # todo return value is an Iterator | returns a list?
        return self.__obj.agents(simulation)

    def agents_in_range(self, w: py_jps.JPS_Simulation_Wrapper, pos: tuple[float, float], distance: float):
        return self.__obj.agents_in_range(w, pos, distance)

    def agents_in_polygon(self, w: py_jps.JPS_Simulation_Wrapper, poly: list[tuple[float, float]]):
        return self.__obj.agents_in_polygon(w, poly)

    def notify_waiting_set(self, w: py_jps.JPS_Simulation_Wrapper, stageId: py_jps.JPS_StageId, active: bool) -> None:
        self.__obj.notify_waiting_set(w, stageId, active)

    def notify_queue(self, w: py_jps.JPS_Simulation_Wrapper, stageId: py_jps.JPS_StageId, count: int) -> None:
        self.__obj.notify_queue(w, stageId, count)

    def set_tracing(self, w: py_jps.JPS_Simulation_Wrapper, status: bool) -> None:
        self.__obj.set_tracing(w, status)

    def get_last_trace(self, w: py_jps.JPS_Simulation_Wrapper) -> py_jps.JPS_Trace:
        return self.__obj.get_last_trace(w)
