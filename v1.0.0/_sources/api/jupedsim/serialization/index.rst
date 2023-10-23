:orphan:

:py:mod:`jupedsim.serialization`
================================

.. py:module:: jupedsim.serialization

.. autoapi-nested-parse::

   Serialization/deserialization support

   In this file you will find interfaces and implementations to serialize and
   deserialize different forms of input / output commonly used.



Module Contents
---------------

.. py:class:: TrajectoryWriter


   Interface for trajectory serialization

   .. py:method:: begin_writing(simulation) -> None
      :abstractmethod:

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as frame rate etc...



   .. py:method:: write_iteration_state(simulation) -> None
      :abstractmethod:

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.



   .. py:method:: every_nth_frame() -> int
      :abstractmethod:

      Returns the interval of this writer in frames between writes.

      1 indicates all frames are written, 10 indicates every 10th frame is
      writen and so on.

      :returns: Number of frames between writes as int



