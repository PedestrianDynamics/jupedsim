:orphan:

:py:mod:`jupedsim.recording`
============================

.. py:module:: jupedsim.recording


Module Contents
---------------

.. py:class:: RecordingAgent


   Data for a single agent at a single frame.

   .. py:attribute:: id
      :type: int

      

   .. py:attribute:: position
      :type: tuple[float, float]

      

   .. py:attribute:: orientation
      :type: tuple[float, float]

      


.. py:class:: RecordingFrame


   A single frame from the simulation.

   .. py:attribute:: index
      :type: int

      

   .. py:attribute:: agents
      :type: list[RecordingAgent]

      


.. py:class:: Recording(db_connection_str: str, uri=False)


   .. py:property:: num_frames
      :type: int

      Access the number of frames stored in this recording.

      :returns: Number of frames in this recording.

   .. py:property:: fps
      :type: float

      How many frames are stored per second.

      :returns: Frames per second of this recording.

   .. py:method:: frame(index: int) -> RecordingFrame

      Access a single frame of the recording.

      :param index: index of the frame to access.
      :type index: int

      :returns: A single frame.


   .. py:method:: geometry() -> shapely.GeometryCollection

      Access this recordings' geometry.

      :returns: walkable area of the simulation that created this recording.


   .. py:method:: bounds() -> jupedsim.internal.aabb.AABB

      Get bounds of the position data contained in this recording.



