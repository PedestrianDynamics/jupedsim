# SPDX-License-Identifier: LGPL-3.0-or-later

import re
import types
from collections.abc import Sequence
from typing import Any, Self

import numpy.typing as npt
import shapely

from jupedsim.agent import Agent
from jupedsim.custom_model import CustomOperationalModel
from jupedsim.internal.agents import (
    agent_state,
    default_state_factory,
    direct_steering,
    first_stage,
    native_location,
)
from jupedsim.internal.tracing import Timer
from jupedsim.pyjupedsim import (
    AnticipationVelocityModel,
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV3,
    FloorfieldOptions,
    GeneralizedCentrifugalForceModel,
    Geometry,
    OperationalModel,
    SimulationError,
    SocialForceModel,
    SpeedGrid,
    TravelTimeGrid,
    WarpDriverModel,
    _PythonModel,
    _Simulation,
)
from jupedsim.route import Route, _require_stage
from jupedsim.serialization import TrajectoryWriter
from jupedsim.stages import Exit, Removal, Waypoint, _check_region, build_areas
from jupedsim.surface import Location, Region, WalkableSurface


def _is_int(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool)


class Simulation:
    """Defines a simulation of pedestrian movement over a continuous walkable area.

    Movement of agents is described with Routes made of Waypoints and Exits. Agents can be added and
    removed at will. The simulation processes one step at a time. No automatic stop condition
    exists. You can simulate multiple disconnected walkable areas by instantiating multiple
    instances of simulation.
    """

    def __init__(
        self,
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
        geometry: WalkableSurface,
        *,
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
                The walkable area: a :class:`~jupedsim.WalkableSurface` made of regions, which
                may be stacked into floors and joined by stairs or ramps.

            dt: Iteration step size in seconds. It is recommended to leave this at its default
                value.
            floorfield_options: Parameters of the floor fields used to route agents to their
                stage areas. Defaults to :class:`~jupedsim.FloorfieldOptions` with its default
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
        if not isinstance(geometry, WalkableSurface):
            raise TypeError(
                "geometry must be a WalkableSurface, got "
                f"{type(geometry).__name__}"
            )
        self._writer = trajectory_writer
        self._surface = geometry
        self._region_count = len(geometry.regions)
        self._stages: list[Waypoint | Exit] = []
        self._routes: dict[int, Route] = {}
        self._direct: tuple[int, int] | None = None
        self._default_state = default_state_factory(model)
        if floorfield_options is None:
            floorfield_options = FloorfieldOptions()
        self._obj = _Simulation(
            model=the_model,
            geometry=geometry._obj.create_geometry(),
            dt=dt,
            floorfield_options=floorfield_options,
        )
        self._timer = Timer(self._obj, timer_log_level=timer_log_level)

    def add_waypoint(
        self,
        area: Any,
        *,
        region: Region | None = None,
        radius: float | None = None,
        name: str | None = None,
    ) -> Waypoint:
        """Add a waypoint and return its handle.

        Args:
            area: A list of ``(area, region)`` pairs, a single polygon, or
                a point ``(x, y)`` together with ``radius``.
            region: Region of a single area or point.
            radius: Radius of the disc around a point.
            name: Optional name used in reprs and error messages.

        Raises:
            TypeError: If ``region`` is missing or not a Region handle.
            ValueError: If the area is invalid or does not overlap its
                region.
        """
        areas = self._build_areas(area, region, radius, "add_waypoint")
        stage_id = self._native_stage(self._obj.create_waypoint_stage, areas)
        handle = Waypoint(self, stage_id, name)
        self._stages.append(handle)
        return handle

    def add_exit(
        self,
        area: Any,
        *,
        region: Region | None = None,
        name: str | None = None,
    ) -> Exit:
        """Add an exit and return its handle.

        Args:
            area: A list of ``(area, region)`` pairs or a single polygon.
            region: Region of a single area.
            name: Optional name used in reprs and error messages.

        Raises:
            TypeError: If ``region`` is missing or not a Region handle.
            ValueError: If the area is invalid or does not overlap its
                region.
        """
        areas = self._build_areas(area, region, None, "add_exit")
        stage_id = self._native_stage(self._obj.create_exit_stage, areas)
        handle = Exit(self, stage_id, name)
        self._stages.append(handle)
        return handle

    @property
    def stages(self) -> tuple[Waypoint | Exit, ...]:
        """Waypoints and exits in creation order."""
        return tuple(self._stages)

    def stage(
        self, name: str | None = None, *, id: int | None = None
    ) -> Waypoint | Exit:
        """Look a stage up by name or by id.

        Raises:
            TypeError: If neither or both of name and id are given, or id
                is not an int.
            KeyError: If no stage has this name or id.
        """
        if (name is None) == (id is None):
            raise TypeError("give a name or an id, not both or neither")
        if id is not None and not _is_int(id):
            raise TypeError("id must be an int, e.g. sim.stage(id=wp.id)")
        for stage in self._stages:
            if (stage.id == id) if id is not None else (stage.name == name):
                return stage
        if id is not None:
            raise KeyError(f"no stage with id {id}")
        known = ", ".join(s.name or repr(s) for s in self._stages)
        raise KeyError(f"no stage named {name}; stages are {known}")

    def _native_stage(self, add: Any, areas: Any) -> int:
        regions = {r.id: r for r in self._surface.regions}
        try:
            return add(areas)
        except SimulationError as error:
            match = re.search(
                r"Area does not intersect region (\d+)", str(error)
            )
            if match is None:
                raise
            label = regions[int(match.group(1))].label
            raise ValueError(f"area does not overlap {label}") from error

    def _build_areas(
        self, area: Any, region: Any, radius: float | None, method: str
    ) -> list[tuple[shapely.Polygon, int]]:
        return build_areas(
            area,
            region=region,
            radius=radius,
            method=method,
            surface=self._surface,
            region_count=self._region_count,
        )

    def add_agent(
        self,
        position: Location | None = None,
        *,
        route: Route | None = None,
        target: Location | None = None,
        start: Waypoint | Exit | None = None,
        state: Any = None,
    ) -> Agent:
        """Add an agent to the simulation.

        With a :class:`~jupedsim.surface.Location` as ``position`` the agent
        follows ``route`` from ``start`` (default: the route's start) or
        steers straight to ``target``; give one of them. ``state`` is the
        agent's model state; built-in models default it, custom models must
        pass it. Returns the agent handle.

        Raises:
            TypeError: If ``position`` or ``target`` is not a Location.
            ValueError: If the arguments are inconsistent, a location or
                route belongs to another surface or simulation, or a
                custom model gets no ``state``.
        """
        native_location(self, position, "position")
        if route is not None and target is not None:
            raise ValueError("use route= or target=, not both")
        if route is None and target is None:
            raise ValueError("give route= or target=")
        if target is not None and start is not None:
            raise ValueError("start= applies to route=, not target=")
        native_target = None
        if target is not None:
            native_target = native_location(self, target, "target")
        stage = first_stage(self, route, start) if route is not None else None
        native_state = agent_state(self, state)
        native_route, native_stage = (
            (route._native_id, stage.id)
            if route is not None
            else direct_steering(self)
        )
        agent_id = self._obj.add_agent(
            route_id=native_route,
            stage_id=native_stage,
            position=position.xy,
            state=native_state,
            region_id=position.region.id,
        )
        if native_target is not None:
            self._obj.set_agent_target(agent_id, native_target)
        return Agent(self, agent_id)

    def _stage_native_id(self, stage: Any) -> int:
        _require_stage(stage)
        if stage._simulation is not self:
            raise ValueError("stage belongs to another simulation")
        return stage.id

    def _region_native_id(self, region: Any, method: str) -> int:
        _check_region(region, method, self._surface, self._region_count)
        return region.id

    def _by_region(self, grids: dict[int, Any]) -> dict[Region, Any]:
        return {self._surface.region(id=i): grid for i, grid in grids.items()}

    def travel_time(self, stage: Waypoint | Exit, location: Location) -> float:
        """Get the travel time in seconds from a location to a stage.

        The value at the nearest walkable floor field cell, not interpolated, so
        its resolution is the cell size. Solves the stage's floor field if no agent
        has needed it yet.

        Args:
            stage: The waypoint or exit to walk to.
            location: Where to start, e.g. ``floor0.at(x, y)``.

        Returns:
            0 inside the stage, ``inf`` if the stage cannot be reached from
            ``location``.

        Raises:
            TypeError: if ``location`` is not a Location.
            ValueError: if the stage belongs to another simulation, or the
                location to another surface.
            SimulationError: if the point is not on a routable cell.
        """
        stage_id = self._stage_native_id(stage)
        native = native_location(self, location, "location")
        return self._obj.travel_time(stage_id, native)

    def travel_times(
        self, stage: Waypoint | Exit
    ) -> dict[Region, TravelTimeGrid]:
        """Get the travel times to a stage's area over the floor field grid of every region.

        Every region has its own grid. Solves the stage's floor field if no agent has
        needed it yet.

        Args:
            stage: The waypoint or exit.

        Returns:
            The grid of every region by region handle, in ascending order, each with
            its placement, travel times in seconds and walking directions.

        Raises:
            ValueError: if the stage belongs to another simulation.
            SimulationError: if the stage does not exist.
        """
        return self._by_region(
            self._obj.travel_times(self._stage_native_id(stage))
        )

    def custom_speed_fields(
        self, stage: Waypoint | Exit
    ) -> dict[Region, SpeedGrid | None]:
        """Get the speed fields a stage has of its own, for every region.

        A stage can have its own speed field in a region instead of the default, see
        :func:`set_custom_speed_field`.

        Args:
            stage: The waypoint or exit whose speed fields to get.

        Returns:
            The grid of every region by region handle, in ascending order: ``None``
            where the stage has no speed field of its own and uses the default, see
            :func:`default_speed_fields`.

        Raises:
            ValueError: if the stage belongs to another simulation.
            SimulationError: if the stage does not exist.
        """
        return self._by_region(
            self._obj.custom_speed_fields(self._stage_native_id(stage))
        )

    def default_speed_fields(self) -> dict[Region, SpeedGrid]:
        """Get the default walking speeds the floor fields are computed with, for every region.

        Every region has a default speed field, which starts out slowing agents down
        near walls (see :class:`~jupedsim.FloorfieldOptions`). Every stage uses it in
        a region where it has no speed field of its own, see
        :func:`custom_speed_fields`.

        Returns:
            The grid of every region by region handle, in ascending order.
        """
        return self._by_region(self._obj.default_speed_fields())

    def set_default_speed_field(
        self, region: Region, values: npt.ArrayLike
    ) -> None:
        """Set the default walking speeds a region's floor fields are computed with.

        Use this to make agents route around slow areas, for example smoke: start
        from :func:`default_speed_fields`, change the values and set them again.
        Every stage without a speed field of its own in this region is affected; its
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
            TypeError: if ``region`` is not a Region.
            ValueError: if ``values`` has the wrong shape or the region belongs
                to another surface.
            SimulationError: if a value is not allowed or the region does not exist.
        """
        region_id = self._region_native_id(region, "set_default_speed_field")
        self._obj.set_default_speed_field(region_id, values)

    def set_custom_speed_field(
        self,
        stage: Waypoint | Exit,
        region: Region,
        values: npt.ArrayLike,
    ) -> None:
        """Set the walking speeds one stage's floor field is computed with in a region.

        The stage then uses these speeds instead of the default in this region, for
        example where smoke only matters on the way to one exit. Other stages are not
        affected; the stage's floor field is computed again when next needed. Use
        :func:`clear_custom_speed_field` to go back to the default.

        The speeds only change the routes, not how fast agents walk.

        Args:
            stage: Stage whose speed field to set.
            region: Region whose speed field to set.
            values: Speeds in m/s, with the same rules as for
                :func:`set_default_speed_field`.

        Raises:
            TypeError: if ``region`` is not a Region.
            ValueError: if ``values`` has the wrong shape, or the stage or region
                belongs to another simulation or surface.
            SimulationError: if a value is not allowed, the region or stage does not
                exist, or the stage has no area to route to.
        """
        stage_id = self._stage_native_id(stage)
        region_id = self._region_native_id(region, "set_custom_speed_field")
        self._obj.set_custom_speed_field(stage_id, region_id, values)

    def clear_custom_speed_field(
        self, stage: Waypoint | Exit, region: Region
    ) -> None:
        """Remove a stage's own speed field in a region, so it uses the default again.

        The stage's floor field is computed again when next needed. Nothing changes
        if the stage has no speed field of its own in this region.

        Args:
            stage: Stage whose speed field to remove.
            region: Region whose speed field to remove.

        Raises:
            TypeError: if ``region`` is not a Region.
            ValueError: if the stage or region belongs to another simulation or
                surface.
            SimulationError: if the region or stage does not exist, or the stage has
                no area to route to.
        """
        stage_id = self._stage_native_id(stage)
        region_id = self._region_native_id(region, "clear_custom_speed_field")
        self._obj.clear_custom_speed_field(stage_id, region_id)

    def mark_agent_for_removal(self, agent_id: int):
        """Marks an agent for removal.

        Marks the given agent for removal in the simulation. The agent will be removed from the
        simulation in the start of the next :func:`iterate` call. The removal will take place before
        any interaction between agents will be computed.

        Args:
            agent_id: Id of the agent marked for removal
        """

        self._obj.mark_agent_for_removal(agent_id)

    def removed_agents(self) -> tuple[Removal, ...]:
        """Records of the agents removed by the most recent :func:`iterate`.

        An agent that reaches an exit during an iteration, or is removed with
        :meth:`Agent.remove`, gets its record when that iteration ends. The
        agent itself leaves at the start of the next :func:`iterate`, so a
        record appears one iterate before ``agent.alive`` turns False. The
        next :func:`iterate` replaces the records; calls in between return
        equal tuples.

        Returns:
            One :class:`Removal` per agent: its id, the exit it left through
            (None for :meth:`Agent.remove`) and the time it reached that exit
            (for :meth:`Agent.remove`, the time of removal).
        """
        by_id = {stage.id: stage for stage in self._stages}
        return tuple(
            Removal(
                agent_id, None if stage_id is None else by_id[stage_id], time
            )
            for agent_id, stage_id, time in self._obj.removed_agents()
        )

    def iterate(self, count: int = 1) -> None:
        """Advance the simulation by the given number of iterations.

        Args:
            count: Number of iterations to advance
        """
        if self._writer and self.iteration_count() == 0:
            self._writer.begin_writing(self)
            self._writer.write_iteration_state(self)

        for _ in range(0, count):
            self._obj.iterate()
            if self._writer:
                self._writer.write_iteration_state(self)

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

    def agents(self) -> tuple[Agent, ...]:
        """Agents in the simulation.

        The set of agents is snapshot when this method is called; agents added or removed afterwards
        are not reflected by the result.

        Returns:
            A tuple of handles in id order. The handles resolve the agent on every attribute
            access and stay valid across :func:`iterate` as long as the agent exists.
        """
        ids = [agent.id for agent in self._obj.agents()]
        return tuple(Agent(self, agent_id) for agent_id in sorted(ids))

    def agents_targeting(
        self, stage: Waypoint | Exit, *, route: Route | None = None
    ) -> tuple[Agent, ...]:
        """Agents whose current target is ``stage``, in id order.

        With ``route``, only agents following that route. The result is a
        snapshot: calling :meth:`Agent.follow` while iterating over it does
        not change which agents are visited.

        Raises:
            ValueError: If the stage or route belongs to another simulation,
                or the stage is not part of the route.
        """
        _require_stage(stage)
        if stage._simulation is not self:
            raise ValueError("stage belongs to another simulation")
        if route is not None:
            route._check_simulation(self)
            route._check_member(stage)
        # Copy plain values: native agent wrappers must not outlive a loop step.
        found = [(a.id, a.route_id, a.stage_id) for a in self._obj.agents()]
        return tuple(
            Agent(self, agent_id)
            for agent_id, route_id, stage_id in sorted(found)
            if stage_id == stage.id
            and (route is None or route_id == route._native_id)
        )

    def agent(self, agent_id) -> Agent:
        """Access specific agent in the simulation.

        Args:
            agent_id: Id of the agent to access

        Returns:
            Handle to the agent. The handle resolves the agent on every attribute access and stays
            valid across :func:`iterate` as long as the agent exists.

        Raises:
            TypeError: if ``agent_id`` is not an int.
            SimulationError: if no agent with this id exists.
        """
        if not _is_int(agent_id):
            raise TypeError("agent_id must be an int, e.g. sim.agent(agent.id)")
        # Resolve once to fail fast on unknown ids.
        self._obj.agent(agent_id)
        return Agent(self, agent_id)

    def agents_in_range(
        self, pos: tuple[float, float], distance: float
    ) -> list[Agent]:
        """Handles to all agents within the given distance to the given position.

        Args:
             pos:  point around which to search for agents
             distance: search radius

        Returns:
            List of handles to all agents within the given distance to the given position.
        """
        return [
            Agent(self, agent_id)
            for agent_id in self._obj.agents_in_range(pos, distance)
        ]

    def agents_in_polygon(
        self,
        poly: shapely.Polygon | Sequence[tuple[float, float]],
    ) -> list[Agent]:
        """Handles to all agents inside the given polygon.

        Args:
            poly: Convex polygon in which to look for agents, as a
                :class:`~shapely.Polygon` without holes or as a list of 2D
                points describing its boundary.

        Returns:
            List of handles to all agents inside the given polygon.

        Raises:
            ValueError: If the polygon has holes.
        """
        return [
            Agent(self, agent_id)
            for agent_id in self._obj.agents_in_polygon(poly)
        ]

    def set_tracing(self, status: bool) -> None:
        self._obj.set_tracing(status)

    def get_geometry(self) -> Geometry:
        """Current geometry of the simulation.

        Returns:
            The geometry of the simulation.
        """
        return self._obj.get_geometry()

    @property
    def timer(self) -> Timer:
        """Timer for measuring time spent in different stages of the simulation.

        Returns:
            Timer object.
        """
        return self._timer
