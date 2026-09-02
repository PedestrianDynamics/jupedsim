:orphan:

:py:mod:`collision_free_speed`
==============================

.. py:module:: collision_free_speed


Module Contents
---------------

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

       Instances of this type are copied when creating the agent, you can safely
       create one instance of this type and modify it between calls to `add_agent`

       E.g.:

       .. code:: python

           positions = [...] # List of initial agent positions
           params = CollisionFreeSpeedModelAgentParameters(v0=0.9) # all agents are slower
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

      


.. py:class:: CollisionFreeSpeedModelState(backing)


   .. py:property:: time_gap
      :type: float


   .. py:property:: v0
      :type: float

      Maximum speed of this agent.

   .. py:property:: radius
      :type: float

      Radius of this agent.


