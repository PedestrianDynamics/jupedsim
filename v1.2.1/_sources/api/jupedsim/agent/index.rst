:orphan:

:py:mod:`jupedsim.agent`
========================

.. py:module:: jupedsim.agent


Module Contents
---------------

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

   .. py:property:: model
      :type: jupedsim.models.generalized_centrifugal_force.GeneralizedCentrifugalForceModelState | jupedsim.models.collision_free_speed.CollisionFreeSpeedModelState | jupedsim.models.social_force.SocialForceModelState

      Access model specific state of this agent.


