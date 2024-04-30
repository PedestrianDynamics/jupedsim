:orphan:

:py:mod:`collision_free_speed_v2`
=================================

.. py:module:: collision_free_speed_v2


Module Contents
---------------

.. py:class:: CollisionFreeSpeedModelV2


   Collision Free Speed Model V2

   This is a variation of the Collision Free Speed Model where geometry and neighbor repulsion are individual
   agent parameters instead of global parameters.

   A general description of the Collision Free Speed Model can be found in the originating publication
   https://arxiv.org/abs/1512.05597

   A more detailed description can be found at https://pedestriandynamics.org/models/collision_free_speed_model/


.. py:class:: CollisionFreeSpeedModelV2AgentParameters


   Agent parameters for Collision Free Speed Model V2.

   See the scientific publication for more details about this model
   https://arxiv.org/abs/1512.05597

   .. note::

       Instances of this type are copied when creating the agent, you can safely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = CollisionFreeSpeedModelV2AgentParameters(v0=0.9) # all agents are slower
           for p in positions:
               params.position = p
               sim.add_agent(params)

   .. attribute:: position

      Position of the agent.

   .. attribute:: time_gap

      Time constant that describe how fast pedestrian close gaps.

   .. attribute:: v0

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

      


.. py:class:: CollisionFreeSpeedModelV2State(backing)


   .. py:property:: time_gap
      :type: float


   .. py:property:: v0
      :type: float

      Maximum speed of this agent.

   .. py:property:: radius
      :type: float

      Radius of this agent.

   .. py:property:: strength_neighbor_repulsion
      :type: float

      Strength of the repulsion from neighbors of this agent.

   .. py:property:: range_neighbor_repulsion
      :type: float

      Range of the repulsion from neighbors of this agent.

   .. py:property:: strength_geometry_repulsion
      :type: float

      Strength of the repulsion from geometry boundaries of this agent.

   .. py:property:: range_geometry_repulsion
      :type: float

      Range of the repulsion from geometry boundaries of this agent.


