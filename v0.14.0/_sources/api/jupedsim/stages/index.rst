:py:mod:`jupedsim.stages`
=========================

.. py:module:: jupedsim.stages


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.stages.NotifiableQueueStage
   jupedsim.stages.WaitingSetState
   jupedsim.stages.WaitingSetStage
   jupedsim.stages.WaypointStage
   jupedsim.stages.ExitStage




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



.. py:class:: WaypointStage(backing)


   Models a waypoint.

   A waypoint is considered to be reached if an agent is within the specified
   distance to the waypoint.

   .. py:method:: count_targeting()

      Returns:
      Number of agents currently targeting this stage.



.. py:class:: ExitStage(backing)


   Models an exit.

   Agents entering the polygon defining the exit will be removed at the
   beginning of the next iteration, i.e. agents will be inside the specified
   polygon for one frame.

   .. py:method:: count_targeting()

      :returns: Number of agents currently targeting this stage.



