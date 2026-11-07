# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from typing import TYPE_CHECKING, Any

from jupedsim.internal.agents import (
    direct_steering,
    first_stage,
    native_location,
)
from jupedsim.pyjupedsim import SimulationError, _CustomModelState
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
    def stranded(self) -> bool:
        """True once the agent cannot continue, e.g. its exit is closed."""
        return self._resolve().stranded

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
        """Model specific state of this agent."""
        raw_state = self._resolve().state
        if isinstance(raw_state, _CustomModelState):
            return raw_state.model
        return _ModelStateHandle(self._simulation, self._id)

    def follow(self, route: Route, *, start: Waypoint | Exit | None = None):
        """Follow ``route`` from ``start`` or the route's default start."""
        self._resolve()
        stage = first_stage(self._simulation, route, start)
        self._simulation._obj.switch_agent_route(
            agent_id=self._id, route_id=route._native_id, stage_id=stage.id
        )

    def steer_to(self, location: _SurfaceLocation) -> None:
        """Leave the route and walk straight to ``location``."""
        self._resolve()
        native = native_location(self._simulation, location, "target")
        route_id, stage_id = direct_steering(self._simulation)
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
