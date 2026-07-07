# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from typing import TYPE_CHECKING, Any

import jupedsim.native as py_jps

if TYPE_CHECKING:
    from jupedsim.simulation import Simulation


class _ModelStateHandle:
    """Handle to the model specific state of one agent.

    Attribute reads and writes are forwarded to the agent's live model state
    inside the simulation. The agent is resolved freshly on every access;
    accessing the state of an agent that no longer exists raises
    :class:`~jupedsim.SimulationError`.

    Internals are private on purpose: no reference into the simulation's
    agent storage is ever retained beyond a single attribute access.
    """

    def __init__(self, simulation: Simulation, agent_id: int) -> None:
        """Do not use.

        Model state handles are obtained via :attr:`Agent.model`.
        """
        object.__setattr__(self, "_ModelStateHandle__simulation", simulation)
        object.__setattr__(self, "_ModelStateHandle__id", agent_id)

    def __resolve(self):
        # Transient reference: only valid for the duration of one attribute
        # access, never stored.
        return self.__simulation._obj.agent(self.__id).model

    def __getattr__(self, name: str) -> Any:
        if name.startswith("_"):
            raise AttributeError(name)
        return getattr(self.__resolve(), name)

    def __setattr__(self, name: str, value: Any) -> None:
        state = self.__resolve()
        if not hasattr(state, name):
            raise AttributeError(
                f"'{type(state).__name__}' has no attribute '{name}'"
            )
        setattr(state, name, value)

    def __dir__(self):
        return sorted(
            set(super().__dir__())
            | {n for n in dir(self.__resolve()) if not n.startswith("_")}
        )

    def __repr__(self) -> str:
        return f"ModelStateHandle(agent_id={self.__id})"


class Agent:
    """Handle to an agent in a :class:`~jupedsim.simulation.Simulation`.

    Agent handles are always retrieved from the simulation and never created
    directly:

    .. code:: python

        # a specific agent
        sim.agent(id)

        # all agents as iterator
        sim.agents()

        # agents in a specific distance to a point
        sim.agents_in_range(position, distance)

        # agents in a polygon
        sim.agents_in_polygon(polygon)

    A handle stores only the simulation and the agent id. Every attribute
    read or write resolves the agent freshly through the simulation, so
    handles remain valid across calls to
    :meth:`~jupedsim.simulation.Simulation.iterate`. Accessing a handle whose
    agent no longer exists raises :class:`~jupedsim.SimulationError`.

    Mutation through properties is supported, e.g.:

    .. code:: python

        agent.target = (1.0, 2.0)
        agent.model.desired_speed = 1.5

    .. note ::

        You need to be aware that currently there are no checks done when
        setting properties on an Agent instance. For example it is possible to
        set an Agent position outside the walkable area of the Simulation
        resulting in a crash.
    """

    def __init__(self, simulation: Simulation, agent_id: int) -> None:
        """Do not use.

        Retrieve agent handles from the simulation.
        """
        self.__simulation = simulation
        self.__id = agent_id

    def __resolve(self):
        # Transient reference: only valid for the duration of one property
        # call, never stored.
        return self.__simulation._obj.agent(self.__id)

    @property
    def id(self) -> int:
        """Numeric id of the agent in this simulation."""
        return self.__resolve().id

    @property
    def journey_id(self) -> int:
        """Id of the :class:`~jupedsim.journey.JourneyDescription` the agent is currently following."""
        return self.__resolve().journey_id

    @property
    def stage_id(self) -> int:
        """Id of the :class:`Stage` the Agent is currently targeting."""
        return self.__resolve().stage_id

    @property
    def position(self) -> tuple[float, float]:
        """Position of the agent."""
        return self.__resolve().position

    @property
    def target(self) -> tuple[float, float]:
        """Current target of the agent.

        Can be used to directly steer an agent towards the given coordinate.
        This will bypass the strategical and tactical level, but the
        operational level will still be active.

        .. important::

            If the agent is not in a Journey with a DirectSteering stage, any
            change will be ignored.

        .. important::

            When setting the target, the given coordinates must lie within the
            walkable area. Otherwise, an error will be thrown at the next
            iteration call.

        Returns:
            Current target of the agent.
        """
        return self.__resolve().target

    @target.setter
    def target(self, target: tuple[float, float]) -> None:
        self.__resolve().target = target

    @property
    def model(self) -> Any:
        """Access model specific state of this agent.

        For built-in models this returns a state handle that resolves the
        agent on every attribute access, e.g.
        ``agent.model.desired_speed = 1.5``. For custom Python models this
        returns the user supplied state object.
        """
        state = self.__resolve().model
        if isinstance(state, py_jps._CustomModelState):
            return state.model
        return _ModelStateHandle(self.__simulation, self.__id)

    def __repr__(self) -> str:
        return f"Agent(id={self.__id})"


class _TransientAgent:
    """Read-only view of an agent used inside custom-model callbacks.

    Wraps the native transient agent reference handed to
    :meth:`~jupedsim.models.custom_model.CustomOperationalModel.compute_next_state`
    and to neighborhood queries. Instances are only valid for the duration of
    the callback they were created in and must never be stored.
    """

    def __init__(self, backing) -> None:
        """Do not use.

        Transient agents are created by the simulation for the duration of a
        custom-model callback.
        """
        self.__obj = backing

    @property
    def id(self) -> int:
        """Numeric id of the agent in this simulation."""
        return self.__obj.id

    @property
    def journey_id(self) -> int:
        """Id of the journey the agent is currently following."""
        return self.__obj.journey_id

    @property
    def stage_id(self) -> int:
        """Id of the stage the agent is currently targeting."""
        return self.__obj.stage_id

    @property
    def position(self) -> tuple[float, float]:
        """Position of the agent."""
        return self.__obj.position

    @property
    def target(self) -> tuple[float, float]:
        """Current target of the agent."""
        return self.__obj.target

    @property
    def model(self) -> Any:
        """Model specific state of this agent.

        For custom models this is the user supplied state object. Treat it as
        read-only: state may only be changed by returning a new state object
        from ``compute_next_state``.
        """
        state = self.__obj.model
        if isinstance(state, py_jps._CustomModelState):
            return state.model
        return state

    def __repr__(self) -> str:
        return f"TransientAgent(id={self.__obj.id})"
