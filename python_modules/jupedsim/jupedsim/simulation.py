# SPDX-License-Identifier: LGPL-3.0-or-later

import types
from collections.abc import Callable, Iterator, Sequence
from typing import Any, Self, TypeVar

import numpy.typing as npt
import shapely

from jupedsim.agent import Agent
from jupedsim.custom_model import CustomOperationalModel
from jupedsim.internal.messages import id_and_name
from jupedsim.internal.tracing import Timer
from jupedsim.pyjupedsim import (
    AnticipationVelocityModel,
    AnticipationVelocityModelState,
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3,
    CollisionFreeSpeedModelV3State,
    FloorfieldOptions,
    GeneralizedCentrifugalForceModel,
    GeneralizedCentrifugalForceModelState,
    Geometry,
    Location,
    OperationalModel,
    Region,
    SimulationError,
    SocialForceModel,
    SocialForceModelState,
    SpeedGrid,
    TravelTimeGrid,
    WarpDriverModel,
    WarpDriverModelState,
    _CustomModelState,
    _PythonModel,
    _Simulation,
)
from jupedsim.records import RemovalRecord, TargetReachedRecord
from jupedsim.route import AGENT_LOCATION, Route, Transition
from jupedsim.serialization import TrajectoryWriter
from jupedsim.target import Exit, RouteTarget, Waypoint

_STATE_TYPES = {
    GeneralizedCentrifugalForceModel: GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModel: CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2: CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3: CollisionFreeSpeedModelV3State,
    AnticipationVelocityModel: AnticipationVelocityModelState,
    SocialForceModel: SocialForceModelState,
    WarpDriverModel: WarpDriverModelState,
}

_Handle = TypeVar("_Handle", RouteTarget, Route, Agent)
_Value = TypeVar("_Value")


class Simulation:
    """Defines a simulation of pedestrian movement over a continuous walkable area.

    Agents walk along routes of targets, or are steered to a location. Agents can be added and
    removed at will. The simulation processes one step at a time. No automatic stop condition
    exists. You can simulate multiple disconnected walkable areas by instantiating multiple
    instances of simulation.
    """

    def __init__(
        self,
        *,
        model: (
            CollisionFreeSpeedModel
            | CollisionFreeSpeedModelV2
            | CollisionFreeSpeedModelV3
            | GeneralizedCentrifugalForceModel
            | SocialForceModel
            | AnticipationVelocityModel
            | WarpDriverModel
            | CustomOperationalModel
        ),
        geometry: Geometry,
        dt: float = 0.01,
        floorfield_options: FloorfieldOptions | None = None,
        trajectory_writer: TrajectoryWriter | None = None,
        timer_log_level: int = 1,
    ) -> None:
        """Creates a Simulation.

        Args:
            model:
                Defines the operational model used in the simulation. Every built-in model is passed
                as a configured instance carrying its model-level parameters, e.g.
                :class:`~jupedsim.CollisionFreeSpeedModel` or :class:`~jupedsim.SocialForceModel`.
                Custom Python models are passed as instances of a
                :class:`~jupedsim.CustomOperationalModel` subclass.

                .. warning::

                    Model instances are consumed by this constructor and must not be reused
                    afterwards.
            geometry:
                The walkable area, created with :meth:`~jupedsim.GeometryBuilder.build` from
                regions which may be stacked into floors and joined by stairs or ramps.

                The geometry is not modified and may be shared, e.g. by several simulations
                or with a :class:`~jupedsim.SurfaceMeshShortestPathRoutingEngine`.

            dt: Iteration step size in seconds. It is recommended to leave this at its default
                value.
            floorfield_options: Parameters of the floor fields used to route agents to their
                target areas. Defaults to :class:`~jupedsim.FloorfieldOptions` with its default
                values.
            trajectory_writer: Any object implementing the TrajectoryWriter interface. JuPedSim
                provides :class:`~jupedsim.Hdf5TrajectoryWriter`, which writes HDF5 files. If
                you want other formats such as CSV you need to provide your own custom
                implementation.
        """
        if isinstance(model, OperationalModel):
            the_model = model
        elif isinstance(model, CustomOperationalModel):
            the_model = _PythonModel(model)
        else:
            raise TypeError(
                "model must be a built-in operational model instance or a "
                "CustomOperationalModel instance, got "
                f"{type(model).__name__}"
            )
        self._writer = trajectory_writer
        self._default_state = _STATE_TYPES.get(type(model))
        if floorfield_options is None:
            floorfield_options = FloorfieldOptions()
        self._obj = _Simulation(
            model=the_model,
            geometry=geometry,
            dt=dt,
            floorfield_options=floorfield_options,
        )
        self._timer = Timer(self._obj, timer_log_level=timer_log_level)
        self._target_descriptions: dict[int, Waypoint | Exit] = {}
        self._reached_targets: list[TargetReachedRecord] = []
        # How many of the native records, which cover its most recent iteration and
        # the agents added since, are in _reached_targets.
        self._native_reached_seen = 0
        self._removals: list[RemovalRecord] = []
        # Stages with a floor field: a default speed field change reaches those
        # without a speed field of their own in its region.
        self._routed_stages: list[int] = []
        # Stage id -> iteration from which on its travel times may differ from
        # the last ones handed out by _take_travel_time_changes.
        self._travel_time_changes: dict[int, int] = {}

    def add_target(
        self, description: Waypoint | Exit, *, name: str | None = None
    ) -> RouteTarget:
        """Add a waypoint or an exit.

        Agents move toward the areas of a target along paths of least travel time on
        the floor field, see :meth:`travel_times`. A target is reached on the floor
        field grid, not on its polygon: arrival is off by up to half a cell.

        Floor fields are solved when first needed and cost memory per target and
        region, so create a target once and share it between routes.

        Args:
            description: The :class:`~jupedsim.Waypoint` or :class:`~jupedsim.Exit`.
            name: Unique among the targets of this simulation, if given. Used in
                reprs, error messages and :meth:`target`.

        Returns:
            Handle to the new target.

        Raises:
            ValueError: if an area has holes, or a region belongs to another geometry.
            SimulationError: if an area does not overlap its region, or the name is
                already used.
        """
        if not isinstance(description, Waypoint | Exit):
            raise TypeError(
                "description must be a jupedsim.Waypoint or jupedsim.Exit, got "
                f"{type(description).__name__}"
            )
        areas = [
            (area, self._region_id(region))
            for area, region in description.areas
        ]
        if isinstance(description, Waypoint):
            target_id = self._obj.add_waypoint_stage(areas, name)
        else:
            target_id = self._obj.add_exit_stage(areas, name)
        self._target_descriptions[target_id] = description
        self._routed_stage_added(target_id)
        return RouteTarget(self, target_id)

    def add_route(
        self, transitions: Sequence[Transition], *, name: str | None = None
    ) -> Route:
        """Add a route: a directed graph of targets, given by its transitions.

        The targets of the route are those in its transitions. The transition leaving
        :data:`~jupedsim.AGENT_LOCATION` chooses an agent's first target, once, when
        it joins the route. An agent reaching a target goes on to what the
        transition leaving that target chooses. A target without an outgoing
        transition ends the route; the agent stays in its area. Loops are allowed.

        Args:
            transitions: Exactly one leaves :data:`~jupedsim.AGENT_LOCATION`, at most
                one leaves each target, and all of them form one graph when their
                directions are ignored. A target that cannot be reached from
                :data:`~jupedsim.AGENT_LOCATION` but leads into the route is a side
                entry, see ``first_target`` of :meth:`add_agent`.
            name: Unique among the routes of this simulation, if given. Used in
                reprs, error messages and :meth:`route`.

        Returns:
            Handle to the new route.

        Raises:
            ValueError: if a target belongs to another simulation.
            SimulationError: if the transitions do not form a route as described,
                or the name is already used.
        """
        native = []
        for transition in transitions:
            for target in transition._targets():
                self._own(target, RouteTarget, "target")
            source = transition.source
            native.append(
                (
                    None
                    if source is AGENT_LOCATION
                    else self._own(source, RouteTarget, "source").id,
                    transition._description(),
                )
            )
        return Route(self, self._obj.add_route(native, name))

    def add_agent(
        self,
        location: Location,
        *,
        route: Route | None = None,
        first_target: RouteTarget | None = None,
        steer_to: Location | None = None,
        state: (
            GeneralizedCentrifugalForceModelState
            | CollisionFreeSpeedModelState
            | CollisionFreeSpeedModelV2State
            | CollisionFreeSpeedModelV3State
            | AnticipationVelocityModelState
            | SocialForceModelState
            | WarpDriverModelState
            | Any
        ) = None,
        name: str | None = None,
    ) -> Agent:
        """Add an agent to the simulation.

        Give either ``route`` or ``steer_to``. An agent placed inside its first target
        has reached it, see :meth:`reached_targets`, and moves on in the next
        iteration; inside an exit, it leaves at the beginning of the next iteration.

        Args:
            location: Where the agent stands, e.g. ``region.at(x, y)``.
            route: The route the agent follows.
            first_target: The first target the agent walks to, instead of the one the
                transition leaving :data:`~jupedsim.AGENT_LOCATION` chooses; the
                agent then takes no turn in that transition. Any target of the route.
                Later targets follow the route's transitions.
            steer_to: Steers the agent to this location instead of a route. The way
                there follows the walkable surface (the shortest path, not the floor
                field), so speed fields do not apply. Its arrival is reported once by
                :meth:`reached_targets`.
            state: Initial per-agent model state. For built-in models the matching
                ``XModelState``, e.g. :class:`~jupedsim.CollisionFreeSpeedModelState`;
                its defaults if not given. For custom models your own object, of
                whatever type your :class:`~jupedsim.CustomOperationalModel` expects;
                required. When adding agents with invalid parameters, or too close to
                the boundary or other agents, this will cause an error.
            name: Unique among the agents of this simulation, if given; free again
                once its agent has left. Used in :meth:`agent`.

        Returns:
            Handle to the new agent.

        Raises:
            TypeError: if ``location`` is not a :class:`~jupedsim.Location`.
            ValueError: if neither or both of ``route`` and ``steer_to`` are given,
                or a handle belongs to another simulation.
            SimulationError: if ``first_target`` is not part of the route, a location
                belongs to another geometry, or the name is already used.
        """
        location = self._checked_location(location, "location")
        if state is None:
            if self._default_state is None:
                raise ValueError(
                    "custom models need state=; pass your model's agent state"
                )
            state = self._default_state()
        elif not isinstance(state, tuple(_STATE_TYPES.values())):
            state = _CustomModelState(state)
        if (route is None) == (steer_to is None):
            raise ValueError("use route= or steer_to=, one of them")
        if steer_to is not None:
            if first_target is not None:
                raise ValueError(
                    "first_target= selects a target of route=; it does not apply "
                    "to steer_to="
                )
            agent_id = self._obj.add_steered_agent(
                location=location,
                steer_to=self._checked_location(steer_to, "steer_to"),
                state=state,
                name=name,
            )
        else:
            agent_id = self._obj.add_agent(
                location=location,
                route_id=self._own(route, Route, "route").id,
                first_stage_id=None
                if first_target is None
                else self._own(first_target, RouteTarget, "first_target").id,
                state=state,
                name=name,
            )
        self._collect_reached_targets()
        return Agent(self, agent_id)

    def travel_times(self, target: RouteTarget) -> dict[Region, TravelTimeGrid]:
        """Get the travel times to a target's areas over the floor field grid of every region.

        Every region has its own grid. Solves the target's floor field if no agent has
        needed it yet.

        Args:
            target: The target.

        Returns:
            The grid of every region, in ascending order of region ids, each with its
            placement, travel times in seconds and walking directions.
        """
        return self._by_region(
            self._obj.travel_times(self._own(target, RouteTarget, "target").id)
        )

    def travel_time(self, target: RouteTarget, location: Location) -> float:
        """Travel time in seconds from a location to a target.

        The value of the floor field cell nearest to ``location``, see
        :meth:`travel_times`: 0 inside the target, ``inf`` if it cannot be reached.

        Args:
            target: The target.
            location: Where to start, e.g. ``region.at(x, y)``.
        """
        return self._obj.travel_time(
            self._own(target, RouteTarget, "target").id,
            self._checked_location(location, "location"),
        )

    def custom_speed_fields(
        self, target: RouteTarget
    ) -> dict[Region, SpeedGrid | None]:
        """Get the speed fields a target has of its own, for every region.

        A target can have its own speed field in a region instead of the default, see
        :func:`set_custom_speed_field`.

        Args:
            target: The target whose speed fields to get.

        Returns:
            The grid of every region, in ascending order of region ids: ``None``
            where the target has no speed field of its own and uses the default, see
            :func:`default_speed_fields`.
        """
        return self._by_region(
            self._obj.custom_speed_fields(
                self._own(target, RouteTarget, "target").id
            )
        )

    def has_custom_speed_field(
        self, target: RouteTarget, region: Region
    ) -> bool:
        """Whether a target has a speed field of its own in a region.

        Unlike :func:`custom_speed_fields`, this does not copy the speed fields.

        Args:
            target: The target.
            region: The region.

        Returns:
            ``True`` if the target uses its own speed field in this region, ``False`` if
            it uses the default.
        """
        return self._obj.has_custom_speed_field(
            self._own(target, RouteTarget, "target").id, self._region_id(region)
        )

    def default_speed_fields(self) -> dict[Region, SpeedGrid]:
        """Get the default walking speeds the floor fields are computed with, for every region.

        Every region has a default speed field, which starts out slowing agents down
        near walls (see :class:`~jupedsim.FloorfieldOptions`). Every target uses it in
        a region where it has no speed field of its own, see
        :func:`custom_speed_fields`.

        Returns:
            The grid of every region, in ascending order of region ids.
        """
        return self._by_region(self._obj.default_speed_fields())

    def set_default_speed_field(
        self, region: Region, values: npt.ArrayLike
    ) -> None:
        """Set the default walking speeds a region's floor fields are computed with.

        Use this to make agents route around slow areas, for example smoke: start
        from :func:`default_speed_fields`, change the values and set them again.
        Every target without a speed field of its own in this region is affected; its
        floor field is computed again when next needed.

        The speeds only change the routes, not how fast agents walk.

        Args:
            region: Region whose default speed field to set.
            values: Speeds in m/s in an array of shape ``(height, width)`` of the
                region's grid, see :class:`~jupedsim.SpeedGrid`. Finite and at least
                0.01 m/s on every walkable cell: slower cells would make travel times
                infinite. Values on other cells are ignored, since which cells are
                walkable is fixed by the geometry.

        Raises:
            ValueError: if ``values`` has the wrong shape.
            SimulationError: if a value is not allowed.
        """
        region_id = self._region_id(region)
        self._obj.set_default_speed_field(region_id, values)
        for stage_id in self._routed_stages:
            if not self._obj.has_custom_speed_field(stage_id, region_id):
                self._travel_times_changed(stage_id)

    def set_custom_speed_field(
        self, target: RouteTarget, region: Region, values: npt.ArrayLike
    ) -> None:
        """Set the walking speeds one target's floor field is computed with in a region.

        The target then uses these speeds instead of the default in this region, for
        example where smoke only matters on the way to one exit. Other targets are not
        affected; the target's floor field is computed again when next needed. Use
        :func:`clear_custom_speed_field` to go back to the default.

        The speeds only change the routes, not how fast agents walk.

        Args:
            target: Target whose speed field to set.
            region: Region whose speed field to set.
            values: Speeds in m/s, with the same rules as for
                :func:`set_default_speed_field`.

        Raises:
            ValueError: if ``values`` has the wrong shape.
            SimulationError: if a value is not allowed.
        """
        stage_id = self._own(target, RouteTarget, "target").id
        self._obj.set_custom_speed_field(
            stage_id, self._region_id(region), values
        )
        self._travel_times_changed(stage_id)

    def clear_custom_speed_field(
        self, target: RouteTarget, region: Region
    ) -> None:
        """Remove a target's own speed field in a region, so it uses the default again.

        The target's floor field is computed again when next needed. Nothing changes
        if the target has no speed field of its own in this region.

        Args:
            target: Target whose speed field to remove.
            region: Region whose speed field to remove.
        """
        stage_id = self._own(target, RouteTarget, "target").id
        region_id = self._region_id(region)
        had_one = self._obj.has_custom_speed_field(stage_id, region_id)
        self._obj.clear_custom_speed_field(stage_id, region_id)
        if had_one:
            self._travel_times_changed(stage_id)

    def _routed_stage_added(self, stage_id: int) -> None:
        self._routed_stages.append(stage_id)
        self._travel_times_changed(stage_id)

    def _travel_times_changed(self, stage_id: int) -> None:
        self._travel_time_changes.setdefault(stage_id, self.iteration_count())

    def _take_travel_time_changes(self) -> dict[int, int]:
        """The targets whose travel times may have changed since the last call, each
        with the iteration from which on agents are routed with the new ones, by
        ascending target id. For :class:`~jupedsim.Hdf5TrajectoryWriter`."""
        changes = dict(sorted(self._travel_time_changes.items()))
        self._travel_time_changes.clear()
        return changes

    def mark_agent_for_removal(self, agent: Agent) -> None:
        """Remove an agent at the beginning of the next :meth:`iterate`.

        The removal takes place before agents interact. Nothing changes for an agent
        that is about to leave already.

        Args:
            agent: The agent to remove.
        """
        self._obj.mark_agent_for_removal(self._own(agent, Agent, "agent").id)

    def reached_targets(self) -> tuple[TargetReachedRecord, ...]:
        """The targets reached in the most recent :meth:`iterate` call, and by the agents
        added since.

        An agent reaches its target when it stands in one of its areas after a step, or
        when it is added there. One record per target assignment: revisiting a target in
        a loop gives another record; staying at the last target of a route gives none,
        and neither does being pushed out of it and returning. In direct steering, one
        record per ``steer_to`` assignment, once the agent is within 0.05 m of the
        location.

        An agent moves on at the beginning of the next iteration, by one target per step:
        if it stands in its next target already, it reaches that one after the step.
        After :meth:`~jupedsim.Agent.reroute`, too, it reaches its new target after the
        next step at the earliest.

        Records cover all iterations of the most recent ``iterate(count)`` call.
        Reading does not consume them; the next :meth:`iterate` replaces them. Scripts
        that react to them call :meth:`iterate` with ``count=1``, so that a
        :meth:`~jupedsim.Agent.reroute` takes effect in the step after.
        """
        return tuple(self._reached_targets)

    def removed_agents(self) -> tuple[RemovalRecord, ...]:
        """The agents removed in the most recent :meth:`iterate` call.

        An agent that reaches an exit, or is marked by :meth:`mark_agent_for_removal`,
        is removed at the beginning of the next iteration; until then
        :attr:`Agent.leaving <jupedsim.Agent.leaving>` is true. Covers the same
        iterations as :meth:`reached_targets`.
        """
        return tuple(self._removals)

    def iterate(self, count: int = 1) -> None:
        """Advance the simulation by the given number of iterations.

        Args:
            count: Number of iterations to advance
        """
        if self._writer and self.iteration_count() == 0:
            self._writer.begin_writing(self)
            self._writer.write_iteration_state(self)

        self._reached_targets.clear()
        self._removals.clear()
        for _ in range(0, count):
            self._obj.iterate()
            self._native_reached_seen = 0
            self._collect_reached_targets()
            for agent_id, exit_id, time in self._obj.removed_agents():
                self._removals.append(
                    RemovalRecord(
                        agent_id,
                        None if exit_id is None else RouteTarget(self, exit_id),
                        time,
                    )
                )
            if self._writer:
                self._writer.write_iteration_state(self)

    def _collect_reached_targets(self) -> None:
        for agent_id, target, time, location in self._obj.reached_targets(
            self._native_reached_seen
        ):
            if isinstance(target, int):
                target = RouteTarget(self, target)
            self._reached_targets.append(
                TargetReachedRecord(agent_id, target, time, location)
            )
            self._native_reached_seen += 1

    def close(self) -> None:
        """Close the simulation and free its memory.

        The simulation cannot be used afterwards: any further use raises a
        SimulationError.
        """
        try:
            if self._writer:
                self._writer.close()
        finally:
            self._obj.close()

    def __enter__(self) -> Self:
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: types.TracebackType | None,
    ) -> None:
        try:
            if exc_type is not None and self._writer:
                self._writer.close_after_error()
        finally:
            self.close()

    def agent_count(self) -> int:
        """Number of agents in the simulation.

        Returns:
            Number of agents in the simulation.
        """
        return self._obj.agent_count()

    def elapsed_time(self) -> float:
        """Elapsed time in seconds since the start of the simulation.

        Returns:
            Time in seconds since the start of the simulation.
        """
        return self._obj.elapsed_time()

    def delta_time(self) -> float:
        """Time step length in seconds of one iteration.

        Returns:
            Time step length of one iteration.
        """
        return self._obj.delta_time()

    def iteration_count(self) -> int:
        """Number of iterations performed since start of the simulation.

        Returns:
            Number of iterations performed.
        """
        return self._obj.iteration_count()

    def agents(
        self, *, filter: Callable[[Agent], bool] | None = None
    ) -> Iterator[Agent]:
        """Agents in the simulation.

        The set of agents is snapshot when this method is called; agents added or removed afterwards
        are not reflected by the returned iterator.

        .. code:: python

            for agent in sim.agents(
                filter=lambda a: a.route == route_c and a.current_target == wp2
            ):
                agent.reroute(first_target=wp3)

        Args:
            filter: Selects the agents for which it returns ``True``; all agents if not
                given.

        Returns:
            Iterator over handles to the agents. The handles resolve the agent on every
            attribute access and stay valid across :func:`iterate` as long as the agent
            exists.
        """
        ids = [agent.id for agent in self._obj.agents()]
        handles = (Agent(self, agent_id) for agent_id in ids)
        if filter is None:
            return handles
        return (agent for agent in handles if filter(agent))

    def agent(self, key: int | str) -> Agent:
        """Look an agent up by id or by name.

        Returns:
            Handle to the agent. The handle resolves the agent on every attribute
            access and stays valid across :func:`iterate` as long as the agent exists.

        Raises:
            KeyError: if there is no such agent, or no longer.
        """
        if isinstance(key, str):
            agent_id = self._obj.agent_named(key)
            if agent_id is None:
                raise KeyError(f"no agent named {key!r}")
            return Agent(self, agent_id)
        # The native lookup crashes on anything but an int.
        if isinstance(key, bool) or not isinstance(key, int):
            raise TypeError(
                f"key must be an agent id or name, got {type(key).__name__}"
            )
        # Fails on a closed simulation with its own error, unlike the lookup below.
        self._obj.agent_count()
        try:
            self._obj.agent(key)
        except SimulationError:
            raise KeyError(f"no agent with id {key}") from None
        return Agent(self, key)

    def target(self, key: int | str) -> RouteTarget:
        """Look a target up by id or by name.

        Raises:
            KeyError: if there is no such target.
        """
        return self._lookup(key, self.targets(), "target")

    def targets(self) -> list[RouteTarget]:
        """All targets, in the order they were added."""
        return [RouteTarget(self, i) for i in self._obj.stage_ids()]

    def route(self, key: int | str) -> Route:
        """Look a route up by id or by name.

        Raises:
            KeyError: if there is no such route.
        """
        return self._lookup(key, self.routes(), "route")

    def routes(self) -> list[Route]:
        """All routes, in the order they were added."""
        return [Route(self, i) for i in self._obj.route_ids()]

    def _lookup(
        self, key: int | str, handles: list[_Handle], kind: str
    ) -> _Handle:
        if isinstance(key, str):
            for handle in handles:
                if handle.name == key:
                    return handle
            raise KeyError(f"no {kind} named {key!r}")
        if isinstance(key, bool) or not isinstance(key, int):
            raise TypeError(
                f"key must be a {kind} id or name, got {type(key).__name__}"
            )
        for handle in handles:
            if handle.id == key:
                return handle
        raise KeyError(f"no {kind} with id {key}")

    def _own(self, handle: object, kind: type[_Handle], what: str) -> _Handle:
        """``handle``, checked to be a ``kind`` of this simulation."""
        if not isinstance(handle, kind):
            raise TypeError(
                f"{what} must be a {kind.__name__}, got {type(handle).__name__}"
            )
        if handle._simulation is not self:
            word = "Target" if kind is RouteTarget else kind.__name__
            raise ValueError(
                f"{word} {id_and_name(handle)} belongs to another simulation"
            )
        return handle

    def _region_id(self, region: object) -> int:
        """Id of ``region``, checked to be a :class:`~jupedsim.Region` of this
        simulation's geometry."""
        if not isinstance(region, Region):
            raise TypeError(
                f"region must be a Region, got {type(region).__name__}"
            )
        try:
            own = self.geometry.region(region.id)
        except KeyError:
            own = None
        if own != region:
            raise ValueError(
                f"Region {id_and_name(region)} belongs to another geometry"
            )
        return region.id

    def _by_region(self, by_id: dict[int, _Value]) -> dict[Region, _Value]:
        geometry = self.geometry
        return {geometry.region(region_id): v for region_id, v in by_id.items()}

    @staticmethod
    def _checked_location(location: object, what: str) -> Location:
        if not isinstance(location, Location):
            raise TypeError(
                f"{what} must be a Location, e.g. region.at(x, y), got "
                f"{type(location).__name__}"
            )
        return location

    def agents_in_range(
        self, location: Location, distance: float
    ) -> list[Agent]:
        """Handles to the agents near a location.

        The agents within ``distance`` of ``location`` that are its neighbors in the
        sense of the operational models: in plain sight over the walkable surface.
        Agents on a floor above or below, or behind a wall, are not.

        Args:
            location: Where to search around, e.g. ``region.at(x, y)``.
            distance: Search radius in metres.
        """
        return [
            Agent(self, agent_id)
            for agent_id in self._obj.agents_in_range(
                self._checked_location(location, "location"), distance
            )
        ]

    def agents_in_polygon(
        self,
        polygon: shapely.Polygon | Sequence[tuple[float, float]],
        *,
        region: Region,
    ) -> list[Agent]:
        """Handles to the agents inside a polygon in one region.

        Only the part of the polygon inside its region counts, as for the areas of a
        :class:`~jupedsim.Waypoint`, so agents on a floor above or below are not
        included.

        Args:
            polygon: A :class:`~shapely.Polygon` without holes or a sequence of at
                least three ``(x, y)`` points.
            region: The region the polygon lies in.

        Raises:
            ValueError: If the polygon has holes, or the region belongs to another
                geometry.
            SimulationError: If the polygon does not overlap the region.
        """
        return [
            Agent(self, agent_id)
            for agent_id in self._obj.agents_in_polygon(
                polygon, self._region_id(region)
            )
        ]

    def set_tracing(self, status: bool) -> None:
        self._obj.set_tracing(status)

    @property
    def geometry(self) -> Geometry:
        """The geometry of the simulation."""
        return self._obj.get_geometry()

    @property
    def timer(self) -> Timer:
        """Timer for measuring time spent in different stages of the simulation.

        Returns:
            Timer object.
        """
        return self._timer
