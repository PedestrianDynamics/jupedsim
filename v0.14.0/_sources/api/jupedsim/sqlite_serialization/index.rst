:py:mod:`jupedsim.sqlite_serialization`
=======================================

.. py:module:: jupedsim.sqlite_serialization


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.sqlite_serialization.SqliteTrajectoryWriter




.. py:class:: SqliteTrajectoryWriter(*, output_file, every_nth_frame = 4)


   Bases: :py:obj:`jupedsim.serialization.TrajectoryWriter`

   Write trajectory data into a sqlite db

   SqliteTrajectoryWriter constructor

   Args:
   output_file : pathlib.Path
       name of the output file.
       Note: the file will not be written until the first call to 'begin_writing'
   every_nth_frame: int
       indicates interval between writes, 1 means every frame, 5 every 5th

   :returns: SqliteTrajectoryWriter

   .. py:method:: begin_writing(simulation)

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation)

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: every_nth_frame()

      Returns the interval of this writer in frames between writes.

      1 indicates all frames are written, 10 indicates every 10th frame is
      writen and so on.

      :returns: Number of frames between writes as int


   .. py:method:: connection()



