# SPDX-License-Identifier: LGPL-3.0-or-later

import types
from collections.abc import Sequence
from typing import Any, Iterator, Self, TypeVar

import numpy.typing as npt
import shapely

from jupedsim.agent import Agent
from jupedsim.custom_model import CustomOperationalModel
from jupedsim.internal.messages import id_and_name
from jupedsim.internal.tracing import Timer
from jupedsim.journey import JourneyDescription
from jupedsim.pyjupedsim import (
    AnticipationVelocityModel,
    AnticipationVelocityModelState,
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3,
    CollisionFreeSpeedModelV3State,
    DirectSteeringStage,
    ExitStage,
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
    WaypointStage,
    _CustomModelState,
    _PythonModel,
    _Simulation,
)
from jupedsim.serialization import TrajectoryWriter

_STATE_TYPES = (
    GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3State,
    AnticipationVelocityModelState,
    SocialForceModelState,
    WarpDriverModelState,
)

_Value = TypeVar("_Value")


class Simulation:
    """Defines a simulation of pedestrian movement over a continuous walkable area.

    Movement of agents is described with Journeys, Stages and Transitions. Agents can be added and
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
        self._writer = trajectory_writer
        if floorfield_options is None:
            floorfield_options = FloorfieldOptions()
        self._obj = _Simulation(
            model=the_model,
            geometry=geometry,
            dt=dt,
            floorfield_options=floorfield_options,
        )
        self._timer = Timer(self._obj, timer_log_level=timer_log_level)
        # Stages with a floor field: a default speed field change reaches those
        # without a speed field of their own in its region.
        self._routed_stages: list[int] = []
        # Stage id -> iteration from which on its travel times may differ from
        # the last ones handed out by _take_travel_time_changes.
        self._travel_time_changes: dict[int, int] = {}

    def add_waypoint_stage(
        self,
        areas: Sequence[
            tuple[shapely.Polygon | Sequence[tuple[float, float]], Region]
        ],
    ) -> int:
        """Add a waypoint stage to the simulation.

        Args:
            areas: Pairs of an area and the region it lies in. An area is a
                :class:`~shapely.Polygon` without holes or a list of 2D points
                describing its boundary. Only the part of an area inside its
                region belongs to the waypoint.

        Returns:
            Id of the new stage.

        Raises:
            ValueError: If an area has holes, or a region belongs to another geometry.
        """
        stage_id = self._obj.add_waypoint_stage(self._area_ids(areas))
        self._routed_stage_added(stage_id)
        return stage_id

    def add_exit_stage(
        self,
        areas: Sequence[
            tuple[shapely.Polygon | Sequence[tuple[float, float]], Region]
        ],
    ) -> int:
        """Add an exit stage to the simulation.

        Args:
            areas: Pairs of an area and the region it lies in. An area is a
                :class:`~shapely.Polygon` without holes or a list of 2D points
                describing its boundary. Only the part of an area inside its
                region belongs to the exit.

        Returns:
            Id of the added exit stage.

        Raises:
            ValueError: If an area has holes, or a region belongs to another geometry.
        """
        stage_id = self._obj.add_exit_stage(self._area_ids(areas))
        self._routed_stage_added(stage_id)
        return stage_id

    def add_direct_steering_stage(self) -> int:
        """Add an direct steering stage to the simulation.

        This stage allows a direct control of the target the agent is walking to. Thus, it will
        bypass the tactical and stragecial level of the simulation, but the operational level will
        still be active.

        .. important::

            A direct steering stage can only be used if it is the only stage in a Journey.

        Returns:
            Id of the added direct steering stage.
        """
        return self._obj.add_direct_steering_stage()

    def add_journey(self, journey: JourneyDescription) -> int:
        """Add a journey to the simulation.

        Args:
            journey: Description of the journey.

        Returns:
            Id of the added Journey.

        """
        return self._obj.add_journey(dict(journey._transitions))

    def add_agent(
        self,
        *,
        journey_id: int,
        stage_id: int,
        location: Location,
        state: (
            GeneralizedCentrifugalForceModelState
            | CollisionFreeSpeedModelState
            | CollisionFreeSpeedModelV2State
            | CollisionFreeSpeedModelV3State
            | AnticipationVelocityModelState
            | SocialForceModelState
            | WarpDriverModelState
            | Any
        ),
        name: str | None = None,
    ) -> int:
        """Add an agent to the simulation.

        Args:
            journey_id: Id of the journey the agent follows.
            stage_id: Id of the stage the agent initially targets.
            location: Where the agent stands, e.g. ``region.at(x, y)``.
            state: Initial per-agent model state. For built-in models this is the matching
                ``XModelState`` instance, e.g. :class:`~jupedsim.CollisionFreeSpeedModelState`. For
                custom models this is your own object, of whatever type your
                :class:`~jupedsim.CustomOperationalModel` expects. The state type has to match the
                model used in this simulation. When adding agents with invalid parameters, or too
                close to the boundary or other agents, this will cause an error.
            name: Unique among the agents of this simulation, if given; free again
                once its agent has left. Used in :meth:`agent`.

        Returns:
            Id of the added agent.

        Raises:
            ValueError: if the location belongs to another geometry.
            SimulationError: if the name is already used.
        """
        location = self._checked_location(location, "location")
        region_id = self._region_id(location.region)
        if not isinstance(state, _STATE_TYPES):
            state = _CustomModelState(state)
        return self._obj.add_agent(
            journey_id=journey_id,
            stage_id=stage_id,
            position=location.xy,
            state=state,
            region_id=region_id,
            name=name,
        )

    def travel_times(self, stage_id: int) -> dict[Region, TravelTimeGrid]:
        """Get the travel times to a stage's area over the floor field grid of every region.

        Every region has its own grid. Solves the stage's floor field if no agent has
        needed it yet.

        Args:
            stage_id: Id of the stage.

        Returns:
            The grid of every region, in ascending order of region ids, each with its
            placement, travel times in seconds and walking directions. Empty for a
            stage without an area to route to, such as a direct steering stage.

        Raises:
            SimulationError: if the stage does not exist.
        """
        return self._by_region(self._obj.travel_times(stage_id))

    def custom_speed_fields(
        self, stage_id: int
    ) -> dict[Region, SpeedGrid | None]:
        """Get the speed fields a stage has of its own, for every region.

        A stage can have its own speed field in a region instead of the default, see
        :func:`set_custom_speed_field`.

        Args:
            stage_id: Id of the stage whose speed fields to get.

        Returns:
            The grid of every region, in ascending order of region ids: ``None`` where
            the stage has no speed field of its own and uses the default, see
            :func:`default_speed_fields`. Empty for a stage without an area to route
            to, such as a direct steering stage.

        Raises:
            SimulationError: if the stage does not exist.
        """
        return self._by_region(self._obj.custom_speed_fields(stage_id))

    def has_custom_speed_field(self, stage_id: int, region: Region) -> bool:
        """Whether a stage has a speed field of its own in a region.

        Unlike :func:`custom_speed_fields`, this does not copy the speed fields.

        Args:
            stage_id: Id of the stage.
            region: The region.

        Returns:
            ``True`` if the stage uses its own speed field in this region, ``False`` if
            it uses the default. ``False`` for a stage without an area to route to, such
            as a direct steering stage, whatever ``region`` is.

        Raises:
            ValueError: if the region belongs to another geometry.
            SimulationError: if the stage does not exist.
        """
        return self._obj.has_custom_speed_field(
            stage_id, self._region_id(region)
        )

    def default_speed_fields(self) -> dict[Region, SpeedGrid]:
        """Get the default walking speeds the floor fields are computed with, for every region.

        Every region has a default speed field, which starts out slowing agents down
        near walls (see :class:`~jupedsim.FloorfieldOptions`). Every stage uses it in
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
            ValueError: if ``values`` has the wrong shape, or the region belongs to
                another geometry.
            SimulationError: if a value is not allowed.
        """
        region_id = self._region_id(region)
        self._obj.set_default_speed_field(region_id, values)
        for stage_id in self._routed_stages:
            if not self._obj.has_custom_speed_field(stage_id, region_id):
                self._travel_times_changed(stage_id)

    def set_custom_speed_field(
        self, stage_id: int, region: Region, values: npt.ArrayLike
    ) -> None:
        """Set the walking speeds one stage's floor field is computed with in a region.

        The stage then uses these speeds instead of the default in this region, for
        example where smoke only matters on the way to one exit. Other stages are not
        affected; the stage's floor field is computed again when next needed. Use
        :func:`clear_custom_speed_field` to go back to the default.

        The speeds only change the routes, not how fast agents walk.

        Args:
            stage_id: Stage whose speed field to set.
            region: Region whose speed field to set.
            values: Speeds in m/s, with the same rules as for
                :func:`set_default_speed_field`.

        Raises:
            ValueError: if ``values`` has the wrong shape, or the region belongs to
                another geometry.
            SimulationError: if a value is not allowed, the stage does not exist, or
                it has no area to route to.
        """
        self._obj.set_custom_speed_field(
            stage_id, self._region_id(region), values
        )
        self._travel_times_changed(stage_id)

    def clear_custom_speed_field(self, stage_id: int, region: Region) -> None:
        """Remove a stage's own speed field in a region, so it uses the default again.

        The stage's floor field is computed again when next needed. Nothing changes
        if the stage has no speed field of its own in this region.

        Args:
            stage_id: Stage whose speed field to remove.
            region: Region whose speed field to remove.

        Raises:
            ValueError: if the region belongs to another geometry.
            SimulationError: if the stage does not exist, or it has no area to route
                to.
        """
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
        """The stages whose travel times may have changed since the last call, each
        with the iteration from which on agents are routed with the new ones, by
        ascending stage id. For :class:`~jupedsim.Hdf5TrajectoryWriter`."""
        changes = dict(sorted(self._travel_time_changes.items()))
        self._travel_time_changes.clear()
        return changes

    def mark_agent_for_removal(self, agent_id: int):
        """Marks an agent for removal.

        Marks the given agent for removal in the simulation. The agent will be removed from the
        simulation in the start of the next :func:`iterate` call. The removal will take place before
        any interaction between agents will be computed.

        Args:
            agent_id: Id of the agent marked for removal
        """

        self._obj.mark_agent_for_removal(agent_id)

    def removed_agents(self) -> list[int]:
        """All agents (given by Id) removed in the last iteration.

        All agents removed from the simulation since the last call of :func:`iterate`. These agents
        are can no longer be accessed.

        Returns:
            Ids of all removed agents since the last call of :func:`iterate`.
        """
        return self._obj.removed_agents()

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

    def switch_agent_journey(
        self, agent_id: int, journey_id: int, stage_id: int
    ) -> None:
        """Switch agent to the given journey at the given stage.

        Args:
            agent_id: Id of the agent to switch
            journey_id: Id of the new journey to follow
            stage_id: Id of the stage in the new journey the agent continues with
        """
        self._obj.switch_agent_journey(
            agent_id=agent_id, journey_id=journey_id, stage_id=stage_id
        )

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

    def agents(self) -> Iterator[Agent]:
        """Agents in the simulation.

        The set of agents is snapshot when this method is called; agents added or removed afterwards
        are not reflected by the returned iterator.

        Returns:
            Iterator over handles to all agents in the simulation. The handles resolve the agent on
            every attribute access and stay valid across :func:`iterate` as long as the agent
            exists.
        """
        ids = [agent.id for agent in self._obj.agents()]
        return iter(Agent(self, agent_id) for agent_id in ids)

    def agent(self, key: int | str) -> Agent:
        """Look an agent up by id or by name.

        Returns:
            Handle to the agent. The handle resolves the agent on every attribute
            access and stays valid across :func:`iterate` as long as the agent exists.

        Raises:
            KeyError: if there is no such agent.
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

    def _region_id(self, region: object) -> int:
        """Id of ``region``, checked to be a :class:`~jupedsim.Region` of this
        simulation's geometry."""
        if not isinstance(region, Region):
            raise TypeError(
                f"region must be a Region, got {type(region).__name__}"
            )
        try:
            own = self.get_geometry().region(region.id)
        except KeyError:
            own = None
        if own != region:
            raise ValueError(
                f"Region {id_and_name(region)} belongs to another geometry"
            )
        return region.id

    def _area_ids(
        self,
        areas: Sequence[
            tuple[shapely.Polygon | Sequence[tuple[float, float]], Region]
        ],
    ) -> list[tuple[shapely.Polygon | Sequence[tuple[float, float]], int]]:
        return [(area, self._region_id(region)) for area, region in areas]

    def _by_region(self, by_id: dict[int, _Value]) -> dict[Region, _Value]:
        geometry = self.get_geometry()
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
        waypoint stage, so agents on a floor above or below are not included.

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

    def stage_ids(self) -> list[int]:
        """Get the ids of all stages.

        Returns:
            The ids of all stages in ascending order.
        """
        return self._obj.stage_ids()

    def get_stage(
        self, stage_id: int
    ) -> WaypointStage | ExitStage | DirectSteeringStage:
        """Get the stage with the given id.

        Args:
            stage_id: Id of the stage to retrieve.

        Returns:
            The stage object. It keeps the simulation alive.

        Raises:
            SimulationError: If no stage with this id exists.
        """
        return self._obj.get_stage(stage_id)

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
