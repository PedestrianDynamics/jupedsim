:py:mod:`jupedsim.geometry_utils`
=================================

.. py:module:: jupedsim.geometry_utils


Module Contents
---------------


Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.geometry_utils.build_geometry



.. py:exception:: GeometryError(message)


   Bases: :py:obj:`Exception`

   Class reflecting errors when creating JuPedSim geometry objects.

   Create GeometryError with the given message.

   :param message: Error message


.. py:function:: build_geometry(geometry, **kwargs)

   Create a :class:`~jupedsim.geometry.Geometry` from different input representations.

   .. note ::
       The geometric data supplied need to form a single "simple" polygon with holes. In case
       the input contains multiple polygons this must hold true for the union of all polygons.

   :param geometry: Data to create the geometry out of. Data may be supplied as:

                    * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

                    * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                    * :class:`~shapely.MultiPolygon`

                    * :class:`~shapely.Polygon`

                    * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                    * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

   :keyword excluded_areas: describes exclusions
                            from the walkable area. Only use this argument if `geometry` was
                            provided as list[tuple[float, float]].
   :kwtype excluded_areas: list[list[tuple(float, float)]]


