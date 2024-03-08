# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Any, Iterable

import shapely

import jupedsim.native as py_jps
from jupedsim.agent import Agent
from jupedsim.geometry import Geometry
from jupedsim.geometry_utils import build_geometry
from jupedsim.journey import JourneyDescription
from jupedsim.models import (
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelAgentParameters,
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2AgentParameters,
    GeneralizedCentrifugalForceModel,
    GeneralizedCentrifugalForceModelAgentParameters,
)
from jupedsim.serialization import TrajectoryWriter
from jupedsim.stages import (
    ExitStage,
    NotifiableQueueStage,
    WaitingSetStage,
    WaypointStage,
)
from jupedsim.tracing import Trace


class Simulation:
    """Defines a simulation of pedestrian movement over a continuous walkable area.

    Movement of agents is described with Journeys, Stages and Transitions.
    Agents can be added and removed at will. The simulation processes one step
    at a time. No automatic stop condition exists. You can simulate multiple
    disconnected walkable areas by instantiating multiple instances of
    simulation.
    """

    def __init__(
        self,
        *,
        model: (
            CollisionFreeSpeedModel
            | GeneralizedCentrifugalForceModel
            | CollisionFreeSpeedModelV2
        ),
        geometry: (
            str
            | shapely.GeometryCollection
            | shapely.Polygon
            | shapely.MultiPolygon
            | shapely.MultiPoint
            | list[tuple[float, float]]
        ),
        dt: float = 0.01,
        trajectory_writer: TrajectoryWriter | None = None,
        **kwargs: Any,
    ) -> None:
        """Creates a Simulation.

        Arguments:
            model (CollisionFreeSpeedModel | GeneralizedCentrifugalForceModel | CollisionFreeSpeedModelv2):
                Defines the operational model used in the simulation.
            geometry:
                Data to create the geometry out of. Data may be supplied as:

                * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

                * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                * :class:`~shapely.MultiPolygon`

                * :class:`~shapely.Polygon`

                * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

            dt: Iteration step size in seconds. It is recommended to
                leave this at its default value.
            trajectory_writer: Any object implementing the
                TrajectoryWriter interface. JuPedSim provides a writer that outputs trajectory data
                in a sqlite database. If you want other formats such as CSV you need to provide
                your own custom implementation.

        Keyword Arguments:
            excluded_areas: describes exclusions
                from the walkable area. Only use this argument if `geometry` was
                provided as list[tuple[float, float]].
        """
        if isinstance(model, CollisionFreeSpeedModel):
            model_builder = py_jps.CollisionFreeSpeedModelBuilder(
                strength_neighbor_repulsion=model.strength_neighbor_repulsion,
                range_neighbor_repulsion=model.range_neighbor_repulsion,
                strength_geometry_repulsion=model.strength_geometry_repulsion,
                range_geometry_repulsion=model.range_geometry_repulsion,
            )
            py_jps_model = model_builder.build()
        elif isinstance(model, CollisionFreeSpeedModelV2):
            model_builder = py_jps.CollisionFreeSpeedModelV2Builder()
            py_jps_model = model_builder.build()
        elif isinstance(model, GeneralizedCentrifugalForceModel):
            model_builder = py_jps.GeneralizedCentrifugalForceModelBuilder(
                strength_neighbor_repulsion=model.strength_neighbor_repulsion,
                strength_geometry_repulsion=model.strength_geometry_repulsion,
                max_neighbor_interaction_distance=model.max_neighbor_interaction_distance,
                max_geometry_interaction_distance=model.max_geometry_interaction_distance,
                max_neighbor_interpolation_distance=model.max_neighbor_interpolation_distance,
                max_geometry_interpolation_distance=model.max_geometry_interpolation_distance,
                max_neighbor_repulsion_force=model.max_neighbor_repulsion_force,
                max_geometry_repulsion_force=model.max_geometry_repulsion_force,
            )
            py_jps_model = model_builder.build()
        else:
            raise Exception("Unknown model type supplied")
        self._writer = trajectory_writer
        self._obj = py_jps.Simulation(
            model=py_jps_model, geometry=build_geometry(geometry)._obj, dt=dt
        )

    def add_waypoint_stage(
        self, position: tuple[float, float], distance
    ) -> int:
        """Add a new waypoint stage to this simulation.

        Arguments:
            position: Position of the waypoint
            distance: Minimum distance required to reach this waypoint

        Returns:
            Id of the new stage.

        """
        return self._obj.add_waypoint_stage(position, distance)

    def add_queue_stage(self, positions: list[tuple[float, float]]) -> int:
        """Add a new queue state to this simulation.

        Arguments:
            positions: Ordered list of the waiting
                points of this queue. The first one in the list is the head of
                the queue while the last one is the back of the queue.

        Returns:
            Id of the new stage.

        """
        return self._obj.add_queue_stage(positions)

    def add_waiting_set_stage(
        self, positions: list[tuple[float, float]]
    ) -> int:
        """Add a new waiting set stage to this simulation.

        Arguments:
            positions: Ordered list of the waiting points of this waiting set.
                The agents will fill the waiting points in the given order. If more agents
                are targeting the waiting, the remaining will wait at the last given point.

        Returns:
            Id of the new stage.
        """
        return self._obj.add_waiting_set_stage(positions)

    def add_exit_stage(
        self,
        polygon: (
            str
            | shapely.GeometryCollection
            | shapely.Polygon
            | shapely.MultiPolygon
            | shapely.MultiPoint
            | list[tuple[float, float]]
        ),
    ) -> int:
        """Add an exit stage to the simulation.

        Arguments:
            polygon:
                Polygon without holes representing the exit stage. Polygon can be passed as:

                * list of 2d points describing the outer boundary

                * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                * :class:`~shapely.MultiPolygon`

                * :class:`~shapely.Polygon`

                * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

        Returns:
            Id of the added exit stage.

        """
        exit_geometry = build_geometry(polygon)
        return self._obj.add_exit_stage(exit_geometry.boundary())

    def add_journey(self, journey: JourneyDescription) -> int:
        """Add a journey to the simulation.

        Arguments:
            journey: Description of the journey.

        Returns:
            Id of the added Journey.

        """
        return self._obj.add_journey(journey._obj)

    def add_agent(
        self,
        parameters: (
            GeneralizedCentrifugalForceModelAgentParameters
            | CollisionFreeSpeedModelAgentParameters
            | CollisionFreeSpeedModelV2AgentParameters
        ),
    ) -> int:
        """Add an agent to the simulation.

        Arguments:
            parameters: Agent Parameters of the newly added model. The parameters have to
                match the model used in this simulation. When adding agents with invalid parameters,
                or too close to the boundary or other agents, this will cause an error.

        Returns:
            Id of the added agent.
        """
        return self._obj.add_agent(parameters.as_native())

    def mark_agent_for_removal(self, agent_id: int) -> bool:
        """Marks an agent for removal.

        Marks the given agent for removal in the simulation. The agent will be
        removed from the simulation in the start of the next :func:`iterate`
        call. The removal will take place before any interaction between
        agents will be computed.

        Arguments:
            agent_id: Id of the agent marked for removal

        Returns:
            marking for removal was successful
        """

        return self._obj.mark_agent_for_removal(agent_id)

    def removed_agents(self) -> list[int]:
        """All agents (given by Id) removed in the last iteration.

        All agents removed from the simulation since the last call of :func:`iterate`.
        These agents are can no longer be accessed.

        Returns:
            Ids of all removed agents since the last call of :func:`iterate`.
        """
        return self._obj.removed_agents()

    def iterate(self, count: int = 1) -> None:
        """Advance the simulation by the given number of iterations.

        Arguments:
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

        Arguments:
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

    def agents(self) -> Iterable[Agent]:
        """Agents in the simulation.

        Returns:
            Iterator over all agents in the simulation.
        """

        return self._obj.agents()

    def agent(self, agent_id) -> Agent:
        """Access specific agent in the simulation.

        Arguments:
            agent_id: Id of the agent to access

        Returns:
            Agent instance
        """
        return self._obj.agent(agent_id)

    def agents_in_range(
        self, pos: tuple[float, float], distance: float
    ) -> list[Agent]:
        """Agents within the given distance to the given position.

        Arguments:
             pos:  point around which to search for agents
             distance: search radius

        Returns:
            List of agents within the given distance to the given position.
        """
        return self._obj.agents_in_range(pos, distance)

    def agents_in_polygon(
        self,
        poly: (
            str
            | shapely.GeometryCollection
            | shapely.Polygon
            | shapely.MultiPolygon
            | shapely.MultiPoint
            | list[tuple[float, float]]
        ),
    ) -> list[Agent]:
        """Return all agents inside the given polygon.

        Args:
            poly:
                Polygon without holes in which to check for pedestrians. Polygon can be passed as:

                * list of 2d points describing the outer boundary

                * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                * :class:`~shapely.MultiPolygon`

                * :class:`~shapely.Polygon`

                * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

        Returns:
            All agents inside given polygon.

        """
        polygon_geometry = build_geometry(poly)

        return self._obj.agents_in_polygon(polygon_geometry.boundary())

    def get_stage(self, stage_id: int):
        """Specific stage in the simulation.

        Arguments:
            stage_id: Id of the stage to retrieve.

        Returns:
            The stage object.
        """
        stage = self._obj.get_stage_proxy(stage_id)
        match stage:
            case py_jps.WaypointProxy():
                return WaypointStage(stage)
            case py_jps.ExitProxy():
                return ExitStage(stage)
            case py_jps.NotifiableQueueProxy():
                return NotifiableQueueStage(stage)
            case py_jps.WaitingSetProxy():
                return WaitingSetStage(stage)
            case _:
                raise Exception(
                    f"Internal error, unexpected type: {type(stage)}"
                )

    def set_tracing(self, status: bool) -> None:
        self._obj.set_tracing(status)

    def get_last_trace(self) -> Trace:
        return self._obj.get_last_trace()

    def get_geometry(self) -> Geometry:
        """Current geometry of the simulation.

        Returns:
            The geometry of the simulation.
        """
        return Geometry(self._obj.get_geometry())

    def switch_geometry(self, geometry: Geometry) -> None:
        """Switch the geometry of the simulation.

        Exchanges the current geometry with the new one. Checks if all agents
        and stages lie within the new geometry.

        Arguments:
            geometry: The new geometry to be used in the simulation.

        """
        internal_geometry = build_geometry(geometry)
        self._obj.switch_geometry(internal_geometry._obj)
