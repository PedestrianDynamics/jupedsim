:py:mod:`jupedsim`
==================

.. py:module:: jupedsim


Package Contents
----------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.Agent
   jupedsim.BuildInfo
   jupedsim.CustomModelAgentState
   jupedsim.CustomOperationalModel
   jupedsim.ExitStage
   jupedsim.Geometry
   jupedsim.JourneyDescription
   jupedsim.LineSegment
   jupedsim.NeighborhoodSearch
   jupedsim.NotifiableQueueStage
   jupedsim.Recording
   jupedsim.RecordingAgent
   jupedsim.RecordingFrame
   jupedsim.RoutingEngine
   jupedsim.Simulation
   jupedsim.SqliteTrajectoryWriter
   jupedsim.TrajectoryWriter
   jupedsim.Transition
   jupedsim.WaitingSetStage
   jupedsim.WaitingSetState
   jupedsim.WaypointStage



Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.distribute_by_density
   jupedsim.distribute_by_number
   jupedsim.distribute_by_percentage
   jupedsim.distribute_in_circles_by_density
   jupedsim.distribute_in_circles_by_number
   jupedsim.distribute_until_filled
   jupedsim.get_build_info
   jupedsim.set_debug_callback
   jupedsim.set_error_callback
   jupedsim.set_info_callback
   jupedsim.set_warning_callback



Attributes
~~~~~~~~~~

.. autoapisummary::

   jupedsim.AnticipationVelocityModel
   jupedsim.AnticipationVelocityModelState
   jupedsim.CollisionFreeSpeedModel
   jupedsim.CollisionFreeSpeedModelState
   jupedsim.CollisionFreeSpeedModelV2
   jupedsim.CollisionFreeSpeedModelV2State
   jupedsim.CollisionFreeSpeedModelV3
   jupedsim.CollisionFreeSpeedModelV3State
   jupedsim.GeneralizedCentrifugalForceModel
   jupedsim.GeneralizedCentrifugalForceModelState
   jupedsim.Hdf5TrajectoryWriter
   jupedsim.SimulationError
   jupedsim.SocialForceModel
   jupedsim.SocialForceModelState
   jupedsim.WarpDriverModel
   jupedsim.WarpDriverModelState
   jupedsim.__commit__
   jupedsim.__compiler__
   jupedsim.__version__


.. py:exception:: AgentNumberError(message)

   Bases: :py:obj:`Exception`


   Common base class for all non-exit exceptions.


   .. py:attribute:: message


.. py:exception:: IncorrectParameterError(message)

   Bases: :py:obj:`Exception`


   Common base class for all non-exit exceptions.


   .. py:attribute:: message


.. py:exception:: NegativeValueError(message)

   Bases: :py:obj:`Exception`


   Common base class for all non-exit exceptions.


   .. py:attribute:: message


.. py:exception:: OverlappingCirclesError(message)

   Bases: :py:obj:`Exception`


   Common base class for all non-exit exceptions.


   .. py:attribute:: message


.. py:class:: Agent(simulation: jupedsim.simulation.Simulation, agent_id: int)

   Handle to an agent in a :class:`~jupedsim.simulation.Simulation`.

   Agent handles are always retrieved from the simulation and never created
   directly:

   .. code:: python

       # a specific agent
       sim.agent(id)

       # all agents as iterator
       sim.agents()

       # agents in a specific distance to a point
       sim.agents_in_range(position, distance)

       # agents in a polygon
       sim.agents_in_polygon(polygon)

   A handle stores only the simulation and the agent id. Every attribute
   read or write resolves the agent freshly through the simulation, so
   handles remain valid across calls to
   :meth:`~jupedsim.simulation.Simulation.iterate`. Accessing a handle whose
   agent no longer exists raises :class:`~jupedsim.SimulationError`.

   Mutation through properties is supported, e.g.:

   .. code:: python

       agent.end_destination = (1.0, 2.0)
       agent.model.desired_speed = 1.5

   .. note ::

       You need to be aware that currently there are no checks done when
       setting properties on an Agent instance. For example it is possible to
       set an Agent position outside the walkable area of the Simulation
       resulting in a crash.


   .. py:property:: end_target
      :type: tuple[float, float]


      Current end target of the agent.

      Can be used to directly steer an agent towards the given coordinate.
      This will bypass the strategical and tactical level, but the
      operational level will still be active.

      .. important::

          If the agent is not in a Journey with a DirectSteering stage, any
          change will be ignored.

      .. important::

          When setting the target, the given coordinates must lie within the
          walkable area. Otherwise, an error will be thrown at the next
          iteration call.

      :returns: Current end target of the agent.


   .. py:property:: id
      :type: int


      Numeric id of the agent in this simulation.


   .. py:property:: journey_id
      :type: int


      Id of the :class:`~jupedsim.journey.JourneyDescription` the agent is currently following.


   .. py:property:: model
      :type: Any


      Access model specific state of this agent.

      For built-in models this returns a state handle that resolves the
      agent on every attribute access, e.g.
      ``agent.model.desired_speed = 1.5``. For custom Python models this
      returns the user supplied state object.


   .. py:property:: next_target
      :type: tuple[float, float]


      Current next target of the agent.

      Next destination is the next waypoint of the current stage of the agent's journey.
      It is used by the operational model to compute the next state of the agent.

      :returns: Current next destination of the agent.


   .. py:property:: position
      :type: tuple[float, float]


      Position of the agent.


   .. py:property:: stage_id
      :type: int


      Id of the :class:`Stage` the Agent is currently targeting.


.. py:class:: BuildInfo

   .. py:property:: compiler
      :type: str


      Compiler the native code was compiled with.

      :returns: Compiler identification.


   .. py:property:: compiler_version
      :type: str


      Compiler version the native code was compiled with.

      :returns: Compiler version number.


   .. py:property:: git_branch
      :type: str


      Branch this commit was crated from.

      :returns: name of the branch this version was build from.


   .. py:property:: git_commit_date
      :type: str


      Date this commit was created.

      :returns: Date the commit of this version as string.


   .. py:property:: git_commit_hash
      :type: str


      SHA1 commit hash of this version.

      :returns: SHA1 of this version.


   .. py:property:: library_version
      :type: str



.. py:class:: CustomModelAgentState

   Bases: :py:obj:`Protocol`


   Structural interface for per-agent model state of custom models.

   Any object exposing a ``position`` attribute of type ``tuple[float, float]``
   satisfies this protocol -- explicitly subclassing it is supported but not
   required. The runtime check performed when adding an agent verifies
   attribute presence only; value types are validated by the simulation
   itself.

   Objects satisfying this protocol are passed directly to
   :meth:`~jupedsim.simulation.Simulation.add_agent` as the agent's initial
   state. It should be an immutable object -- a ``@dataclass(frozen=True)``
   is strongly recommended -- because the simulation shares it live with your
   model during each step (see :class:`CustomOperationalModel`).


   .. py:attribute:: position
      :type:  tuple[float, float]


.. py:class:: CustomOperationalModel

   Bases: :py:obj:`abc.ABC`


   Base class for operational models implemented in Python.

   Subclasses implement :meth:`compute_next_state` and optionally
   :meth:`check_model_constraint`. Constraint violations should be reported by
   raising an exception.

   .. warning::

       **Per-agent model state is live and shared -- never mutate it in place.**

       The ``ped.model`` object you receive (and every neighbor's ``.model``
       returned from a neighborhood query) is the agent's *live* state, shared
       by reference with the running simulation for performance. JuPedSim
       advances agents in two phases per step: it first *computes* every
       agent's update from the current state of all agents, then *applies* all
       updates together. Mutating ``ped.model`` (or a neighbor's) during the
       compute phase changes state that other agents are still reading in the
       same step, silently breaking the compute-then-apply ordering and
       producing order-dependent results.

       The only correct way to change state is to return a new state object
       from :meth:`compute_next_state` -- returning ``ped.model`` itself
       (even unchanged) raises an error; use
       ``dataclasses.replace(ped.model, ...)``. Make your state type
       immutable -- a ``@dataclass(frozen=True)`` -- so accidental in-place
       writes raise immediately instead of silently corrupting the
       simulation.

   .. warning::

       The ``ped`` object passed to the callbacks (and the neighbor objects
       returned from neighborhood queries) are transient views that are only
       valid for the duration of the callback. Never store them. Calling
       mutating methods on the simulation (``add_agent``,
       ``mark_agent_for_removal``, journey or stage mutation) from within a
       callback raises :class:`~jupedsim.SimulationError`.


   .. py:method:: check_model_constraint(ped: jupedsim.agent._TransientAgent, neighborhood_search: jupedsim.neighborhood.NeighborhoodSearch, geometry: jupedsim.geometry.Geometry) -> None

      Raise an exception when ``ped`` violates this model's constraints.



   .. py:method:: compute_next_state(dt: float, ped: jupedsim.agent._TransientAgent, geometry: jupedsim.geometry.Geometry, neighborhood_search: jupedsim.neighborhood.NeighborhoodSearch) -> CustomModelAgentState
      :abstractmethod:


      Compute one update for ``ped``.



.. py:class:: ExitStage(backing)

   Models an exit.

   Agents entering the polygon defining the exit will be removed at the
   beginning of the next iteration, i.e. agents will be inside the specified
   polygon for one frame.


   .. py:method:: count_targeting()

      :returns: Number of agents currently targeting this stage.



.. py:class:: Geometry(obj: jupedsim.native.Geometry)

   Geometry object representing the area agents can move on.

   Gain access to the simulation's walkable area by calling:

   .. code :: python

       sim.get_geometry()


   .. py:method:: as_wkt() -> str

      _summary_

      :returns: _description_
      :rtype: String



   .. py:method:: boundary() -> list[tuple[float, float]]

      Access the boundary polygon of the walkable area.

      :returns: List of 2d points describing the polygon.



   .. py:method:: get_walls_close_to(point: tuple[float, float]) -> list[jupedsim.linesegment.LineSegment]

      Find line segments of the geometry that are within a certain distance to a point.

      :param point: The point to check against.
      :type point: tuple[float, float]
      :param distance: The maximum distance for line segments to be included.
      :type distance: float

      :returns: List of LineSegment objects that are close to the given point.



   .. py:method:: get_walls_in_distance_to(point: tuple[float, float], distance: float) -> list[jupedsim.linesegment.LineSegment]

      Find line segments of the geometry that are within a certain distance to a point.

      :param point: The point to check against.
      :type point: tuple[float, float]
      :param distance: The maximum distance for line segments to be included.
      :type distance: float

      :returns: List of LineSegment objects that are close to the given point.



   .. py:method:: holes() -> list[list[tuple[float, float]]]

      Access holes (inner boundaries) of the walkable area.

      :returns: A list of polygons forming holes inside the boundary.



   .. py:method:: linesegments_close_to(point: tuple[float, float]) -> list[jupedsim.linesegment.LineSegment]

      Find line segments of the geometry that are within a certain distance to a point.

      :param point: The point to check against.
      :type point: tuple[float, float]
      :param distance: The maximum distance for line segments to be included.
      :type distance: float

      :returns: List of LineSegment objects that are close to the given point.



.. py:class:: JourneyDescription(stage_ids: Optional[list[int]] = None)

   Used to describe a journey for construction by the :class:`~jupedsim.simulation.Simulation`.

   A Journey describes the desired stations an agent should take when moving through
   the simulation space. A journey is described by a graph of stages (nodes) and
   transitions (edges). See :class:`~jupedsim.journey.Transition` for an overview of the possible
   transitions.


   .. py:method:: add(stages: int | list[int]) -> None

      Add additional stage or stages.

      :param stages: A single stage id or a list of stage ids.



   .. py:method:: set_transition_for_stage(stage_id: int, transition: Transition) -> None

      Set a new transition for the specified stage.

      Any prior set transition for this stage will be removed.

      :param stage_id: id of the stage to set the transition for.
      :param transition: transition to set



.. py:class:: LineSegment(obj: jupedsim.native.LineSegment)

   Represents a line segment in 2D space, defined by two endpoints p1 and p2.


   .. py:method:: closest_point(point: tuple[float, float]) -> tuple[float, float]

      Calculate the closest point on the line segment to a given point.

      :param point: The point to find the closest point to.
      :type point: tuple[float, float]

      :returns: The closest point on the line segment to the given point.
      :rtype: tuple[float, float]



   .. py:method:: distance_to_point(point: tuple[float, float]) -> float

      Calculate the distance from a given point to this line segment.

      :param point: The point to calculate the distance to.
      :type point: tuple[float, float]

      :returns: The distance from the point to the line segment.
      :rtype: float



   .. py:property:: p1
      :type: tuple[float, float]


      Get the first endpoint of the line segment.


   .. py:property:: p2
      :type: tuple[float, float]


      Get the second endpoint of the line segment.


.. py:class:: NeighborhoodSearch(obj: jupedsim.native.NeighborhoodSearch)

   Pure Python wrapper for the C++ NeighborhoodSearch class.

   Provides efficient spatial queries for finding neighboring agents within
   a given radius. Uses a grid-based data structure for O(1) cell lookups
   with configurable cell size.


   .. rubric:: Example

   >>> neighbors = neighborhood.get_neighboring_agents(
   ...     position=(5.0, 5.0),
   ...     radius=2.0
   ... )


   .. py:method:: get_neighboring_agents(position: Tuple[float, float], radius: float) -> List[_TransientAgent]

      Get all agents within a certain radius of a position.

      Uses the underlying spatial grid for efficient O(1) average-case
      lookup time (worst case depends on number of agents per cell).

      :param position: Query position as (x, y) tuple [m]
      :param radius: Search radius [m]

      :returns: List of transient agent views within the radius. Empty list if no
                agents found. The returned objects are only valid for the duration
                of the custom-model callback they were created in; never store
                them.

      :raises ValueError: If radius < 0

      .. rubric:: Example

      >>> neighbors = neighborhood.get_neighboring_agents(
      ...     position=(5.0, 5.0),
      ...     radius=2.5
      ... )
      >>> print(f"Found {len(neighbors)} neighbors")



.. py:class:: NotifiableQueueStage(backing)

   Models a queue where agents can wait until notified.

   The queues waiting positions are predefined and agents will wait on the
   first empty position. When agents leave the queue the remaining waiting
   agents move up. If there are more agents trying to enqueue than there are
   waiting positions defined the overflow agents will wait at the last waiting
   position in the queue.

   .. note::
       This type is used to interact with an already created stage. To create
       a stage of this type see :class:`~jupedsim.simulation.Simulation`



   .. py:method:: count_enqueued() -> int

      :returns: Number of agents currently enqueued at this stage.



   .. py:method:: count_targeting() -> int

      :returns: Number of agents currently targeting this stage.



   .. py:method:: enqueued() -> list[int]

      Access the ids of all enqueued agents in order they are waiting at
      the queue.

      :returns: list of enqueued agents ordered by their position in the queue.



   .. py:method:: pop(count) -> None

      Pop `count` number of agents from the front of the queue.

      :param count: Number of agents to be popped from the front of the
                    queue



.. py:class:: Recording(db_connection_str: str, uri=False)

   .. py:method:: bounds() -> jupedsim.internal.aabb.AABB


   .. py:method:: frame(index: int) -> RecordingFrame

      Access a single frame of the recording.

      :param index: index of the frame to access.
      :type index: int

      :returns: A single frame.



   .. py:method:: geometry() -> shapely.GeometryCollection

      Access this recordings' geometry.

      :returns: walkable area of the simulation that created this recording.



   .. py:method:: geometry_id_for_frame(frame_id) -> int


   .. py:attribute:: db


   .. py:property:: fps
      :type: float


      How many frames are stored per second.

      :returns: Frames per second of this recording.


   .. py:property:: num_frames
      :type: int


      Access the number of frames stored in this recording.

      :returns: Number of frames in this recording.


.. py:class:: RecordingAgent

   Data for a single agent at a single frame.


   .. py:attribute:: id
      :type:  int


   .. py:attribute:: position
      :type:  tuple[float, float]


.. py:class:: RecordingFrame

   A single frame from the simulation.


   .. py:attribute:: agents
      :type:  list[RecordingAgent]


   .. py:attribute:: index
      :type:  int


.. py:class:: RoutingEngine(geometry: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]], **kwargs: Any)

   RoutingEngine to compute the shortest paths with navigation meshes.


   .. py:method:: compute_waypoints(frm: tuple[float, float], to: tuple[float, float]) -> list[tuple[float, float]]

      Computes shortest path between specified points.

      :param geometry: Data to create the geometry out of. Data may be supplied as:

                       * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

                       * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                       * :class:`~shapely.MultiPolygon`

                       * :class:`~shapely.Polygon`

                       * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                       * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.
      :param frm: point from which to find the shortest path
      :param to: point to which to find the shortest path

      :keyword excluded_areas: describes exclusions
                               from the walkable area. Only use this argument if `geometry` was
                               provided as list[tuple[float, float]].

      :returns: List of points (path) from 'frm' to 'to' including from and to.



   .. py:method:: edges_for(vertex_id: int)


   .. py:method:: is_routable(p: tuple[float, float]) -> bool

      Tests if the supplied point is inside the underlying geometry.

      :returns: If the point is inside the geometry.



   .. py:method:: mesh() -> tuple[list[tuple[float, float]], list[list[int]]]

      Access the navigation mesh geometry.

      The navigation mesh is store as a collection of convex polygons in CCW order.

      The returned data is to be interpreted as:

      .. code::

          tuple[
              list[tuple[float, float]], # All vertices in this mesh.
              list[ # List of polygons
                  list[int] # List of indices into the vertices that compose this polygon in CCW order
              ]
          ]

      :returns: A tuple of vertices and list of polygons which in turn are a list of indices
                tuple[list[tuple[float, float]],list[list[int]]]



.. py:class:: Simulation(*, model: jupedsim.models.collision_free_speed.CollisionFreeSpeedModel | jupedsim.models.collision_free_speed_v2.CollisionFreeSpeedModelV2 | jupedsim.models.collision_free_speed_v3.CollisionFreeSpeedModelV3 | jupedsim.models.generalized_centrifugal_force.GeneralizedCentrifugalForceModel | jupedsim.models.social_force.SocialForceModel | jupedsim.models.anticipation_velocity_model.AnticipationVelocityModel | jupedsim.models.warp_driver.WarpDriverModel | jupedsim.models.custom_model.CustomOperationalModel, geometry: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]], dt: float = 0.01, trajectory_writer: jupedsim.serialization.TrajectoryWriter | None = None, timer_log_level: int = 1, **kwargs: Any)

   Defines a simulation of pedestrian movement over a continuous walkable area.

   Movement of agents is described with Journeys, Stages and Transitions.
   Agents can be added and removed at will. The simulation processes one step
   at a time. No automatic stop condition exists. You can simulate multiple
   disconnected walkable areas by instantiating multiple instances of
   simulation.


   .. py:method:: add_agent(*, journey_id: int, stage_id: int, state: jupedsim.models.generalized_centrifugal_force.GeneralizedCentrifugalForceModelState | jupedsim.models.collision_free_speed.CollisionFreeSpeedModelState | jupedsim.models.collision_free_speed_v2.CollisionFreeSpeedModelV2State | jupedsim.models.collision_free_speed_v3.CollisionFreeSpeedModelV3State | jupedsim.models.anticipation_velocity_model.AnticipationVelocityModelState | jupedsim.models.social_force.SocialForceModelState | jupedsim.models.warp_driver.WarpDriverModelState | jupedsim.models.custom_model.CustomModelAgentState) -> int

      Add an agent to the simulation.

      The agent is spawned at ``state.position``.

      :param journey_id: Id of the journey the agent follows.
      :param stage_id: Id of the stage the agent initially targets.
      :param state: Initial per-agent model state. For built-in models this is
                    the matching ``XModelState`` instance, e.g.
                    :class:`~jupedsim.CollisionFreeSpeedModelState`. For custom
                    models this is your own object satisfying
                    :class:`~jupedsim.CustomModelAgentState`, i.e. exposing a
                    ``position`` attribute. The state type has to match the model
                    used in this simulation. When adding agents with invalid
                    parameters, or too close to the boundary or other agents, this
                    will cause an error.

      :returns: Id of the added agent.



   .. py:method:: add_direct_steering_stage() -> int

      Add an direct steering stage to the simulation.

      This stage allows a direct control of the target the agent is walking to.
      Thus, it will bypass the tactical and stragecial level of the simulation, but the
      operational level will still be active.

      .. important::

          A direct steering stage can only be used if it is the only stage in a Journey.

      :returns: Id of the added direct steering stage.



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



   .. py:method:: add_queue_stage(positions: list[tuple[float, float]]) -> int

      Add a new queue state to this simulation.

       Arguments:
           positions: Ordered list of the waiting
               points of this queue. The first one in the list is the head of
               the queue while the last one is the back of the queue.
      :returns: Id of the new stage.



   .. py:method:: add_waiting_set_stage(positions: list[tuple[float, float]]) -> int

      Add a new waiting set stage to this simulation.

      :param positions: Ordered list of the waiting points of this waiting set.
                        The agents will fill the waiting points in the given order. If more agents
                        are targeting the waiting, the remaining will wait at the last given point.

      :returns: Id of the new stage.



   .. py:method:: add_waypoint_stage(position: tuple[float, float], distance) -> int

      Add a new waypoint stage to this simulation.

      :param position: Position of the waypoint
      :param distance: Minimum distance required to reach this waypoint

      :returns: Id of the new stage.



   .. py:method:: agent(agent_id) -> jupedsim.agent.Agent

      Access specific agent in the simulation.

      :param agent_id: Id of the agent to access

      :returns: Handle to the agent. The handle resolves the agent on every
                attribute access and stays valid across :func:`iterate` as long
                as the agent exists.

      :raises SimulationError: if no agent with this id exists.



   .. py:method:: agent_count() -> int

      Number of agents in the simulation.

      :returns: Number of agents in the simulation.



   .. py:method:: agents() -> Iterator[jupedsim.agent.Agent]

      Agents in the simulation.

      The set of agents is snapshot when this method is called; agents
      added or removed afterwards are not reflected by the returned
      iterator.

      :returns: Iterator over handles to all agents in the simulation. The
                handles resolve the agent on every attribute access and stay
                valid across :func:`iterate` as long as the agent exists.



   .. py:method:: agents_in_polygon(poly: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]) -> list[jupedsim.agent.Agent]

      Handles to all agents inside the given polygon.

      :param poly: Polygon without holes in which to check for pedestrians. Polygon can be passed as:

                   * list of 2d points describing the outer boundary

                   * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                   * :class:`~shapely.MultiPolygon`

                   * :class:`~shapely.Polygon`

                   * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                   * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

      :returns: List of handles to all agents inside the given polygon.



   .. py:method:: agents_in_range(pos: tuple[float, float], distance: float) -> list[jupedsim.agent.Agent]

      Handles to all agents within the given distance to the given position.

      :param pos: point around which to search for agents
      :param distance: search radius

      :returns: List of handles to all agents within the given distance to the
                given position.



   .. py:method:: delta_time() -> float

      Time step length in seconds of one iteration.

      :returns: Time step length of one iteration.



   .. py:method:: elapsed_time() -> float

      Elapsed time in seconds since the start of the simulation.

      :returns: Time in seconds since the start of the simulation.



   .. py:method:: get_geometry() -> jupedsim.geometry.Geometry

      Current geometry of the simulation.

      :returns: The geometry of the simulation.



   .. py:method:: get_stage(stage_id: int)

      Specific stage in the simulation.

      :param stage_id: Id of the stage to retrieve.

      :returns: The stage object.



   .. py:method:: iterate(count: int = 1) -> None

      Advance the simulation by the given number of iterations.

      :param count: Number of iterations to advance



   .. py:method:: iteration_count() -> int

      Number of iterations performed since start of the simulation.

      :returns: Number of iterations performed.



   .. py:method:: mark_agent_for_removal(agent_id: int)

      Marks an agent for removal.

      Marks the given agent for removal in the simulation. The agent will be
      removed from the simulation in the start of the next :func:`iterate`
      call. The removal will take place before any interaction between
      agents will be computed.

      :param agent_id: Id of the agent marked for removal



   .. py:method:: removed_agents() -> list[int]

      All agents (given by Id) removed in the last iteration.

      All agents removed from the simulation since the last call of :func:`iterate`.
      These agents are can no longer be accessed.

      :returns: Ids of all removed agents since the last call of :func:`iterate`.



   .. py:method:: switch_agent_journey(agent_id: int, journey_id: int, stage_id: int) -> None

      Switch agent to the given journey at the given stage.

      :param agent_id: Id of the agent to switch
      :param journey_id: Id of the new journey to follow
      :param stage_id: Id of the stage in the new journey the agent continues with



   .. py:property:: timer
      :type: jupedsim.internal.tracing.Timer


      Timer for measuring time spent in different stages of the simulation.

      :returns: Timer object.


.. py:class:: SqliteTrajectoryWriter(*, output_file: pathlib.Path, every_nth_frame: int = 4, commit_every_nth_write: int = 100)

   Bases: :py:obj:`jupedsim.serialization.TrajectoryWriter`


   Write trajectory data into a sqlite db


   .. py:method:: begin_writing(simulation: jupedsim.simulation.Simulation) -> None

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...



   .. py:method:: close() -> None

      Flush buffer and close DB connection. Call at simulation end.



   .. py:method:: connection() -> sqlite3.Connection


   .. py:method:: every_nth_frame() -> int

      Returns the interval of this writer in frames between writes.

      1 indicates all frames are written, 10 indicates every 10th frame is
      writen and so on.

      :returns: Number of frames between writes as int



   .. py:method:: write_iteration_state(simulation: jupedsim.simulation.Simulation) -> None

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration. The default behaviour is to buffer frames in memory
      and only writing to disk when the buffer is full or close() is called.



.. py:class:: TrajectoryWriter

   Interface for trajectory serialization


   .. py:exception:: Exception

      Bases: :py:obj:`Exception`


      Represents exceptions specific to the trajectory writer.



   .. py:method:: begin_writing(simulation) -> None
      :abstractmethod:


      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as frame rate etc...




   .. py:method:: every_nth_frame() -> int
      :abstractmethod:


      Returns the interval of this writer in frames between writes.

      1 indicates all frames are written, 10 indicates every 10th frame is
      writen and so on.

      :returns: Number of frames between writes as int



   .. py:method:: write_iteration_state(simulation) -> None
      :abstractmethod:


      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.




.. py:class:: Transition(backing)

   Describes the Transition at a stage.

   This type describes how a agent will proceed after completing its stage.
   This effectively describes the set of outbound edges for a stage.

   There are 3 types of transitions currently available:

   * **Fixed transitions:** On completion of this transitions stage all agents
     will proceed to the specified next stage.

   * **Round robin transitions:** On completion of this transitions stage agents
     will proceed in a weighted round-robin manner. A round-robin transitions
     with 3 outgoing stages and the weights 5, 7, 11 the first 5 agents to make
     a choice will take the first stage, the next 7 the second stage and the
     next 11 the third stage. Next 5 will take the first stage, and so on...

   * **Least targeted transition:** On completion of this stage agents will
     proceed towards the currently least targeted amongst the specified choices.
     The number of "targeting" agents is the amount of agents currently moving
     towards this stage. This includes agents from different journeys.


   .. py:method:: create_fixed_transition(stage_id: int) -> Transition
      :staticmethod:


      Create a fixed transition.

      On completion of this transitions stage all agents will proceed to the
      specified next stage.

      :param stage_id: id of the stage to move to next.



   .. py:method:: create_least_targeted_transition(stage_ids: list[int]) -> Transition
      :staticmethod:


      Create a least targeted transition.

      On completion of this stage agents will proceed towards the currently
      least targeted amongst the specified choices. The number of "targeting"
      agents is the amount of agents currently moving towards this stage.
      This includes agents from different journeys.

      :param stage_ids: list of stage ids to choose the next target from.



   .. py:method:: create_none_transition() -> Transition
      :staticmethod:



   .. py:method:: create_round_robin_transition(stage_weights: list[tuple[int, int]]) -> Transition
      :staticmethod:


      Create a round-robin transition.

      Round-robin transitions: On completion of this transitions stage agents
      will proceed in a weighted round-robin manner. A round-robin
      transitions with 3 outgoing stages and the weights 5, 7, 11 the first 5
      agents to make a choice will take the first stage, the next 7 the
      second stage and the next 11 the third stage. Next 5 will take the
      first stage, and so on...

      :param stage_weights: list of id/weight tuples.



.. py:class:: WaitingSetStage(backing)

   Models a set of waiting positions that can be activated or deactivated.

   Similar as with a :class:`NotifiableQueueStage` there needs to be a set of
   waiting positions defined which will be filled in order of definition. The
   :class:`WaitingSetStage` now can be active or inactive. If active agents will fill
   waiting positions until all are occupied. Additional agents will all try to
   wait at the last defined waiting position. In inactive state the
   :class:`WaitingSetStage` acts as a simple waypoint at the position of the first
   defined waiting position.


   .. py:method:: count_targeting() -> int

      :returns: Number of agents currently targeting this stage.



   .. py:method:: count_waiting() -> int

      :returns: Number of agents currently waiting at this stage.



   .. py:method:: waiting() -> list[int]

      Access the ids of all waiting agents in order they are waiting.

      :returns: list of waiting agents ordered by their position.



   .. py:property:: state
      :type: WaitingSetState


      State of the set.

      Can be active or inactive, see :class:`WaitingSetState`


.. py:class:: WaitingSetState(*args, **kwds)

   Bases: :py:obj:`enum.Enum`


   Create a collection of name/value pairs.

   Example enumeration:

   >>> class Color(Enum):
   ...     RED = 1
   ...     BLUE = 2
   ...     GREEN = 3

   Access them by:

   - attribute access:

     >>> Color.RED
     <Color.RED: 1>

   - value lookup:

     >>> Color(1)
     <Color.RED: 1>

   - name lookup:

     >>> Color['RED']
     <Color.RED: 1>

   Enumerations can be iterated over, and know how many members they have:

   >>> len(Color)
   3

   >>> list(Color)
   [<Color.RED: 1>, <Color.BLUE: 2>, <Color.GREEN: 3>]

   Methods can be added to enumerations, and members can have their own
   attributes -- see the documentation for details.


   .. py:attribute:: ACTIVE


   .. py:attribute:: INACTIVE


.. py:class:: WaypointStage(backing)

   Models a waypoint.

   A waypoint is considered to be reached if an agent is within the specified
   distance to the waypoint.


   .. py:method:: count_targeting() -> int

      Returns:
      Number of agents currently targeting this stage.



.. py:function:: distribute_by_density(*, polygon: shapely.Polygon, density: float, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000) -> list[tuple[float, float]]

   Generates randomized 2D coordinates based on a desired agent density per
   square meter.

   This function will generate as many 2D coordinates as required to reach the
   desired density. Essentially this function tries to place area * density
   many agents while adhering to the distance_to_polygon and
   distance_to_agents constraints. This function may not always be able to
   generate the requested coordinate because it cannot do so without violating
   the constraints. In this case the function will stop after max_iterations
   and raise an Exception.

   :param polygon: Area where to generate 2D coordinates in.
   :param density: desired density in agents per square meter
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: distribute_by_number(*, polygon: shapely.Polygon, number_of_agents: int, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000) -> list[tuple[float, float]]

   Generates specified number of randomized 2D coordinates.

   This function will generate the speficied number of 2D coordinates where
   all coordinates are inside the specified geometry and generated coordinates
   are constraint by distance_to_agents and distance_to_polygon. This function
   may not always be able to generate the requested coordinate because it
   cannot do so without violating the constraints. In this case the function
   will stop after max_iterations and raise an Exception.

   :param polygon: polygon where the agents shall be placed
   :param number_of_agents: number of agents to be distributed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: distribute_by_percentage(*, polygon: shapely.Polygon, percent: float, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000, k: int = 30)

   Generates randomized 2D coordinates that fill the specified area to a
   percentage of a possible maximum.

   This function will generate 2D coordinates in the specified area. The
   number of positions generated depends on the ability to place aditional
   points. This function may not always be able to generate the requested
   coordinate because it cannot do so without violating the constraints. In
   this case the function will stop after max_iterations and raise an
   Exception.

   :param polygon: polygon where agents can be placed.
   :param percent: percent value of occupancy to generate. needs to be in
                   the intervall (0, 100]
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000
   :param k: maximum number of attempts to place neighbors to already inserted
             points. A higher value will result in a higher density but will greatly
             increase runtim.

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: distribute_in_circles_by_density(*, polygon: shapely.Polygon, distance_to_agents: float, distance_to_polygon: float, center_point: tuple[float, float], circle_segment_radii: list[tuple[float, float]], densities: list[float], seed: int | None = None, max_iterations: int = 10000) -> list[tuple[float, float]]

   Generates randomized 2D coordinates in a user defined number of rings
   with defined density.

   This function will generate 2D coordinates in the intersection of the
   polygon and the rings specified by the centerpoint and the min/max radii of
   each ring. The number of positions generated is defined by the desired
   density and available space of each ring. This function may not always by
   able to generate the requested coordinate because it cannot do so without
   violating the constraints. In this case the function will stop after
   max_iterations and raise an Exception.

   :param polygon: polygon where agents can be placed.
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param center_point: Center point of the rings.
   :param circle_segment_radii: min/max radius per ring, rings may not overlap
   :param desnities: density in positionsper square meter for each ring
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to place a
                          random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`
   :raises OverlappingCirclesError: if rings in circle_segment_radii
       overlapp


.. py:function:: distribute_in_circles_by_number(*, polygon: shapely.Polygon, distance_to_agents: float, distance_to_polygon: float, center_point: tuple[float, float], circle_segment_radii: list[tuple[float, float]], numbers_of_agents: list[int], seed=None, max_iterations=10000) -> list[tuple[float, float]]

   Generates randomized 2D coordinates in a user defined number of rings.

   This function will generate 2D coordinates in the intersection of the
   polygon and the rings specified by the centerpoint and the min/max radii of
   each ring. `number_of_agents` is expected to contain the number of agents
   to be placed for each ring. This function may not always be able to
   generate the requested coordinate because it cannot do so without violating
   the constraints. In this case the function will stop after max_iterations
   and raise an Exception.

   :param polygon: polygon where agents can be placed.
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param center_point: Center point of the rings.
   :param circle_segment_radii: min/max radius per ring, rings may not overlap
   :param number_of_agents: agents to be placed per ring
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`
   :raises OverlappingCirclesError: if rings in circle_segment_radii
       overlapp


.. py:function:: distribute_until_filled(*, polygon: shapely.Polygon, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000, k: int = 30) -> list[tuple[float, float]]

   Generates randomized 2D coordinates that fill the specified area.

   This function will generate 2D coordinates in the specified area. The
   number of positions generated depends on the ability to place aditional
   points. This function may not always be able to generate the requested
   coordinate because it cannot do so without violating the constraints. In
   this case the function will stop after max_iterations and raise an
   Exception.

   :param polygon: polygon where agents can be placed.
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000
   :param k: maximum number of attempts to place neighbors to already inserted
             points. A higher value will result in a higher density but will greatly
             increase runtim.

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: get_build_info() -> BuildInfo

   Get build information about jupedsim.

   The received :class:`BuildInfo` is printable, e.g.

   .. code:: python

       print(get_build_info())

   This will display a human-readable string stating
   basic information about this library.


.. py:function:: set_debug_callback(fn: Callable[[str], None]) -> None

   Set receiver for debug messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_error_callback(fn: Callable[[str], None]) -> None

   Set receiver for error messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_info_callback(fn: Callable[[str], None]) -> None

   Set receiver for info messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_warning_callback(fn: Callable[[str], None]) -> None

   Set receiver for warning messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:data:: AnticipationVelocityModel

.. py:data:: AnticipationVelocityModelState

.. py:data:: CollisionFreeSpeedModel

.. py:data:: CollisionFreeSpeedModelState

.. py:data:: CollisionFreeSpeedModelV2

.. py:data:: CollisionFreeSpeedModelV2State

.. py:data:: CollisionFreeSpeedModelV3

.. py:data:: CollisionFreeSpeedModelV3State

.. py:data:: GeneralizedCentrifugalForceModel

.. py:data:: GeneralizedCentrifugalForceModelState

.. py:data:: Hdf5TrajectoryWriter
   :value: None


.. py:data:: SimulationError

   Raised for simulation errors, e.g. when accessing an agent handle whose
   agent no longer exists or when calling mutating simulation methods from a
   custom-model callback.

.. py:data:: SocialForceModel

.. py:data:: SocialForceModelState

.. py:data:: WarpDriverModel

.. py:data:: WarpDriverModelState




