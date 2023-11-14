:orphan:

:py:mod:`jupedsim.simulation`
=============================

.. py:module:: jupedsim.simulation


Module Contents
---------------

.. py:class:: Simulation(*, model: jupedsim.models.CollisionFreeSpeedModel | jupedsim.models.GeneralizedCentrifugalForceModel, geometry: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]], dt: float = 0.01, trajectory_writer: jupedsim.serialization.TrajectoryWriter | None = None, **kwargs: Any)


   Defines a simulation of pedestrian movement over a continuous walkable area.

   Movement of agents is described with Journeys, Stages and Transitions.
   Agents can be added and removed at will. The simulation processes one step
   at a time. No automatic stop condition exists. You can simulate multiple
   disconnected walkable areas by instantiating multiple instances of
   simulation.

   Creates a Simulation.

   :param model: Defines the operational model used in the simulation.
   :type model: CollisionFreeSpeedModel | GeneralizedCentrifugalForceModel
   :param geometry: Data to create the geometry out of. Data may be supplied as:

                    * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

                    * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                    * :class:`~shapely.MultiPolygon`

                    * :class:`~shapely.Polygon`

                    * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                    * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.
   :param dt: Iteration step size in seconds. It is recommended to
              leave this at its default value.
   :param trajectory_writer: Any object implementing the
                             TrajectoryWriter interface. JuPedSim provides a writer that outputs trajectory data
                             in a sqlite database. If you want other formats such as CSV you need to provide
                             your own custom implementation.

   :keyword excluded_areas: describes exclusions
                            from the walkable area. Only use this argument if `geometry` was
                            provided as list[tuple[float, float]].

   .. py:method:: add_waypoint_stage(position: tuple[float, float], distance) -> int

      Add a new waypoint stage to this simulation.

      :param position: Position of the waypoint
      :param distance: Minimum distance required to reach this waypoint

      :returns: Id of the new stage.


   .. py:method:: add_queue_stage(positions: list[tuple[float, float]]) -> int

      Add a new queue state to this simulation.

      :param positions: Ordered list of the waiting
                        points of this queue. The first one in the list is the head of
                        the queue while the last one is the back of the queue.

      :returns: Id of the new stage.


   .. py:method:: add_waiting_set_stage(positions: list[tuple[float, float]]) -> int


   .. py:method:: add_exit_stage(polygon: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]) -> int

      Add an exit stage to the simulation.

      :param polygon: Polygon without holes representing the exit stage. Polygon can be passed as:

                      * list of 2d points describing the outer boundary

                      * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                      * :class:`~shapely.MultiPolygon`

                      * :class:`~shapely.Polygon`

                      * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                      * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

      :returns: Id of the added exit stage.


   .. py:method:: add_journey(journey: jupedsim.journey.JourneyDescription) -> int


   .. py:method:: add_agent(parameters: jupedsim.models.GeneralizedCentrifugalForceModelAgentParameters | jupedsim.models.CollisionFreeSpeedModelAgentParameters) -> int


   .. py:method:: mark_agent_for_removal(agent_id: int) -> bool

      Marks an agent for removal.

      Marks the given agent for removal in the simulation. The agent will be
      removed from the simulation in the start of the next :func:`iterate`
      call. The removal will take place before any interaction between
      agents will be computed.

      :param agent_id: Id of the agent marked for removal

      :returns: marking for removal was successful


   .. py:method:: removed_agents() -> list[int]


   .. py:method:: iterate(count: int = 1) -> None


   .. py:method:: switch_agent_journey(agent_id: int, journey_id: int, stage_id: int) -> None


   .. py:method:: agent_count() -> int


   .. py:method:: elapsed_time() -> float


   .. py:method:: delta_time() -> float


   .. py:method:: iteration_count() -> int


   .. py:method:: agents() -> Iterable[jupedsim.agent.Agent]


   .. py:method:: agent(agent_id) -> jupedsim.agent.Agent


   .. py:method:: agents_in_range(pos: tuple[float, float], distance: float) -> list[jupedsim.agent.Agent]


   .. py:method:: agents_in_polygon(poly: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]) -> list[jupedsim.agent.Agent]

      Return all agents inside the given polygon.

      :param poly: Polygon without holes in which to check for pedestrians. Polygon can be passed as:

                   * list of 2d points describing the outer boundary

                   * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                   * :class:`~shapely.MultiPolygon`

                   * :class:`~shapely.Polygon`

                   * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                   * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

      :returns: All agents inside given polygon.


   .. py:method:: get_stage(stage_id: int)


   .. py:method:: set_tracing(status: bool) -> None


   .. py:method:: get_last_trace() -> jupedsim.tracing.Trace


   .. py:method:: get_geometry() -> jupedsim.geometry.Geometry



