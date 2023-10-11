:py:mod:`jupedsim.agent`
========================

.. py:module:: jupedsim.agent


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.agent.Agent




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


