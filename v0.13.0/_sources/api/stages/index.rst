:py:mod:`stages`
================

.. py:module:: stages


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   stages.NotifiableQueueProxy
   stages.WaitingSetState
   stages.WaitingSetProxy
   stages.WaypointProxy
   stages.ExitProxy




.. py:class:: NotifiableQueueProxy(backing)


   .. py:method:: count_targeting() -> int


   .. py:method:: count_enqueued() -> int


   .. py:method:: pop(count) -> None


   .. py:method:: enqueued() -> list[int]



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

      


.. py:class:: WaitingSetProxy(backing)


   .. py:property:: state


   .. py:method:: count_targeting()


   .. py:method:: count_waiting()


   .. py:method:: waiting()



.. py:class:: WaypointProxy(backing)


   .. py:method:: count_targeting()



.. py:class:: ExitProxy(backing)


   .. py:method:: count_targeting()



