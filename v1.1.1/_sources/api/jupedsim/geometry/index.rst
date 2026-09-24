:orphan:

:py:mod:`jupedsim.geometry`
===========================

.. py:module:: jupedsim.geometry


Module Contents
---------------

.. py:class:: Geometry(obj: jupedsim.native.Geometry)


   Geometry object representing the area agents can move on.

   Gain access to the simulation's walkable area by calling:

   .. code :: python

       sim.get_geometry()

   .. py:method:: boundary() -> list[tuple[float, float]]

      Access the boundary polygon of the walkable area.

      :returns: List of 2d points describing the polygon.


   .. py:method:: holes() -> list[list[tuple[float, float]]]

      Access holes (inner boundaries) of the walkable area.

      :returns: A list of polygons forming holes inside the boundary.


   .. py:method:: as_wkt() -> str

      _summary_

      :returns: _description_
      :rtype: String



