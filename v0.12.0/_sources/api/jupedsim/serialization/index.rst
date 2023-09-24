:py:mod:`jupedsim.serialization`
================================

.. py:module:: jupedsim.serialization

.. autoapi-nested-parse::

   Serialization/deserialization support

   In this file you will find interfaces and implementations to serialize and
   deserialize different forms of input / output commonly used.



Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.serialization.TrajectoryWriter
   jupedsim.serialization.JpsCoreStyleTrajectoryWriter



Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.serialization.parse_dlr_ad_hoc
   jupedsim.serialization.parse_wkt



.. py:class:: TrajectoryWriter


   Interface for trajectory serialization

   .. py:method:: begin_writing() -> None
      :abstractmethod:

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation: jupedsim.native.simulation.Simulation) -> None
      :abstractmethod:

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: end_writing() -> None
      :abstractmethod:

      End writing trajectory data.

      This method is intended to handle finalizing writing of trajectory
      data, e.g. write closing tags, or footer meta data.



.. py:class:: JpsCoreStyleTrajectoryWriter(output_file: pathlib.Path)


   Bases: :py:obj:`TrajectoryWriter`

   Writes jpscore / jpsvis compatible trajectory files w.o. a referenced geometry.

   This implementation tracks the number of calls to 'write_iteration_state'
   and inserts the appropriate frame number, to write a useful file header the
   fps the data is written in needs to be supplied on construction

   .. py:method:: begin_writing(fps: float) -> None

      Writes trajectory file header information

      Parameters
      ----------
      fps: float
          fps of the data to be written

      Raises
      ------
      IOError
          Opens the output file with pathlib.Path.open(). Any exception passed on.


   .. py:method:: write_iteration_state(simulation: jupedsim.native.simulation.Simulation) -> None

      Writes trajectory information for a single iteration.

      Parameters
      ----------
      simulation : jupedsim.Simulation
          The simulation object to get the trajectory data from

      Raises
      ------
      TrajectoryWriter.Exception
          Will be raised if the output file is not yet opened, i.e.
          'being_writing' has not been called yet.



   .. py:method:: end_writing() -> None

      End writing trajectory information

      Will close the file handle and end writing.



.. py:exception:: ParseException


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:function:: parse_dlr_ad_hoc(input: str) -> shapely.GeometryCollection

   This function parses data from an ad-hoc file format as it was used by the DLR to
   specify accessible areas, E.g:
       Lane :J1_w0_0
       98.5
       5
       98.5
       -5
       101.5
       -1.5
       101.5
       1.5
       Lane :J2_w0_0
       198.5
       1.5
       198.5
       -1.5
       201.5
       -5
       201.5
       5
   Identifiers are followed by x and y coordinates, each specifying a polygon.

   Parameters
   ----------
   input : str
       text in the above mentioned format

   Returns
   -------
   shapely.GeometryCollection that only contains polygons


.. py:function:: parse_wkt(input: str) -> shapely.GeometryCollection

   Creates a Geometry collection from a WKT collection

   Parameters
   ----------
   input : str
       text containing one WKT GEOMETRYCOLLECTION

   Raises
   ------
   ParseException will be raised on any errors parsing the input

   Returns
   -------
   A shapely.GeometryCollection that only contains polygons


