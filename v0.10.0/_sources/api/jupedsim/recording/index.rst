:py:mod:`jupedsim.recording`
============================

.. py:module:: jupedsim.recording


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.recording.RecordingAgent
   jupedsim.recording.RecordingFrame
   jupedsim.recording.Recording




.. py:class:: RecordingAgent


   .. py:attribute:: id
      :type: int

      

   .. py:attribute:: position
      :type: tuple[float, float]

      

   .. py:attribute:: orientation
      :type: tuple[float, float]

      


.. py:class:: RecordingFrame


   .. py:attribute:: index
      :type: int

      

   .. py:attribute:: agents
      :type: list[RecordingAgent]

      


.. py:class:: Recording(db_connection_str: str, uri=False)


   .. py:property:: num_frames
      :type: int


   .. py:property:: fps
      :type: float


   .. py:method:: frame(index: int) -> RecordingFrame


   .. py:method:: geometry() -> shapely.GeometryCollection


   .. py:method:: bounds() -> jupedsim.aabb.AABB



