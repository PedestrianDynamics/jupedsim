:py:mod:`jupedsim.trajectory_writer_sqlite`
===========================================

.. py:module:: jupedsim.trajectory_writer_sqlite


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.trajectory_writer_sqlite.SqliteTrajectoryWriter




.. py:class:: SqliteTrajectoryWriter(output_file: pathlib.Path)


   Bases: :py:obj:`jupedsim.serialization.TrajectoryWriter`

   Write trajectory data into a sqlite db

   .. py:method:: begin_writing(fps: float, geometry_as_wkt: str) -> None

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation: jupedsim.Simulation) -> None

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: end_writing() -> None

      End writing trajectory data.

      This method is intended to handle finalizing writing of trajectory
      data, e.g. write closing tags, or footer meta data.


   .. py:method:: connection() -> sqlite3.Connection



