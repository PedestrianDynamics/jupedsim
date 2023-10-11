:py:mod:`jupedsim.models`
=========================

.. py:module:: jupedsim.models


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.models.CollisionFreeSpeedModel
   jupedsim.models.GeneralizedCentrifugalForceModel
   jupedsim.models.GeneralizedCentrifugalForceModelAgentParameters
   jupedsim.models.CollisionFreeSpeedModelAgentParameters
   jupedsim.models.GeneralizedCentrifugalForceModelState
   jupedsim.models.CollisionFreeSpeedModelState




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


.. py:class:: CollisionFreeSpeedModelState(backing)


   .. py:property:: time_gap
      :type: float


   .. py:property:: v0
      :type: float

      Maximum speed of this agent.

   .. py:property:: radius
      :type: float

      Radius of this agent.


