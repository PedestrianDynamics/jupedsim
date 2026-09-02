:orphan:

:py:mod:`social_force`
======================

.. py:module:: social_force


Module Contents
---------------

.. py:class:: SocialForceModel


   Parameters for Social Force Model

   All attributes are initialized with reasonably good defaults.

   See the scientific publication for more details about this model
   https://doi.org/10.1038/35035023

   .. attribute:: bodyForce

      describes the strength with which an agent is influenced by pushing forces from obstacles and neighbors in its direct proximity. [in kg s^-2] (is called k)

   .. attribute:: friction

      describes the strength with which an agent is influenced by frictional forces from obstacles and neighbors in its direct proximity. [in kg m^-1 s^-1] (is called :math:`\kappa`)

   .. py:attribute:: bodyForce
      :type: float
      :value: 120000

      

   .. py:attribute:: friction
      :type: float
      :value: 240000

      


.. py:class:: SocialForceModelAgentParameters


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

   .. attribute:: desiredSpeed

      desired Speed of the agent. [in m/s] (is called v0)

   .. attribute:: reactionTime

      reaction Time of the agent. [in s] (is called :math:`\tau`)

   .. attribute:: agentScale

      indicates how strong an agent is influenced by pushing forces from neighbors. [in N] (is called A)

   .. attribute:: obstacleScale

      indicates how strong an agent is influenced by pushing forces from obstacles. [in N] (is called A)

   .. attribute:: forceDistance

      indicates how much the distance between an agent and obstacles or neighbors influences social forces. [in m] (is called B)

   .. attribute:: radius

      radius of the space an agent occupies. [in m] (is called r)

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

      

   .. py:attribute:: velocity
      :type: tuple[float, float]
      :value: (0.0, 0.0)

      

   .. py:attribute:: mass
      :type: float
      :value: 80.0

      

   .. py:attribute:: desiredSpeed
      :type: float
      :value: 0.8

      

   .. py:attribute:: reactionTime
      :type: float
      :value: 0.5

      

   .. py:attribute:: agentScale
      :type: float
      :value: 2000

      

   .. py:attribute:: obstacleScale
      :type: float
      :value: 2000

      

   .. py:attribute:: forceDistance
      :type: float
      :value: 0.08

      

   .. py:attribute:: radius
      :type: float
      :value: 0.3

      


.. py:class:: SocialForceModelState(backing)


   .. py:property:: velocity
      :type: float

      velocity of this agent.

   .. py:property:: mass
      :type: float

      mass of this agent.

   .. py:property:: desiredSpeed
      :type: float

      desired Speed of this agent.

   .. py:property:: reactionTime
      :type: float

      reaction Time of this agent.

   .. py:property:: agentScale
      :type: float

      agent Scale of this agent.

   .. py:property:: obstacleScale
      :type: float

      obstacle Scale of this agent.

   .. py:property:: forceDistance
      :type: float

      force Distance of this agent.

   .. py:property:: radius
      :type: float

      radius of this agent.


