:py:mod:`jupedsim`
==================

.. py:module:: jupedsim


Package Contents
----------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.Agent
   jupedsim.AnticipationVelocityModel
   jupedsim.AnticipationVelocityModelAgentParameters
   jupedsim.AnticipationVelocityModelState
   jupedsim.BuildInfo
   jupedsim.CollisionFreeSpeedModel
   jupedsim.CollisionFreeSpeedModelAgentParameters
   jupedsim.CollisionFreeSpeedModelState
   jupedsim.CollisionFreeSpeedModelV2
   jupedsim.CollisionFreeSpeedModelV2AgentParameters
   jupedsim.CollisionFreeSpeedModelV2State
   jupedsim.ExitStage
   jupedsim.GeneralizedCentrifugalForceModel
   jupedsim.GeneralizedCentrifugalForceModelAgentParameters
   jupedsim.GeneralizedCentrifugalForceModelState
   jupedsim.Geometry
   jupedsim.JourneyDescription
   jupedsim.NotifiableQueueStage
   jupedsim.Recording
   jupedsim.RecordingAgent
   jupedsim.RecordingFrame
   jupedsim.RoutingEngine
   jupedsim.Simulation
   jupedsim.SocialForceModel
   jupedsim.SocialForceModelAgentParameters
   jupedsim.SocialForceModelState
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


   .. py:property:: id
      :type: int


      Numeric id of the agent in this simulation.


   .. py:property:: journey_id
      :type: int


      Id of the :class:`~jupedsim.journey.JourneyDescription` the agent is currently following.


   .. py:property:: model
      :type: jupedsim.models.generalized_centrifugal_force.GeneralizedCentrifugalForceModelState | jupedsim.models.collision_free_speed.CollisionFreeSpeedModelState | jupedsim.models.collision_free_speed_v2.CollisionFreeSpeedModelV2State | jupedsim.models.anticipation_velocity_model.AnticipationVelocityModelState | jupedsim.models.social_force.SocialForceModelState


      Access model specific state of this agent.


   .. py:property:: orientation
      :type: tuple[float, float]


      Orientation of the agent.


   .. py:property:: position
      :type: tuple[float, float]


      Position of the agent.


   .. py:property:: stage_id
      :type: int


      Id of the :class:`Stage` the Agent is currently targeting.


   .. py:property:: target
      :type: tuple[float, float]


      Current target of the agent.

      Can be used to directly steer an agent towards the given coordinate.
      This will bypass the strategical and tactical level, but the operational level
      will still be active.

      .. important::

          If the agent is not in a Journey with a DirectSteering stage, any change will be
          ignored.

      .. important::

          When setting the target, the given coordinates must lie within the walkable area.
          Otherwise, an error will be thrown at the next iteration call.

      :returns: Current target of the agent.


.. py:class:: AnticipationVelocityModel

   Anticipation Velocity Model (AVM).

   The AVM incorporates pedestrian anticipation, divided into three phases:
   1. Perception of the current situation.
   2. Prediction of future situations.
   3. Strategy selection leading to action.

   This model quantitatively reproduces bidirectional pedestrian flow by accounting for:
   - Anticipation of changes in neighboring pedestrians' positions.
   - The strategy of following others' movement. The AVM is a model that takes into consideration
     the anticipation of pedestrians. For this, the process of anticipation is divided into three parts:
     - perception of the actual situation,
     - prediction of a future situation and
     - selection of a strategy leading to action.


   A general description of the AVM can be found in the originating publication
   https://doi.org/10.1016/j.trc.2021.103464

   .. attribute:: pushout_strength

      The pushout mechanism ensures agents maintain a safe distance

   .. attribute:: from walls by adding a small outward component to their movement when within the

      

   .. attribute:: critical wall distance. This outward component, scaled by `pushoutStrength`,

      

   .. attribute:: combines with the parallel component of the agent's direction to create smooth,

      

   .. attribute:: gliding behavior along walls.

      

   .. attribute:: rng_seed

      seed value of internally used rng. If not explicitly set this
      value will be chosen randomly.


   .. py:attribute:: pushout_strength
      :type:  float
      :value: 0.3



   .. py:attribute:: rng_seed
      :type:  int


.. py:class:: AnticipationVelocityModelAgentParameters

   Agent parameters for Anticipation Velocity Model (AVM).

   See publication for more details about this model
   https://doi.org/10.1016/j.trc.2021.103464

   .. note::

       Instances of this type are copied when creating the agent, you can safely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = AnticipationVelocityModelAgentParameters(desired_speed=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: position

      Position of the agent.

   .. attribute:: time_gap

      Time constant that describe how fast pedestrian close gaps.

   .. attribute:: desired_speed

      Maximum speed of the agent.

   .. attribute:: radius

      Radius of the agent.

   .. attribute:: journey_id

      Id of the journey the agent follows.

   .. attribute:: stage_id

      Id of the stage the agent targets.

   .. attribute:: strength_neighbor_repulsion

      Strength of the repulsion from neighbors

   .. attribute:: range_neighbor_repulsion

      Range of the repulsion from neighbors

   .. attribute:: wall_buffer_distance

      Buffer distance of agents to the walls.

   .. attribute:: anticipation_time

      Anticipation time of an agent.

   .. attribute:: reaction_time

      reaction time of an agent to change its direction.


   .. py:attribute:: anticipation_time
      :type:  float
      :value: 1.0



   .. py:attribute:: desired_speed
      :type:  float
      :value: 1.2



   .. py:attribute:: journey_id
      :type:  int
      :value: 0



   .. py:attribute:: position
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: radius
      :type:  float
      :value: 0.2



   .. py:attribute:: range_neighbor_repulsion
      :type:  float
      :value: 0.1



   .. py:attribute:: reaction_time
      :type:  float
      :value: 0.3



   .. py:attribute:: stage_id
      :type:  int
      :value: 0



   .. py:attribute:: strength_neighbor_repulsion
      :type:  float
      :value: 8.0



   .. py:attribute:: time_gap
      :type:  float
      :value: 1.06



   .. py:attribute:: wall_buffer_distance
      :type:  float
      :value: 0.1



.. py:class:: AnticipationVelocityModelState(backing)

   .. py:property:: anticipation_time
      :type: float


      Anticipation time of this agent.


   .. py:property:: desired_speed
      :type: float


      desired Speed of this agent.


   .. py:property:: radius
      :type: float


      Radius of this agent.


   .. py:property:: range_neighbor_repulsion
      :type: float


      Range of the repulsion from neighbors of this agent.


   .. py:property:: reaction_time
      :type: float


      Reaction time of this agent.


   .. py:property:: strength_neighbor_repulsion
      :type: float


      Strength of the repulsion from neighbors of this agent.


   .. py:property:: time_gap
      :type: float



   .. py:property:: wall_buffer_distance

      Wall buffer distance of agent to walls.


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



.. py:class:: CollisionFreeSpeedModel

   Collision Free Speed Model

   A general description of the Collision Free Speed Model can be found in the originating publication
   https://arxiv.org/abs/1512.05597

   A more detailed description can be found at https://pedestriandynamics.org/models/collision_free_speed_model/

   .. attribute:: strength_neighbor_repulsion

      Strength of the repulsion from neighbors

   .. attribute:: range_neighbor_repulsion

      Range of the repulsion from neighbors

   .. attribute:: strength_geometry_repulsion

      Strength of the repulsion from geometry boundaries

   .. attribute:: range_geometry_repulsion

      Range of the repulsion from geometry boundaries


   .. py:attribute:: range_geometry_repulsion
      :type:  float
      :value: 0.02



   .. py:attribute:: range_neighbor_repulsion
      :type:  float
      :value: 0.1



   .. py:attribute:: strength_geometry_repulsion
      :type:  float
      :value: 5.0



   .. py:attribute:: strength_neighbor_repulsion
      :type:  float
      :value: 8.0



.. py:class:: CollisionFreeSpeedModelAgentParameters(*, position: tuple[float, float] = (0.0, 0.0), time_gap: float = 1.0, desired_speed: float = 1.2, radius: float = 0.2, journey_id: int = 0, stage_id: int = 0, v0: float | None = None)

   Agent parameters for Collision Free Speed Model.

   See the scientific publication for more details about this model
   https://arxiv.org/abs/1512.05597

   .. note::

       Instances of this type are copied when creating the agent, you can safely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = CollisionFreeSpeedModelAgentParameters(desired_speed=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: position

      Position of the agent.

   .. attribute:: time_gap

      Time constant that describe how fast pedestrian close gaps.

   .. attribute:: desired_speed

      Maximum speed of the agent.

   .. attribute:: radius

      Radius of the agent.

   .. attribute:: journey_id

      Id of the journey the agent follows.

   .. attribute:: stage_id

      Id of the stage the agent targets.


   .. py:attribute:: desired_speed
      :type:  float
      :value: 1.2



   .. py:attribute:: journey_id
      :type:  int
      :value: 0



   .. py:attribute:: position
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: radius
      :type:  float
      :value: 0.2



   .. py:attribute:: stage_id
      :type:  int
      :value: 0



   .. py:attribute:: time_gap
      :type:  float
      :value: 1.0



   .. py:property:: v0
      :type: float



.. py:class:: CollisionFreeSpeedModelState(backing)

   .. py:property:: desired_speed
      :type: float


      desired Speed of this agent.


   .. py:property:: radius
      :type: float


      Radius of this agent.


   .. py:property:: time_gap
      :type: float



   .. py:property:: v0
      :type: float


      Maximum speed of this agent.


.. py:class:: CollisionFreeSpeedModelV2

   Collision Free Speed Model V2

   This is a variation of the Collision Free Speed Model where geometry and neighbor repulsion are individual
   agent parameters instead of global parameters.

   A general description of the Collision Free Speed Model can be found in the originating publication
   https://arxiv.org/abs/1512.05597

   A more detailed description can be found at https://pedestriandynamics.org/models/collision_free_speed_model/


.. py:class:: CollisionFreeSpeedModelV2AgentParameters(*, position: tuple[float, float] = (0.0, 0.0), time_gap: float = 1.0, desired_speed: float = 1.2, v0: float | None = None, radius: float = 0.2, journey_id: int = 0, stage_id: int = 0, strength_neighbor_repulsion: float = 8.0, range_neighbor_repulsion: float = 0.1, strength_geometry_repulsion: float = 5.0, range_geometry_repulsion: float = 0.02)

   Agent parameters for Collision Free Speed Model V2.

   See the scientific publication for more details about this model
   https://arxiv.org/abs/1512.05597

   .. note::

       Instances of this type are copied when creating the agent, you can safely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = CollisionFreeSpeedModelV2AgentParameters(desired_speed=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: position

      Position of the agent.

   .. attribute:: time_gap

      Time constant that describe how fast pedestrian close gaps.

   .. attribute:: desired_speed

      Maximum speed of the agent.

   .. attribute:: radius

      Radius of the agent.

   .. attribute:: journey_id

      Id of the journey the agent follows.

   .. attribute:: stage_id

      Id of the stage the agent targets.

   .. attribute:: strength_neighbor_repulsion

      Strength of the repulsion from neighbors

   .. attribute:: range_neighbor_repulsion

      Range of the repulsion from neighbors

   .. attribute:: strength_geometry_repulsion

      Strength of the repulsion from geometry boundaries

   .. attribute:: range_geometry_repulsion

      Range of the repulsion from geometry boundaries


   .. py:attribute:: desired_speed
      :type:  float
      :value: 1.2



   .. py:attribute:: journey_id
      :type:  int
      :value: 0



   .. py:attribute:: position
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: radius
      :type:  float
      :value: 0.2



   .. py:attribute:: range_geometry_repulsion
      :type:  float
      :value: 0.02



   .. py:attribute:: range_neighbor_repulsion
      :type:  float
      :value: 0.1



   .. py:attribute:: stage_id
      :type:  int
      :value: 0



   .. py:attribute:: strength_geometry_repulsion
      :type:  float
      :value: 5.0



   .. py:attribute:: strength_neighbor_repulsion
      :type:  float
      :value: 8.0



   .. py:attribute:: time_gap
      :type:  float
      :value: 1.0



   .. py:property:: v0
      :type: float



.. py:class:: CollisionFreeSpeedModelV2State(backing)

   .. py:property:: desired_speed
      :type: float


      desired Speed of this agent.


   .. py:property:: radius
      :type: float


      Radius of this agent.


   .. py:property:: range_geometry_repulsion
      :type: float


      Range of the repulsion from geometry boundaries of this agent.


   .. py:property:: range_neighbor_repulsion
      :type: float


      Range of the repulsion from neighbors of this agent.


   .. py:property:: strength_geometry_repulsion
      :type: float


      Strength of the repulsion from geometry boundaries of this agent.


   .. py:property:: strength_neighbor_repulsion
      :type: float


      Strength of the repulsion from neighbors of this agent.


   .. py:property:: time_gap
      :type: float



   .. py:property:: v0
      :type: float


      Maximum speed of this agent.


.. py:class:: ExitStage(backing)

   Models an exit.

   Agents entering the polygon defining the exit will be removed at the
   beginning of the next iteration, i.e. agents will be inside the specified
   polygon for one frame.


   .. py:method:: count_targeting()

      :returns: Number of agents currently targeting this stage.



.. py:class:: GeneralizedCentrifugalForceModel

   Parameters for Generalized Centrifugal Force Model

   All attributes are initialized with reasonably good defaults.

   .. attribute:: strength_neighbor_repulsion

      Strength of the repulsion from neighbors

   .. attribute:: strength_geometry_repulsion

      Strength of the repulsion from geometry boundaries

   .. attribute:: max_neighbor_interaction_distance

      cut-off-radius for ped-ped repulsion (r_c in FIG. 7)

   .. attribute:: max_geometry_interaction_distance

      cut-off-radius for ped-wall repulsion (r_c in FIG. 7)

   .. attribute:: max_neighbor_interpolation_distance

      distance of interpolation of repulsive force for ped-ped interaction (r_eps in FIG. 7)

   .. attribute:: max_geometry_interpolation_distance

      distance of interpolation of repulsive force for ped-wall interaction (r_eps in FIG. 7)

   .. attribute:: max_neighbor_repulsion_force

      maximum of the repulsion force for ped-ped interaction by contact of ellipses (f_m in FIG. 7)

   .. attribute:: max_geometry_repulsion_force

      maximum of the repulsion force for ped-wall interaction by contact of ellipses (f_m in FIG. 7)


   .. py:attribute:: max_geometry_interaction_distance
      :type:  float
      :value: 2



   .. py:attribute:: max_geometry_interpolation_distance
      :type:  float
      :value: 0.1



   .. py:attribute:: max_geometry_repulsion_force
      :type:  float
      :value: 3



   .. py:attribute:: max_neighbor_interaction_distance
      :type:  float
      :value: 2



   .. py:attribute:: max_neighbor_interpolation_distance
      :type:  float
      :value: 0.1



   .. py:attribute:: max_neighbor_repulsion_force
      :type:  float
      :value: 9



   .. py:attribute:: strength_geometry_repulsion
      :type:  float
      :value: 0.2



   .. py:attribute:: strength_neighbor_repulsion
      :type:  float
      :value: 0.3



.. py:class:: GeneralizedCentrifugalForceModelAgentParameters(*, speed: float = 0.0, desired_direction: tuple[float, float] = (0.0, 0.0), position: tuple[float, float] = (0.0, 0.0), orientation: tuple[float, float] = (1.0, 0.0), journey_id: int = -1, stage_id: int = -1, mass: float = 1, tau: float = 0.5, desired_speed: float = 1.2, a_v: float = 1, a_min: float = 0.2, b_min: float = 0.2, b_max: float = 0.4, v0=None, e0=None)

   Parameters required to create an Agent in the Generalized Centrifugal Force
   Model.

   See the scientific publication for more details about this model
   https://arxiv.org/abs/1008.4297

   .. note::
       Instances of this type are copied when creating the agent, you can safely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = GeneralizedCentrifugalForceModelAgentParameters(speed=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: speed

      Speed of the agent.

   .. attribute:: desired_direction

      Desired direction of the agent.

   .. attribute:: position

      Position of the agent.

   .. attribute:: orientation

      Orientation of the agent.

   .. attribute:: journey_id

      Id of the journey the agent follows.

   .. attribute:: stage_id

      Id of the stage the agent targets.

   .. attribute:: mass

      Mass of the agent.

   .. attribute:: tau

      Time constant that describes how fast the agent accelerates to its desired speed (v0).

   .. attribute:: desired_speed

      Maximum speed of the agent.

   .. attribute:: a_v

      Stretch of the ellipsis semi-axis along the movement vector.

   .. attribute:: a_min

      Minimum length of the ellipsis semi-axis along the movement vector.

   .. attribute:: b_min

      Minimum length of the ellipsis semi-axis orthogonal to the movement vector.

   .. attribute:: b_max

      Maximum length of the ellipsis semi-axis orthogonal to the movement vector.


   .. py:attribute:: a_min
      :type:  float
      :value: 0.2



   .. py:attribute:: a_v
      :type:  float
      :value: 1



   .. py:attribute:: b_max
      :type:  float
      :value: 0.4



   .. py:attribute:: b_min
      :type:  float
      :value: 0.2



   .. py:attribute:: desired_direction
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: desired_speed
      :type:  float
      :value: 1.2



   .. py:property:: e0
      :type: tuple[float, float]



   .. py:attribute:: journey_id
      :type:  int
      :value: -1



   .. py:attribute:: mass
      :type:  float
      :value: 1



   .. py:attribute:: orientation
      :type:  tuple[float, float]
      :value: (1.0, 0.0)



   .. py:attribute:: position
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: speed
      :type:  float
      :value: 0.0



   .. py:attribute:: stage_id
      :type:  int
      :value: -1



   .. py:attribute:: tau
      :type:  float
      :value: 0.5



.. py:class:: GeneralizedCentrifugalForceModelState(backing)

   .. py:property:: a_min
      :type: float


      Minimum length of the ellipsis semi-axis along the movement vector.


   .. py:property:: a_v
      :type: float


      Stretch of the ellipsis semi-axis along the movement vector.


   .. py:property:: b_max
      :type: float


      Maximum length of the ellipsis semi-axis orthogonal to the movement vector.


   .. py:property:: b_min
      :type: float


      Minimum length of the ellipsis semi-axis orthogonal to the movement vector.


   .. py:property:: desired_direction
      :type: float


      desired direction of this agent.


   .. py:property:: desired_speed
      :type: float


      desired Speed of this agent.


   .. py:property:: e0
      :type: tuple[float, float]


      Desired direction of this agent.


   .. py:property:: speed
      :type: float


      Speed of this agent.


   .. py:property:: tau
      :type: float



   .. py:property:: v0
      :type: float


      Maximum speed of this agent.


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



   .. py:method:: holes() -> list[list[tuple[float, float]]]

      Access holes (inner boundaries) of the walkable area.

      :returns: A list of polygons forming holes inside the boundary.



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

      Get bounds of the position data contained in this recording.



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


   .. py:attribute:: orientation
      :type:  tuple[float, float]


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



.. py:class:: Simulation(*, model: jupedsim.models.collision_free_speed.CollisionFreeSpeedModel | jupedsim.models.generalized_centrifugal_force.GeneralizedCentrifugalForceModel | jupedsim.models.collision_free_speed_v2.CollisionFreeSpeedModelV2 | jupedsim.models.anticipation_velocity_model.AnticipationVelocityModel | jupedsim.models.social_force.SocialForceModel, geometry: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]], dt: float = 0.01, trajectory_writer: jupedsim.serialization.TrajectoryWriter | None = None, **kwargs: Any)

   Defines a simulation of pedestrian movement over a continuous walkable area.

   Movement of agents is described with Journeys, Stages and Transitions.
   Agents can be added and removed at will. The simulation processes one step
   at a time. No automatic stop condition exists. You can simulate multiple
   disconnected walkable areas by instantiating multiple instances of
   simulation.


   .. py:method:: add_agent(parameters: jupedsim.models.generalized_centrifugal_force.GeneralizedCentrifugalForceModelAgentParameters | jupedsim.models.collision_free_speed.CollisionFreeSpeedModelAgentParameters | jupedsim.models.collision_free_speed_v2.CollisionFreeSpeedModelV2AgentParameters | jupedsim.models.anticipation_velocity_model.AnticipationVelocityModelAgentParameters | jupedsim.models.social_force.SocialForceModelAgentParameters) -> int

      Add an agent to the simulation.

      :param parameters: Agent Parameters of the newly added model. The parameters have to
                         match the model used in this simulation. When adding agents with invalid parameters,
                         or too close to the boundary or other agents, this will cause an error.

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



   .. py:method:: add_waypoint_stage(position: tuple[float, float], distance) -> int

      Add a new waypoint stage to this simulation.

      :param position: Position of the waypoint
      :param distance: Minimum distance required to reach this waypoint

      :returns: Id of the new stage.



   .. py:method:: agent(agent_id) -> jupedsim.agent.Agent

      Access specific agent in the simulation.

      :param agent_id: Id of the agent to access

      :returns: Agent instance



   .. py:method:: agent_count() -> int

      Number of agents in the simulation.

      :returns: Number of agents in the simulation.



   .. py:method:: agents() -> Iterable[jupedsim.agent.Agent]

      Agents in the simulation.

      :returns: Iterator over all agents in the simulation.



   .. py:method:: agents_in_polygon(poly: str | shapely.GeometryCollection | shapely.Polygon | shapely.MultiPolygon | shapely.MultiPoint | list[tuple[float, float]]) -> list[jupedsim.agent.Agent]

      Return all ids for agents inside the given polygon.

      :param poly: Polygon without holes in which to check for pedestrians. Polygon can be passed as:

                   * list of 2d points describing the outer boundary

                   * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                   * :class:`~shapely.MultiPolygon`

                   * :class:`~shapely.Polygon`

                   * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                   * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

      :returns: All ids for agents inside given polygon.



   .. py:method:: agents_in_range(pos: tuple[float, float], distance: float) -> list[int]

      Ids of agents within the given distance to the given position.

      :param pos: point around which to search for agents
      :param distance: search radius

      :returns: List of ids of agents within the given distance to the given position.



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



   .. py:method:: switch_agent_journey(agent_id: int, journey_id: int, stage_id: int) -> None

      Switch agent to the given journey at the given stage.

      :param agent_id: Id of the agent to switch
      :param journey_id: Id of the new journey to follow
      :param stage_id: Id of the stage in the new journey the agent continues with



   .. py:method:: switch_geometry(geometry: jupedsim.geometry.Geometry) -> None

      Switch the geometry of the simulation.

      Exchanges the current geometry with the new one. Checks if all agents
      and stages lie within the new geometry.

      :param geometry: The new geometry to be used in the simulation.



.. py:class:: SocialForceModel(*, body_force: float = 120000, friction: float = 240000, bodyForce=None)

   Parameters for Social Force Model

   All attributes are initialized with reasonably good defaults.

   See the scientific publication for more details about this model
   https://doi.org/10.1038/35035023

   .. attribute:: body_force

      describes the strength with which an agent is influenced by pushing forces from obstacles and neighbors in its direct proximity. [in kg s^-2] (is called k)

   .. attribute:: friction

      describes the strength with which an agent is influenced by frictional forces from obstacles and neighbors in its direct proximity. [in kg m^-1 s^-1] (is called :math:`\kappa`)


   .. py:property:: bodyForce
      :type: float



   .. py:attribute:: body_force
      :type:  float
      :value: 120000



   .. py:attribute:: friction
      :type:  float
      :value: 240000



.. py:class:: SocialForceModelAgentParameters(position: tuple[float, float] = (0.0, 0.0), orientation: tuple[float, float] = (0.0, 0.0), journey_id: int = -1, stage_id: int = -1, velocity: tuple[float, float] = (0.0, 0.0), mass: float = 80.0, desired_speed: float = 0.8, reaction_time: float = 0.5, agent_scale: float = 2000, obstacle_scale: float = 2000, force_distance: float = 0.08, radius: float = 0.3, desiredSpeed=None, reactionTime=None, agentScale=None, obstacleScale=None, forceDistance=None)

   Parameters required to create an Agent in the Social Force Model.

   See the scientific publication for more details about this model
   https://doi.org/10.1038/35035023

   .. attribute:: position

      Position of the agent.

   .. attribute:: orientation

      Orientation of the agent.

   .. attribute:: journey_id

      Id of the journey the agent follows.

   .. attribute:: stage_id

      Id of the stage the agent targets.

   .. attribute:: velocity

      current velocity of the agent.

   .. attribute:: mass

      mass of the agent. [in kg] (is called m)

   .. attribute:: desired_speed

      desired Speed of the agent. [in m/s] (is called v0)

   .. attribute:: reaction_time

      reaction Time of the agent. [in s] (is called :math:`\tau`)

   .. attribute:: agent_scale

      indicates how strong an agent is influenced by pushing forces from neighbors. [in N] (is called A)

   .. attribute:: obstacle_scale

      indicates how strong an agent is influenced by pushing forces from obstacles. [in N] (is called A)

   .. attribute:: force_distance

      indicates how much the distance between an agent and obstacles or neighbors influences social forces. [in m] (is called B)

   .. attribute:: radius

      radius of the space an agent occupies. [in m] (is called r)


   .. py:property:: agentScale
      :type: float



   .. py:attribute:: agent_scale
      :type:  float
      :value: 2000



   .. py:property:: desiredSpeed
      :type: float



   .. py:attribute:: desired_speed
      :type:  float
      :value: 0.8



   .. py:property:: forceDistance
      :type: float



   .. py:attribute:: force_distance
      :type:  float
      :value: 0.08



   .. py:attribute:: journey_id
      :type:  int
      :value: -1



   .. py:attribute:: mass
      :type:  float
      :value: 80.0



   .. py:property:: obstacleScale
      :type: float



   .. py:attribute:: obstacle_scale
      :type:  float
      :value: 2000



   .. py:attribute:: orientation
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: position
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



   .. py:attribute:: radius
      :type:  float
      :value: 0.3



   .. py:property:: reactionTime
      :type: float



   .. py:attribute:: reaction_time
      :type:  float
      :value: 0.5



   .. py:attribute:: stage_id
      :type:  int
      :value: -1



   .. py:attribute:: velocity
      :type:  tuple[float, float]
      :value: (0.0, 0.0)



.. py:class:: SocialForceModelState(backing)

   .. py:property:: agentScale
      :type: float



   .. py:property:: agent_scale
      :type: float



   .. py:property:: desiredSpeed
      :type: float


      desired Speed of this agent.


   .. py:property:: desired_speed
      :type: float


      desired Speed of this agent.


   .. py:property:: forceDistance
      :type: float



   .. py:property:: force_distance
      :type: float



   .. py:property:: mass
      :type: float


      mass of this agent.


   .. py:property:: obstacleScale
      :type: float



   .. py:property:: obstacle_scale
      :type: float



   .. py:property:: radius
      :type: float


      radius of this agent.


   .. py:property:: reactionTime
      :type: float


      reaction Time of this agent.


   .. py:property:: reaction_time
      :type: float



   .. py:property:: velocity
      :type: tuple[float, float]


      velocity of this agent.


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





