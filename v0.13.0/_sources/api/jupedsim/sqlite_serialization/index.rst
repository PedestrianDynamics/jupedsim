:py:mod:`jupedsim.sqlite_serialization`
=======================================

.. py:module:: jupedsim.sqlite_serialization


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.sqlite_serialization.SqliteTrajectoryWriter




.. py:class:: SqliteTrajectoryWriter(*, output_file: pathlib.Path, every_nth_frame: int = 4)


   Bases: :py:obj:`jupedsim.serialization.TrajectoryWriter`

   Write trajectory data into a sqlite db

   .. py:method:: begin_writing(simulation: jupedsim.native.simulation.Simulation) -> None

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation: jupedsim.native.simulation.Simulation) -> None

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: end_writing(simulation) -> None

      End writing trajectory data.

      This method is intended to handle finalizing writing of trajectory
      data, e.g. write closing tags, or footer meta data.


   .. py:method:: every_nth_frame() -> int

      Returns the intervall of this writer in frames between writes.

      1 indicates all frames are writen, 10 indicates every 10th frame is
      writen and so on.

      Returns:
          Number of frames beween writes as int


   .. py:method:: connection() -> sqlite3.Connection



