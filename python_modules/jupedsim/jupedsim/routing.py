# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Any

import shapely

import jupedsim.native as py_jps
from jupedsim.geometry_utils import build_geometry


class RoutingEngine:
    """RoutingEngine to compute the shortest paths with navigation meshes."""

    def __init__(
        self,
        geometry: (
            str
            | shapely.GeometryCollection
            | shapely.Polygon
            | shapely.MultiPolygon
            | shapely.MultiPoint
            | list[tuple[float, float]]
        ),
        **kwargs: Any,
    ) -> None:
        self._obj = py_jps.RoutingEngine(
            build_geometry(geometry, *kwargs)._obj
        )

    def compute_waypoints(
        self, frm: tuple[float, float], to: tuple[float, float]
    ) -> list[tuple[float, float]]:
        """Computes shortest path between specified points.

        Arguments:
            geometry: Data to create the geometry out of. Data may be supplied as:

                * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

                * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                * :class:`~shapely.MultiPolygon`

                * :class:`~shapely.Polygon`

                * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

            frm: point from which to find the shortest path
            to: point to which to find the shortest path

        Keyword Arguments:
            excluded_areas: describes exclusions
                from the walkable area. Only use this argument if `geometry` was
                provided as list[tuple[float, float]].

        Returns:
            List of points (path) from 'frm' to 'to' including from and to.

        """
        return self._obj.compute_waypoints(frm, to)

    def is_routable(self, p: tuple[float, float]) -> bool:
        """Tests if the supplied point is inside the underlying geometry.

        Returns:
            If the point is inside the geometry.

        """
        return self._obj.is_routable(p)

    def mesh(
        self,
    ) -> list[
        tuple[tuple[float, float], tuple[float, float], tuple[float, float]]
    ]:
        """Access the navigation mesh geometry.

        The navigation mesh is store as a collection of triangles in CCW order.

        Returns:
            The triangles composing the navigation mesh. Each triangle is encoded as 3-tuple of points.

        """
        return self._obj.mesh()

    def edges_for(self, vertex_id: int):
        return self._obj.edges_for(vertex_id)
