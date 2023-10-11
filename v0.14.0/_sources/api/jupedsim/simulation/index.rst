:py:mod:`jupedsim.simulation`
=============================

.. py:module:: jupedsim.simulation


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.simulation.Simulation




.. py:class:: Simulation(*, model, geometry, dt = 0.01, trajectory_writer = None, **kwargs)


   Defines a simulation of pedestrian movement over a continuous walkable area.

   Movement of agents is described with Journeys, Stages and Transitions.
   Agents can be added and removed at will. The simulation processes one step
   at a time. No automatic stop condition exists. You can simulate multiple
   disconnected walkable areas by instantiating multiple instances of
   simulation.

   Creates a Simulation.

   Arguments:
   model (CollisionFreeSpeedModel | GeneralizedCentrifugalForceModel):
           Defines the operational model used in the simulation.
       geometry (str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]):
           Data to create the geometry out of. Data may be supplied as:

           * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

           * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

           * :class:`~shapely.MultiPolygon`

           * :class:`~shapely.Polygon`

           * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

           * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

       dt (float): Iteration step size in seconds. It is recommended to
           leave this at its default value.
       trajectory_writer (TrajectoryWriter): Any object implementing the
           TrajectoryWriter interface. JuPedSim provides a writer that outputs trajectory data
           in a sqlite database. If you want other formats such as CSV you need to provide
           your own custom implementation.

   :keyword excluded_areas: describes exclusions
                            from the walkable area. Only use this argument if `geometry` was
                            provided as list[tuple[float, float]].
   :kwtype excluded_areas: list[list[tuple(float, float)]]

   .. py:method:: add_waypoint_stage(position, distance)

      Add a new waypoint stage to this simulation.

      :param position: Position of the waypoint
      :type position: tuple[float, float]
      :param distance: Minimum distance required to reach this waypoint
      :type distance: float

      :returns: Id of the new stage.


   .. py:method:: add_queue_stage(positions)

      Add a new queue state to this simulation.

      :param positions: Ordered list of the waiting
                        points of this queue. The first one in the list is the head of
                        the queue while the last one is the back of the queue.
      :type positions: list[tuple[float, float]]

      :returns: Id of the new stage.


   .. py:method:: add_waiting_set_stage(positions)


   .. py:method:: add_exit_stage(polygon)

      Add an exit stage to the simulation.

      :param polygon: Polygon without holes representing the exit stage. Polygon can be passed as:

                      * list of 2d points describing the outer boundary

                      * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                      * :class:`~shapely.MultiPolygon`

                      * :class:`~shapely.Polygon`

                      * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                      * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.
      :type polygon: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]

      :returns: Id of the added exit stage.


   .. py:method:: add_journey(journey)


   .. py:method:: add_agent(parameters)


   .. py:method:: mark_agent_for_removal(agent_id)

      Marks an agent for removal.

      Marks the given agent for removal in the simulation. The agent will be
      removed from the simulation in the start of the next :func:`iterate`
      call. The removal will take place before any interaction between
      agents will be computed.

      :param agent_id: Id of the agent marked for removal
      :type agent_id: int

      :returns: marking for removal was successful


   .. py:method:: removed_agents()


   .. py:method:: iterate(count = 1)


   .. py:method:: switch_agent_journey(agent_id, journey_id, stage_id)


   .. py:method:: agent_count()


   .. py:method:: elapsed_time()


   .. py:method:: delta_time()


   .. py:method:: iteration_count()


   .. py:method:: agents()


   .. py:method:: agent(agent_id)


   .. py:method:: agents_in_range(pos, distance)


   .. py:method:: agents_in_polygon(poly)

      Return all agents inside the given polygon.

      :param poly: Polygon without holes in which to check for pedestrians. Polygon can be passed as:

                   * list of 2d points describing the outer boundary

                   * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                   * :class:`~shapely.MultiPolygon`

                   * :class:`~shapely.Polygon`

                   * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                   * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.
      :type poly: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]

      :returns: All agents inside given polygon.


   .. py:method:: get_stage(stage_id)


   .. py:method:: set_tracing(status)


   .. py:method:: get_last_trace()


   .. py:method:: get_geometry()



