# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from typing import TYPE_CHECKING, Any

from jupedsim.pyjupedsim import Location, _CustomModelState
from jupedsim.route import Route
from jupedsim.target import RouteTarget

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
    """Handle to an agent in a :class:`~jupedsim.simulation.Simulation`.

    Agent handles are always retrieved from the simulation and never created
    directly:

    .. code:: python

        # the agent just added
        agent = sim.add_agent(location, route=route)

        # a specific agent, by id or by name
        sim.agent(17)
        sim.agent("anna")

        # all agents as iterator
        sim.agents()

    A handle stores only the simulation and the agent id. Every attribute
    read or write resolves the agent freshly through the simulation, so
    handles remain valid across calls to
    :meth:`~jupedsim.simulation.Simulation.iterate`. Accessing a handle whose
    agent no longer exists raises :class:`~jupedsim.SimulationError`. Handles
    are equal if they refer to the same agent.

    The model state can be changed through the handle, e.g.:

    .. code:: python

        agent.state.desired_speed = 1.5
    """

    __slots__ = ("_simulation", "_id")

    def __init__(self, simulation: Simulation, agent_id: int) -> None:
        """Do not use.

        Retrieve agent handles from the simulation.
        """
        self._simulation = simulation
        self._id = agent_id

    def _resolve(self):
        # Transient reference: only valid for the duration of one property
        # call, never stored.
        return self._simulation._obj.agent(self._id)

    @property
    def id(self) -> int:
        """Numeric id of the agent in this simulation."""
        return self._id

    @property
    def name(self) -> str | None:
        """Name of the agent, if it was given one."""
        return self._simulation._obj.agent_name(self._id)

    @property
    def route(self) -> Route | None:
        """The route the agent follows, ``None`` while it is steered to a location."""
        route_id = self._resolve().route_id
        return None if route_id is None else Route(self._simulation, route_id)

    @property
    def current_target(self) -> RouteTarget | Location:
        """What the agent heads for now.

        The target on its route, or the location it is steered to, see
        :meth:`reroute`.
        """
        agent = self._resolve()
        if agent.stage_id is not None:
            return RouteTarget(self._simulation, agent.stage_id)
        return agent.final_target

    @property
    def location(self) -> Location:
        """Where the agent stands, with its region and height; a snapshot.

        Read it again to get the agent's current location.
        """
        return self._resolve().location

    @property
    def leaving(self) -> bool:
        """Whether the agent leaves the simulation at the beginning of the next
        iteration: it reached an exit, or was marked for removal."""
        return self._resolve().leaving

    @property
    def route_orientation(self) -> tuple[float, float]:
        """Unit vector toward the current target.

        It is used by the operational model to compute the next state of the agent.
        ``(0, 0)`` inside the area of the target.
        """
        return self._resolve().route_orientation

    @property
    def state(self) -> Any:
        """Access model specific state of this agent.

        For built-in models this returns a state handle that resolves the
        agent on every attribute access, e.g.
        ``agent.state.desired_speed = 1.5``. For custom Python models this
        returns the user supplied state object.
        """
        raw_state = self._resolve().state
        if isinstance(raw_state, _CustomModelState):
            return raw_state.model
        return _ModelStateHandle(self._simulation, self._id)

    def reroute(
        self,
        *,
        route: Route | None = None,
        first_target: RouteTarget | None = None,
        steer_to: Location | None = None,
    ) -> None:
        """Change where the agent goes, with the parameters of
        :meth:`~jupedsim.simulation.Simulation.add_agent`.

        .. code:: python

            agent.reroute(route=r)                  # join r, AGENT_LOCATION decides
            agent.reroute(route=r, first_target=t)  # join r at t
            agent.reroute(first_target=t)           # jump to t in the current route
            agent.reroute(steer_to=location)        # direct steering

        It takes effect in the next iteration and cannot be called while the
        simulation iterates. :attr:`current_target` returns the new target at once.
        The agent reaches it after the next step at the earliest, even if it stands
        in it already.

        Raises:
            ValueError: if the arguments do not form one of the calls above, or a
                handle belongs to another simulation.
            SimulationError: if ``first_target`` is not part of the route, the agent
                has no route to jump in, the location belongs to another geometry, or
                the agent is :attr:`leaving`.
        """
        sim = self._simulation
        if steer_to is not None:
            if route is not None or first_target is not None:
                raise ValueError(
                    "use route= and first_target=, or steer_to=, not both"
                )
            sim._obj.steer(
                self._id, sim._checked_location(steer_to, "steer_to")
            )
        elif route is not None:
            sim._obj.reroute(
                self._id,
                sim._own(route, Route, "route").id,
                None
                if first_target is None
                else sim._own(first_target, RouteTarget, "first_target").id,
            )
        elif first_target is not None:
            sim._obj.reroute_within_route(
                self._id, sim._own(first_target, RouteTarget, "first_target").id
            )
        else:
            raise ValueError("give route=, first_target= or steer_to=")

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, Agent)
            and other._simulation is self._simulation
            and other._id == self._id
        )

    def __hash__(self) -> int:
        return hash((id(self._simulation), self._id))

    def __repr__(self) -> str:
        name = self.name
        if name is None:
            return f"Agent(id={self._id})"
        return f"Agent(id={self._id}, name={name!r})"
