:py:mod:`jupedsim.geometry`
===========================

.. py:module:: jupedsim.geometry


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.geometry.Geometry




.. py:class:: Geometry(obj)


   Geometry object representing the area agents can move on.

   Gain access to the simulation's walkable area by calling:

   .. code :: python

       sim.get_geometry()

   .. py:method:: boundary()

      Access the boundary polygon of the walkable area.

      :returns: List of 2d points describing the polygon.


   .. py:method:: holes()

      Access holes (inner boundaries) of the walkable area.

      :returns: A list of polygons forming holes inside the boundary.



