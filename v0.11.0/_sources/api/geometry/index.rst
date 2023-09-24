:py:mod:`geometry`
==================

.. py:module:: geometry


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   geometry.Geometry
   geometry.GeometryBuilder




.. py:class:: Geometry(obj)


   Geometry object for simulations.


.. py:class:: GeometryBuilder


   .. py:method:: add_accessible_area(polygon: list[tuple[float, float]]) -> None

      Adds an area which can be accessed by the agents to the geometry.

      Args:
          polygon (list[tuple[float, float]]): list of x,y coordinates of
              the points of a polygon


   .. py:method:: exclude_from_accessible_area(polygon: list[tuple[float, float]]) -> None

      Marks an area as un-accessible by the agents to the geometry.

      Args:
          polygon (list[tuple[float, float]]): list of x,y coordinates of
              the points of a polygon


   .. py:method:: build() -> Geometry

      Builds a Geometry from the given accessible and un-accessible areas.

      Returns:
          Geometry object



