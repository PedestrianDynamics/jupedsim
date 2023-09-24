:py:mod:`jupedsim.util`
=======================

.. py:module:: jupedsim.util


Module Contents
---------------


Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.util.geometry_from_wkt
   jupedsim.util.geometry_from_shapely
   jupedsim.util.geometry_from_coordinates



.. py:exception:: GeometryError(message)


   Bases: :py:obj:`Exception`

   Class reflecting errors when creating JuPedSim geometry objects.


.. py:function:: geometry_from_wkt(wkt_input: str) -> jupedsim.native.geometry.Geometry


.. py:function:: geometry_from_shapely(geometry_input: shapely.Polygon | shapely.MultiPolygon | shapely.GeometryCollection | shapely.MultiPoint) -> jupedsim.native.geometry.Geometry


.. py:function:: geometry_from_coordinates(coordinates: List[Tuple], excluded_areas: Optional[List[Tuple]]) -> jupedsim.native.geometry.Geometry


