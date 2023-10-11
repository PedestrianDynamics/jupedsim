:py:mod:`jupedsim`
==================

.. py:module:: jupedsim


Subpackages
-----------
.. toctree::
   :titlesonly:
   :maxdepth: 3

   native/index.rst


Submodules
----------
.. toctree::
   :titlesonly:
   :maxdepth: 1

   agent/index.rst
   distributions/index.rst
   geometry/index.rst
   geometry_utils/index.rst
   journey/index.rst
   library/index.rst
   models/index.rst
   recording/index.rst
   routing/index.rst
   serialization/index.rst
   simulation/index.rst
   sqlite_serialization/index.rst
   stages/index.rst
   tracing/index.rst


Package Contents
----------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.Agent
   jupedsim.Geometry
   jupedsim.JourneyDescription
   jupedsim.Transition
   jupedsim.BuildInfo
   jupedsim.CollisionFreeSpeedModel
   jupedsim.CollisionFreeSpeedModelAgentParameters
   jupedsim.CollisionFreeSpeedModelState
   jupedsim.GeneralizedCentrifugalForceModel
   jupedsim.GeneralizedCentrifugalForceModelAgentParameters
   jupedsim.GeneralizedCentrifugalForceModelState
   jupedsim.Recording
   jupedsim.RecordingAgent
   jupedsim.RecordingFrame
   jupedsim.RoutingEngine
   jupedsim.TrajectoryWriter
   jupedsim.Simulation
   jupedsim.SqliteTrajectoryWriter
   jupedsim.ExitStage
   jupedsim.NotifiableQueueStage
   jupedsim.WaitingSetStage
   jupedsim.WaitingSetState
   jupedsim.WaypointStage
   jupedsim.Trace



Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.distribute_by_density
   jupedsim.distribute_by_number
   jupedsim.distribute_by_percentage
   jupedsim.distribute_in_circles_by_density
   jupedsim.distribute_in_circles_by_number
   jupedsim.distribute_till_full
   jupedsim._geometry_from_coordinates
   jupedsim._geometry_from_shapely
   jupedsim._geometry_from_wkt
   jupedsim.get_build_info
   jupedsim.set_debug_callback
   jupedsim.set_error_callback
   jupedsim.set_info_callback
   jupedsim.set_warning_callback



Attributes
~~~~~~~~~~

.. autoapisummary::

   jupedsim.__version__
   jupedsim.__commit__
   jupedsim.__compiler__


.. py:class:: Agent(backing)


   Represents an Agent in the simulation.

   Agent objects are always retrieved from the simulation and never created directly.

   Agents can be accessed with:

   .. code:: python

       # a specific agent
       sim.agent(id)

       # all agents as iterator
       sim.agents()

       # agents in a specific distance to a point as iterator
       sim.agents_in_range(position, distance)

       # agents in a polygon as iterator
       sim.agents_in_polygon(polygon)

   .. note ::
       You need to be aware that currently there are no checks done when setting
       properties on an Agent instance. For example it is possible to set an Agent position
       outside the walkable area of the Simulation resulting in a crash.

   Do not use.

   Retrieve agents from the simulation.

   .. py:property:: id
      :type: int

      Numeric id of the agent in this simulation.

   .. py:property:: journey_id
      :type: int

      Id of the :class:`~jupedsim.journey.JourneyDescription` the agent is currently following.

   .. py:property:: stage_id
      :type: int

      Id of the :class:`Stage` the Agent is currently targeting.

   .. py:property:: position
      :type: tuple[float, float]

      Position of the agent.

   .. py:property:: orientation
      :type: tuple[float, float]

      Orientation of the agent.

   .. py:property:: model
      :type: jupedsim.models.GeneralizedCentrifugalForceModelState | jupedsim.models.CollisionFreeSpeedModelState

      Access model specific state of this agent.


.. py:exception:: AgentNumberError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:exception:: IncorrectParameterError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:exception:: NegativeValueError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:exception:: OverlappingCirclesError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:function:: distribute_by_density(*, polygon, density, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon with the given density

   :param polygon: shapely polygon in which the agents will be placed
   :param density: Density of agents inside the polygon
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_by_number(*, polygon, number_of_agents, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000)

   "returns number_of_agents points randomly placed inside the polygon

   :param polygon: shapely polygon in which the agents will be placed
   :param number_of_agents: number of agents distributed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
   :return: list of created points


.. py:function:: distribute_by_percentage(*, polygon, percent, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000, k=30)

   returns points for the desired percentage of agents that fit inside the polygon (max possible number)
   fills the polygon entirely using Bridson’s algorithm for Poisson-disc sampling and then selects the percentage of placed agents

   :param polygon: shapely polygon in which the agents will be placed
   :param percent: percentage of agents selected - 100% ≙ completely filled polygon 0% ≙ 0 placed points
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_in_circles_by_density(*, polygon, distance_to_agents, distance_to_polygon, center_point, circle_segment_radii, densities, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon inside each the circle segments

   :param polygon: shapely polygon in which the agents will be placed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param center_point: the Center point of the circle segments
   :param circle_segment_radii: a list of minimal and maximal radius for each circle segment
       Circle segments must not overlap
       formatted like [(minimum_radius, maximum_radius)]
   :param densities: a list of densities for each Circle segment
       the position of the number corresponds to the order in which the Circle segments are given
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_in_circles_by_number(*, polygon, distance_to_agents, distance_to_polygon, center_point, circle_segment_radii, numbers_of_agents, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon inside each the circle segments

   :param polygon: shapely polygon in which the agents will be placed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param center_point: the Center point of the circle segments
   :param circle_segment_radii: a list of minimal and maximal radius for each circle segment
       Circle segments must not overlap
       formatted like [(minimum_radius, maximum_radius)]
   :param numbers_of_agents: a list of number of agents for each Circle segment
       the position of the number corresponds to the order in which the Circle segments are given
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_till_full(*, polygon, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000, k=30)

   returns as many randomly placed points as fit into the polygon.

   Points are distributed using Bridson’s algorithm for Poisson-disc sampling
   The algorithm is explained in Robert Bridson´s Paper "Fast Poisson Disk Sampling in Arbitrary Dimensions"

   :param polygon: shapely polygon in which the agents will be placed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
   :param k: around each point k point will be created before the point is considered inactive
   :return: list of created points


.. py:class:: Geometry(obj)


   Geometry object representing the area agents can move on.

   Gain access to the simulation's walkable area by calling:

   .. code :: python

       sim.get_geometry()

   .. py:method:: boundary()

      Access the boundary polygon of the walkable area.

      :returns: List of 2d points describing the polygon.


   .. py:method:: holes()

      Access holes (inner boundaries) of the walkable area.

      :returns: A list of polygons forming holes inside the boundary.






.. py:class:: JourneyDescription(stage_ids = None)


   Used to describe a journey for construction by the :class:`~jupedsim.simulation.Simulation`.

   A Journey describes the desired stations an agent should take when moving through
   the simulation space. A journey is described by a graph of stages (nodes) and
   transitions (edges). See :class:`~jupedsim.journey.Transition` for an overview of the possible
   transitions.

   Create a Journey Description.

   :param stage_ids: list of stages this journey should contain.
   :type stage_ids: Optional[list[int]]

   .. py:method:: add(stages)

      Add additional stage or stages.

      :param stages: A single stage id or a list of stage ids.
      :type stages: int | list[int]


   .. py:method:: set_transition_for_stage(stage_id, transition)

      Set a new transition for the specified stage.

      Any prior set transition for this stage will be removed.

      :param stage_id: id of the stage to set the transition for.
      :type stage_id: int
      :param transition: transition to set
      :type transition: Transition



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

   .. py:method:: create_fixed_transition(stage_id)
      :staticmethod:

      Create a fixed transition.

      On completion of this transitions stage all agents will proceed to the
      specified next stage.

      :param stage_id: id of the stage to move to next.
      :type stage_id: int


   .. py:method:: create_round_robin_transition(stage_weights)
      :staticmethod:

      Create a round-robin transition.

      Round-robin transitions: On completion of this transitions stage agents
      will proceed in a weighted round-robin manner. A round-robin
      transitions with 3 outgoing stages and the weights 5, 7, 11 the first 5
      agents to make a choice will take the first stage, the next 7 the
      second stage and the next 11 the third stage. Next 5 will take the
      first stage, and so on...

      :param stage_weights: list of id/weight tuples.
      :type stage_weights: list[tuple[int, int]]


   .. py:method:: create_least_targeted_transition(stage_ids)
      :staticmethod:

      Create a least targeted transition.

      On completion of this stage agents will proceed towards the currently
      least targeted amongst the specified choices. The number of "targeting"
      agents is the amount of agents currently moving towards this stage.
      This includes agents from different journeys.

      :param stage_ids: list of stage ids to choose the next target
                        from.
      :type stage_ids: list[int]



.. py:class:: BuildInfo


   .. py:property:: git_commit_hash
      :type: str

      SHA1 commit hash of this version.

      :returns: SHA1 of this version.

   .. py:property:: git_commit_date
      :type: str

      Date this commit was created.

      :returns: Date the commit of this version as string.

   .. py:property:: git_branch
      :type: str

      Branch this commit was crated from.

      :returns: name of the branch this version was build from.

   .. py:property:: compiler
      :type: str

      Compiler the native code was compiled with.

      :returns: Compiler identification.

   .. py:property:: compiler_version
      :type: str

      Compiler version the native code was compiled with.

      :returns: Compiler version number.

   .. py:property:: library_version
      :type: str



.. py:function:: get_build_info()

   Get build information about jupedsim.

   The received :class:`BuildInfo` is printable, e.g.

   .. code:: python

       print(get_build_info())

   This will display a human-readable string stating
   basic information about this library.


.. py:function:: set_debug_callback(fn)

   Set receiver for debug messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_error_callback(fn)

   Set receiver for error messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_info_callback(fn)

   Set receiver for info messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:function:: set_warning_callback(fn)

   Set receiver for warning messages.

   :param fn: function that accepts a msg as string
   :type fn: fn<str>


.. py:class:: CollisionFreeSpeedModel


   Parameters for Collision Free Speed Model

   All attributes are initialized with reasonably good defaults.

   .. attribute:: strength_neighbor_repulsion

      Strength of the repulsion from neighbors

      :type: float

   .. attribute:: range_neighbor_repulsion

      Range of the repulsion from neighbors

      :type: float

   .. attribute:: strength_geometry_repulsion

      Strength of the repulsion from geometry boundaries

      :type: float

   .. attribute:: range_geometry_repulsion

      Range of the repulsion from geometry boundaries

      :type: float

   .. py:attribute:: strength_neighbor_repulsion
      :type: float
      :value: 8.0

      

   .. py:attribute:: range_neighbor_repulsion
      :type: float
      :value: 0.1

      

   .. py:attribute:: strength_geometry_repulsion
      :type: float
      :value: 5.0

      

   .. py:attribute:: range_geometry_repulsion
      :type: float
      :value: 0.02

      


.. py:class:: CollisionFreeSpeedModelAgentParameters


   Agent parameters for Collision Free Speed Model.

   See the scientific publication for more details about this model
   https://arxiv.org/abs/1512.05597

   .. note::
       Insances of this type are copied when creating the agent, you can savely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

           .. code:: python

           positions = [...] # List of initial agent positions
           params = CollisionFreeSpeedModelAgentParameters(speed=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: position

      

      :type: tuple[float, float]

   .. attribute:: time_gap

      

      :type: float

   .. attribute:: v0

      

      :type: float

   .. attribute:: radius

      

      :type: float

   .. attribute:: journey_id

      

      :type: int

   .. attribute:: stage_id

      

      :type: int

   .. py:attribute:: position
      :type: tuple[float, float]
      :value: (0.0, 0.0)

      

   .. py:attribute:: time_gap
      :type: float
      :value: 1.0

      

   .. py:attribute:: v0
      :type: float
      :value: 1.2

      

   .. py:attribute:: radius
      :type: float
      :value: 0.2

      

   .. py:attribute:: journey_id
      :type: int
      :value: 0

      

   .. py:attribute:: stage_id
      :type: int
      :value: 0

      

   .. py:method:: as_native()



.. py:class:: CollisionFreeSpeedModelState(backing)


   .. py:property:: time_gap
      :type: float


   .. py:property:: v0
      :type: float

      Maximum speed of this agent.

   .. py:property:: radius
      :type: float

      Radius of this agent.


.. py:class:: GeneralizedCentrifugalForceModel


   Parameters for Generalized Centrifugal Force Model

   All attributes are initialized with reasonably good defaults.

   .. attribute:: strength_neighbor_repulsion

      strengh_neighbor_repulsion

      :type: float

   .. attribute:: strength_geometry_repulsion

      strength_geometry_repulsion

      :type: float

   .. attribute:: max_neighbor_interaction_distance

      cut-off-radius for ped-ped repulsion (r_c in FIG. 7)

      :type: float

   .. attribute:: max_geometry_interaction_distance

      cut-off-radius for ped-wall repulsion (r_c in FIG. 7)

      :type: float

   .. attribute:: max_neighbor_interpolation_distance

      distance of interpolation of repulsive force for ped-ped interaction (r_eps in FIG. 7)

      :type: float

   .. attribute:: max_geometry_interpolayion_distance

      distance of interpolation of repulsive force for ped-wall interaction (r_eps in FIG. 7)

      :type: float

   .. attribute:: max_neighbor_repulsion_force

      maximum of the repulsion force for ped-ped interaction by contact of ellipses (f_m in FIG. 7)

      :type: float

   .. attribute:: max_geometry_repulsion_force

      maximum of the repulsion force for ped-wall interaction by contact of ellipses (f_m in FIG. 7)

      :type: float

   .. py:attribute:: strength_neighbor_repulsion
      :type: float
      :value: 0.3

      

   .. py:attribute:: strength_geometry_repulsion
      :type: float
      :value: 0.2

      

   .. py:attribute:: max_neighbor_interaction_distance
      :type: float
      :value: 2

      

   .. py:attribute:: max_geometry_interaction_distance
      :type: float
      :value: 2

      

   .. py:attribute:: max_neighbor_interpolation_distance
      :type: float
      :value: 0.1

      

   .. py:attribute:: max_geometry_interpolation_distance
      :type: float
      :value: 0.1

      

   .. py:attribute:: max_neighbor_repulsion_force
      :type: float
      :value: 3

      

   .. py:attribute:: max_geometry_repulsion_force
      :type: float
      :value: 3

      


.. py:class:: GeneralizedCentrifugalForceModelAgentParameters


   Parameters required to create an Agent in the Generalized Centrifugal Force
   Model.

   See the scientific publication for more details about this model
   https://arxiv.org/abs/1008.4297

   .. note::
       Insances of this type are copied when creating the agent, you can savely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = GeneralizedCentrifugalForceModelAgentParameters(speed=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: speed

      

      :type: float

   .. attribute:: e0

      

      :type: tuple[float, float]

   .. attribute:: position

      

      :type: tuple[float, float]

   .. attribute:: orientation

      

      :type: tuple[float, float]

   .. attribute:: journey_id

      

      :type: int

   .. attribute:: stage_id

      

      :type: int

   .. attribute:: mass

      

      :type: float

   .. attribute:: tau

      

      :type: float

   .. attribute:: v0

      

      :type: float

   .. attribute:: a_v

      

      :type: float

   .. attribute:: a_min

      

      :type: float

   .. attribute:: b_min

      

      :type: float

   .. attribute:: b_max

      

      :type: float

   .. py:attribute:: speed
      :type: float
      :value: 0.0

      

   .. py:attribute:: e0
      :type: tuple[float, float]
      :value: (0.0, 0.0)

      

   .. py:attribute:: position
      :type: tuple[float, float]
      :value: (0.0, 0.0)

      

   .. py:attribute:: orientation
      :type: tuple[float, float]
      :value: (0.0, 0.0)

      

   .. py:attribute:: journey_id
      :type: int

      

   .. py:attribute:: stage_id
      :type: int

      

   .. py:attribute:: mass
      :type: float
      :value: 1

      

   .. py:attribute:: tau
      :type: float
      :value: 0.5

      

   .. py:attribute:: v0
      :type: float
      :value: 1.2

      

   .. py:attribute:: a_v
      :type: float
      :value: 1

      

   .. py:attribute:: a_min
      :type: float
      :value: 0.2

      

   .. py:attribute:: b_min
      :type: float
      :value: 0.2

      

   .. py:attribute:: b_max
      :type: float
      :value: 0.4

      

   .. py:method:: as_native()



.. py:class:: GeneralizedCentrifugalForceModelState(backing)


   .. py:property:: speed
      :type: float

      Speed of this agent.

   .. py:property:: e0
      :type: tuple[float, float]

      Desired direction of this agent.

   .. py:property:: tau
      :type: float


   .. py:property:: v0
      :type: float

      Maximum speed of this agent.

   .. py:property:: a_v
      :type: float

      Stretch of the elipsis semi-axis along the movement vector.

   .. py:property:: a_min
      :type: float

      Minimum length of the ellipsis semi-axis along the movement vector.

   .. py:property:: b_min
      :type: float

      Minimum length of the ellipsis semi-axis orthogonal to the movement vector.

   .. py:property:: b_max
      :type: float

      Maximum length of the ellipsis semi-axis orthogonal to the movement vector.


.. py:class:: Recording(db_connection_str, uri=False)


   .. py:property:: num_frames
      :type: int

      Access the number of frames stored in this recording.

      :returns: Number of frames in this recording.

   .. py:property:: fps
      :type: float

      How many frames are stored per second.

      :returns: Frames per second of this recording.

   .. py:method:: frame(index)

      Access a single frame of the recording.

      :param index: index of the frame to access.
      :type index: int

      :returns: A single frame.


   .. py:method:: geometry()

      Access this recordings' geometry.

      :returns: walkable area of the simulation that created this recording.


   .. py:method:: bounds()

      Get bounds of the position data contained in this recording.



.. py:class:: RecordingAgent


   Data for a single agent at a single frame.

   .. py:attribute:: id
      :type: int

      

   .. py:attribute:: position
      :type: tuple[float, float]

      

   .. py:attribute:: orientation
      :type: tuple[float, float]

      


.. py:class:: RecordingFrame


   A single frame from the simulation.

   .. py:attribute:: index
      :type: int

      

   .. py:attribute:: agents
      :type: list[RecordingAgent]

      


.. py:class:: RoutingEngine(geometry, **kwargs)


   RoutingEngine to compute the shortest paths with navigation meshes.

   .. py:method:: compute_waypoints(frm, to)

      Computes shortest path between specified points.

      :param frm: point from which to find the shortest path
      :type frm: tuple[float, float]
      :param to: point to which to find the shortest path
      :type to: tuple[float, float]

      :returns: List of points (path) from 'frm' to 'to' including from and to.


   .. py:method:: is_routable(p)

      Tests if the supplied point is inside the underlying geometry.

      :returns: If the point is inside the geometry.


   .. py:method:: mesh()

      Access the navigation mesh geometry.

      The navigation mesh is store as a collection of triangles in CCW order.

      :returns: The triangles composing the navigation mesh. Each triangle is encoded as 3-tuple of points.


   .. py:method:: edges_for(vertex_id)



.. py:class:: TrajectoryWriter


   Interface for trajectory serialization

   .. py:method:: begin_writing(simulation)
      :abstractmethod:

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as frame rate etc...



   .. py:method:: write_iteration_state(simulation)
      :abstractmethod:

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.



   .. py:method:: every_nth_frame()
      :abstractmethod:

      Returns the interval of this writer in frames between writes.

      1 indicates all frames are written, 10 indicates every 10th frame is
      writen and so on.

      :returns: Number of frames between writes as int



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



.. py:class:: SqliteTrajectoryWriter(*, output_file, every_nth_frame = 4)


   Bases: :py:obj:`jupedsim.serialization.TrajectoryWriter`

   Write trajectory data into a sqlite db

   SqliteTrajectoryWriter constructor

   Args:
   output_file : pathlib.Path
       name of the output file.
       Note: the file will not be written until the first call to 'begin_writing'
   every_nth_frame: int
       indicates interval between writes, 1 means every frame, 5 every 5th

   :returns: SqliteTrajectoryWriter

   .. py:method:: begin_writing(simulation)

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation)

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: every_nth_frame()

      Returns the interval of this writer in frames between writes.

      1 indicates all frames are written, 10 indicates every 10th frame is
      writen and so on.

      :returns: Number of frames between writes as int


   .. py:method:: connection()



.. py:class:: ExitStage(backing)


   Models an exit.

   Agents entering the polygon defining the exit will be removed at the
   beginning of the next iteration, i.e. agents will be inside the specified
   polygon for one frame.

   .. py:method:: count_targeting()

      :returns: Number of agents currently targeting this stage.



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


   .. py:method:: count_targeting()

      :returns: Number of agents currently targeting this stage.


   .. py:method:: count_enqueued()

      :returns: Number of agents currently enqueued at this stage.


   .. py:method:: pop(count)

      Pop `count` number of agents from the front of the queue.

      :param count: Number of agents to be popped from the front of the
                    queue


   .. py:method:: enqueued()

      Access the ids of all enqueued agents in order they are waiting at
      the queue.

      :returns: list of enqueued agents ordered by their position in the queue.



.. py:class:: WaitingSetStage(backing)


   Models a set of waiting positions that can be activated or deactivated.

   Similar as with a :class:`NotifiableQueueStage there needs to be a set of
   waiting positions defined which will be filled in order of definition. The
   :class:`WaitingSetStage` now can be active or inactive. If active agents will fill
   waiting positions until all are occupied. Additional agents will all try to
   wait at the last defined waiting position. In inactive state the
   :class:`WaitingSetStage` acts as a simple waypoint at the position of the first
   defined waiting position.

   .. py:property:: state
      :type: WaitingSetState

      State of the set.

      Can be active or inactive, see :class:`WaitingSetState`

   .. py:method:: count_targeting()

      :returns: Number of agents currently targeting this stage.


   .. py:method:: count_waiting()

      :returns: Number of agents currently waiting at this stage.


   .. py:method:: waiting()

      Access the ids of all waiting agents in order they are waiting.

      :returns: list of waiting agents ordered by their position.



.. py:class:: WaitingSetState(*args, **kwds)


   Bases: :py:obj:`enum.Enum`

   Create a collection of name/value pairs.

   Example enumeration:

   >>> class Color(Enum):
   ...     RED = 1
   ...     BLUE = 2
   ...     GREEN = 3

   Access them by:

   - attribute access::

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

   .. py:method:: count_targeting()

      Returns:
      Number of agents currently targeting this stage.



.. py:class:: Trace


   .. py:property:: iteration_duration

      Time for one simulation iteration in us.

      :returns: Time for one simulation iteration in us

   .. py:property:: operational_level_duration

      Time for one simulation iteration in the operational level in us.

      :returns: Time for one simulation iteration in the operational level in us





