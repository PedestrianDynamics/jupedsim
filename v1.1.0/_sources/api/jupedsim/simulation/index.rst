:orphan:

:py:mod:`jupedsim.simulation`
=============================

.. py:module:: jupedsim.simulation


Module Contents
---------------

.. py:class:: Simulation(*, model: jupedsim.models.CollisionFreeSpeedModel | jupedsim.models.GeneralizedCentrifugalForceModel | jupedsim.models.CollisionFreeSpeedModelV2, geometry: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]], dt: float = 0.01, trajectory_writer: jupedsim.serialization.TrajectoryWriter | None = None, **kwargs: Any)


   Defines a simulation of pedestrian movement over a continuous walkable area.

   Movement of agents is described with Journeys, Stages and Transitions.
   Agents can be added and removed at will. The simulation processes one step
   at a time. No automatic stop condition exists. You can simulate multiple
   disconnected walkable areas by instantiating multiple instances of
   simulation.

   Creates a Simulation.

   :param model: Defines the operational model used in the simulation.
   :type model: CollisionFreeSpeedModel | GeneralizedCentrifugalForceModel | CollisionFreeSpeedModelv2
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

      Add a new waiting set stage to this simulation.

      :param positions: Ordered list of the waiting points of this waiting set.
                        The agents will fill the waiting points in the given order. If more agents
                        are targeting the waiting, the remaining will wait at the last given point.

      :returns: Id of the new stage.


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

      Add a journey to the simulation.

      :param journey: Description of the journey.

      :returns: Id of the added Journey.


   .. py:method:: add_agent(parameters: jupedsim.models.GeneralizedCentrifugalForceModelAgentParameters | jupedsim.models.CollisionFreeSpeedModelAgentParameters | jupedsim.models.CollisionFreeSpeedModelV2AgentParameters) -> int

      Add an agent to the simulation.

      :param parameters: Agent Parameters of the newly added model. The parameters have to
                         match the model used in this simulation. When adding agents with invalid parameters,
                         or too close to the boundary or other agents, this will cause an error.

      :returns: Id of the added agent.


   .. py:method:: mark_agent_for_removal(agent_id: int) -> bool

      Marks an agent for removal.

      Marks the given agent for removal in the simulation. The agent will be
      removed from the simulation in the start of the next :func:`iterate`
      call. The removal will take place before any interaction between
      agents will be computed.

      :param agent_id: Id of the agent marked for removal

      :returns: marking for removal was successful


   .. py:method:: removed_agents() -> list[int]

      All agents (given by Id) removed in the last iteration.

      All agents removed from the simulation since the last call of :func:`iterate`.
      These agents are can no longer be accessed.

      :returns: Ids of all removed agents since the last call of :func:`iterate`.


   .. py:method:: iterate(count: int = 1) -> None

      Advance the simulation by the given number of iterations.

      :param count: Number of iterations to advance


   .. py:method:: switch_agent_journey(agent_id: int, journey_id: int, stage_id: int) -> None

      Switch agent to the given journey at the given stage.

      :param agent_id: Id of the agent to switch
      :param journey_id: Id of the new journey to follow
      :param stage_id: Id of the stage in the new journey the agent continues with


   .. py:method:: agent_count() -> int

      Number of agents in the simulation.

      :returns: Number of agents in the simulation.


   .. py:method:: elapsed_time() -> float

      Elapsed time in seconds since the start of the simulation.

      :returns: Time in seconds since the start of the simulation.


   .. py:method:: delta_time() -> float

      Time step length in seconds of one iteration.

      :returns: Time step length of one iteration.


   .. py:method:: iteration_count() -> int

      Number of iterations performed since start of the simulation.

      :returns: Number of iterations performed.


   .. py:method:: agents() -> Iterable[jupedsim.agent.Agent]

      Agents in the simulation.

      :returns: Iterator over all agents in the simulation.


   .. py:method:: agent(agent_id) -> jupedsim.agent.Agent

      Access specific agent in the simulation.

      :param agent_id: Id of the agent to access

      :returns: Agent instance


   .. py:method:: agents_in_range(pos: tuple[float, float], distance: float) -> list[jupedsim.agent.Agent]

      Agents within the given distance to the given position.

      :param pos: point around which to search for agents
      :param distance: search radius

      :returns: List of agents within the given distance to the given position.


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

      Specific stage in the simulation.

      :param stage_id: Id of the stage to retrieve.

      :returns: The stage object.


   .. py:method:: set_tracing(status: bool) -> None


   .. py:method:: get_last_trace() -> jupedsim.tracing.Trace


   .. py:method:: get_geometry() -> jupedsim.geometry.Geometry

      Current geometry of the simulation.

      :returns: The geometry of the simulation.


   .. py:method:: switch_geometry(geometry: jupedsim.geometry.Geometry) -> None

      Switch the geometry of the simulation.

      Exchanges the current geometry with the new one. Checks if all agents
      and stages lie within the new geometry.

      :param geometry: The new geometry to be used in the simulation.



