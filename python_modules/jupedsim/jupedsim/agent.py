# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from typing import TYPE_CHECKING, Any

from jupedsim.pyjupedsim import Location, SimulationError, _CustomModelState
from jupedsim.surface import Location as _SurfaceLocation

if TYPE_CHECKING:
    from jupedsim.route import Route
    from jupedsim.simulation import Simulation
    from jupedsim.stages import Exit, Waypoint


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

        Model state handles are obtained via :attr:`Agent.state`.
        """
        object.__setattr__(self, "_ModelStateHandle__simulation", simulation)
        object.__setattr__(self, "_ModelStateHandle__id", agent_id)

    def __resolve(self):
        # Transient reference: only valid for the duration of one attribute
        # access, never stored.
        return self.__simulation._obj.agent(self.__id).state

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


class _AgentV1:
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

        agent.final_target = (1.0, 2.0)
        agent.state.desired_speed = 1.5

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
        """Id of the :class:`~jupedsim.JourneyDescription` the agent is currently following."""
        return self.__resolve().route_id

    @property
    def stage_id(self) -> int:
        """Id of the :class:`Stage` the Agent is currently targeting."""
        return self.__resolve().stage_id

    @property
    def position(self) -> tuple[float, float]:
        """Position of the agent."""
        return self.__resolve().position

    @property
    def location(self) -> Location:
        """Current location of the agent.

        Same as :attr:`position`, plus the region and the height of the surface.
        Can be used wherever the simulation accepts a location, e.g. to set
        another agent's :attr:`final_target`.
        """
        return self.__resolve().location

    @property
    def final_target(self) -> Location | None:
        """Current final target of the agent.

        Can be used to directly steer an agent towards the given coordinate.
        This will bypass the strategical and tactical level, but the
        operational level will still be active.

        .. important::

            If the agent is not in a Journey with a DirectSteering stage, any
            change will be ignored.

        .. important::

            When setting the target, the given coordinates must lie within the
            walkable area. Otherwise, an error will be thrown immediately.

        Accepts a :class:`~jupedsim.Location` or an ``(x, y)`` tuple. A
        tuple is placed on the surface closest to the agent's own height; to
        target another floor, pass a location from
        :meth:`~jupedsim.simulation.Simulation.get_location`.

        Returns:
            Current final target of the agent, or ``None`` while it heads
            for an area such as an exit.
        """
        return self.__resolve().final_target

    @final_target.setter
    def final_target(
        self, final_target: Location | tuple[float, float]
    ) -> None:
        self.__simulation._obj.set_agent_target(self.__id, final_target)

    @property
    def route_orientation(self) -> tuple[float, float]:
        """Unit vector along the route to the agent's final target.

        It is used by the operational model to compute the next state of the agent.
        Zero when the agent has already reached its final target.
        """
        return self.__resolve().route_orientation

    @property
    def state(self) -> Any:
        """Access model specific state of this agent.

        For built-in models this returns a state handle that resolves the
        agent on every attribute access, e.g.
        ``agent.state.desired_speed = 1.5``. For custom Python models this
        returns the user supplied state object.
        """
        raw_state = self.__resolve().state
        if isinstance(raw_state, _CustomModelState):
            return raw_state.model
        return _ModelStateHandle(self.__simulation, self.__id)

    def __repr__(self) -> str:
        return f"Agent(id={self.__id})"


class Agent:
    """Handle to an agent of a simulation built from a ``WalkableSurface``.

    Handles come from :meth:`~jupedsim.simulation.Simulation.add_agent`,
    :meth:`~jupedsim.simulation.Simulation.agent` and
    :meth:`~jupedsim.simulation.Simulation.agents`. A handle stores only the
    simulation and the id; every attribute is resolved freshly. Once the
    agent has left the simulation, ``id`` and ``alive`` stay readable and
    everything else raises :class:`RuntimeError`.
    """

    def __init__(self, simulation: Simulation, agent_id: int) -> None:
        """Do not use.

        Retrieve agent handles from the simulation.
        """
        self._simulation = simulation
        self._id = agent_id

    def _resolve(self):
        # Transient reference: only valid for the duration of one access.
        try:
            return self._simulation._obj.agent(self._id)
        except SimulationError as error:
            raise RuntimeError(
                f"Agent(id={self._id}) has left the simulation "
                "(agent.alive is False)"
            ) from error

    def _to_location(self, native: Any) -> _SurfaceLocation:
        region = self._simulation._surface.region(id=native.region_id)
        return _SurfaceLocation(native.x, native.y, region)

    @property
    def id(self) -> int:
        """Numeric id of the agent in this simulation."""
        return self._id

    @property
    def alive(self) -> bool:
        """False once the agent has left the simulation."""
        try:
            self._simulation._obj.agent(self._id)
        except SimulationError:
            return False
        return True

    @property
    def position(self) -> _SurfaceLocation:
        """Current location of the agent, a snapshot."""
        return self._to_location(self._resolve().location)

    @property
    def route(self) -> Route | None:
        """The route the agent follows, None in direct steering."""
        return self._simulation._routes.get(self._resolve().route_id)

    @route.setter
    def route(self, value: Any) -> None:
        raise AttributeError("use agent.follow(route, start=...)")

    @property
    def stage(self) -> Waypoint | Exit | None:
        """The stage the agent is heading for, None in direct steering."""
        native = self._resolve()
        if native.route_id not in self._simulation._routes:
            return None
        return self._simulation.stage(id=native.stage_id)

    @property
    def target(self) -> _SurfaceLocation | None:
        """The point the agent steers to, None while it follows a route."""
        native = self._resolve()
        if native.route_id in self._simulation._routes:
            return None
        target = native.final_target
        return None if target is None else self._to_location(target)

    @target.setter
    def target(self, value: Any) -> None:
        raise AttributeError("use agent.steer_to(location)")

    @property
    def route_orientation(self) -> tuple[float, float]:
        """Unit vector toward the current target, or (0, 0) at the destination."""
        return self._resolve().route_orientation

    @property
    def state(self) -> Any:
        """Model specific state of this agent, see :attr:`_AgentV1.state`."""
        raw_state = self._resolve().state
        if isinstance(raw_state, _CustomModelState):
            return raw_state.model
        return _ModelStateHandle(self._simulation, self._id)

    def follow(self, route: Route, *, start: Waypoint | Exit | None = None):
        """Follow ``route`` from ``start`` or the route's default start."""
        self._resolve()
        stage = self._simulation._first_stage(route, start)
        self._simulation._obj.switch_agent_route(
            agent_id=self._id, route_id=route._native_id, stage_id=stage.id
        )

    def steer_to(self, location: _SurfaceLocation) -> None:
        """Leave the route and walk straight to ``location``."""
        self._resolve()
        native = self._simulation._native_location(location, "target")
        route_id, stage_id = self._simulation._direct_steering()
        self._simulation._obj.switch_agent_route(
            agent_id=self._id, route_id=route_id, stage_id=stage_id
        )
        self._simulation._obj.set_agent_target(self._id, native)

    def remove(self) -> None:
        """Remove the agent at the start of the next iteration."""
        self._resolve()
        self._simulation._obj.mark_agent_for_removal(self._id)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Agent):
            return NotImplemented
        return self._simulation is other._simulation and self._id == other._id

    def __hash__(self) -> int:
        return hash((id(self._simulation), self._id))

    def __repr__(self) -> str:
        return f"Agent(id={self._id})"
