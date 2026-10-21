# SPDX-License-Identifier: LGPL-3.0-or-later

from collections.abc import Sequence
from typing import Any, Iterator

import shapely

from jupedsim.agent import Agent
from jupedsim.custom_model import CustomOperationalModel
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
    SocialForceModel,
    SocialForceModelState,
    TravelTimeGrid,
    WalkableSurface,
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
        geometry: WalkableSurface | Geometry,
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
                may be stacked into floors and joined by stairs or ramps, or a
                :class:`~jupedsim.Geometry` created from one.

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
        if isinstance(geometry, WalkableSurface):
            geometry = geometry.create_geometry()
        if floorfield_options is None:
            floorfield_options = FloorfieldOptions()
        self._obj = _Simulation(
            model=the_model,
            geometry=geometry,
            dt=dt,
            floorfield_options=floorfield_options,
        )
        self._timer = Timer(self._obj, timer_log_level=timer_log_level)

    def add_waypoint_stage(
        self,
        areas: Sequence[
            tuple[shapely.Polygon | Sequence[tuple[float, float]], int]
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
            ValueError: If an area has holes.
        """
        return self._obj.add_waypoint_stage(areas)

    def add_exit_stage(
        self,
        areas: Sequence[
            tuple[shapely.Polygon | Sequence[tuple[float, float]], int]
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
            ValueError: If an area has holes.
        """
        return self._obj.add_exit_stage(areas)

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
        position: tuple[float, float],
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
        region_id: int,
    ) -> int:
        """Add an agent to the simulation.

        Args:
            journey_id: Id of the journey the agent follows.
            stage_id: Id of the stage the agent initially targets.
            position: Position to spawn the agent at, as ``(x, y)`` in metres.
            region_id: Region the agent stands in, as returned by
                :meth:`~jupedsim.WalkableSurface.add_region` or
                :meth:`~jupedsim.WalkableSurface.connect_regions`.
            state: Initial per-agent model state. For built-in models this is the matching
                ``XModelState`` instance, e.g. :class:`~jupedsim.CollisionFreeSpeedModelState`. For
                custom models this is your own object, of whatever type your
                :class:`~jupedsim.CustomOperationalModel` expects. The state type has to match the
                model used in this simulation. When adding agents with invalid parameters, or too
                close to the boundary or other agents, this will cause an error.

        Returns:
            Id of the added agent.
        """
        if isinstance(state, _STATE_TYPES):
            return self._obj.add_agent(
                journey_id=journey_id,
                stage_id=stage_id,
                position=position,
                state=state,
                region_id=region_id,
            )
        return self._obj.add_agent(
            journey_id=journey_id,
            stage_id=stage_id,
            position=position,
            state=_CustomModelState(state),
            region_id=region_id,
        )

    def get_location(self, x: float, y: float, *, region_id: int) -> Location:
        """Get the location at ``(x, y)`` in region ``region_id``.

        The location stays valid as long as this simulation exists.

        Args:
            x: x coordinate in metres.
            y: y coordinate in metres.
            region_id: Region the place lies in, see :func:`add_agent`.

        Returns:
            The location.

        Raises:
            SimulationError: if ``region_id`` does not exist or ``(x, y)`` is not in it.
        """
        return self._obj.get_location(x, y, region_id)

    def travel_times(self, stage_id: int) -> dict[int, TravelTimeGrid]:
        """Get the travel times to a stage's area over the floor field grid of every region.

        Every region has its own grid. Solves the stage's floor field if no agent has
        needed it yet.

        Args:
            stage_id: Id of the stage.

        Returns:
            The grid of every region by region id, in ascending order, each with its
            placement, travel times in seconds and walking directions. Empty for a
            stage without an area to route to, such as a direct steering stage.

        Raises:
            SimulationError: if the stage does not exist.
        """
        return self._obj.travel_times(stage_id)

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

    def agent(self, agent_id) -> Agent:
        """Access specific agent in the simulation.

        Args:
            agent_id: Id of the agent to access

        Returns:
            Handle to the agent. The handle resolves the agent on every attribute access and stays
            valid across :func:`iterate` as long as the agent exists.

        Raises:
            SimulationError: if no agent with this id exists.
        """
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
