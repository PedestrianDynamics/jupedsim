# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from typing import Any, List, Optional, Tuple

import shapely

import jupedsim.native as py_jps
from jupedsim.geometry import Geometry


class GeometryError(Exception):
    """Class reflecting errors when creating JuPedSim geometry objects."""

    def __init__(self, message) -> None:
        """Create GeometryError with the given message.

        Args:
            message: Error message
        """
        self.message = message


def _geometry_from_wkt(wkt_input: str) -> Geometry:
    geometry_collection = None
    try:
        wkt_type = shapely.from_wkt(wkt_input)
    except Exception as exc:
        raise GeometryError(
            f"Could not create geometry objects from the given WKT: "
            f"{wkt_input}. See following error message:\n{exc}"
        ) from exc

    if isinstance(wkt_type, shapely.GeometryCollection):
        geometry_collection = wkt_type
    else:
        try:
            geometry_collection = shapely.GeometryCollection([wkt_type])
        except Exception as exc:
            raise GeometryError(
                f"Could not create a geometry collection from the given WKT: "
                f"{wkt_input}. See following error message:\n{exc}"
            ) from exc

    polygons = _polygons_from_geometry_collection(geometry_collection)
    return Geometry(_internal_build_geometry(polygons))


def _geometry_from_shapely(
    geometry_input: (
        shapely.Polygon
        | shapely.MultiPolygon
        | shapely.GeometryCollection
        | shapely.MultiPoint
    ),
) -> Geometry:
    polygons = _polygons_from_geometry_collection(
        shapely.GeometryCollection([geometry_input])
    )
    return Geometry(_internal_build_geometry(polygons))


def _geometry_from_coordinates(
    coordinates: List[Tuple], *, excluded_areas: Optional[List[Tuple]] = None
) -> Geometry:
    polygon = shapely.Polygon(coordinates, holes=excluded_areas)
    return Geometry(_internal_build_geometry([polygon]))


def _polygons_from_geometry_collection(
    geometry_collection: shapely.GeometryCollection,
) -> List[shapely.Polygon]:
    def _polygons_from_multi_polygon(
        multi_polygon: shapely.MultiPolygon,
    ) -> List[shapely.Polygon]:
        result = []
        for polygon in multi_polygon.geoms:
            result += _polygons_from_polygon(polygon)
        return result

    def _polygons_from_linear_ring(
        linear_ring: shapely.LinearRing,
    ) -> List[shapely.Polygon]:
        return _polygons_from_polygon(shapely.Polygon(linear_ring))

    def _polygons_from_polygon(
        polygon: shapely.Polygon,
    ) -> List[shapely.Polygon]:
        return [polygon]

    polygons = []
    for geo in geometry_collection.geoms:
        if shapely.get_type_id(geo) == shapely.GeometryType.GEOMETRYCOLLECTION:
            polygons += _polygons_from_geometry_collection(geo)
        elif shapely.get_type_id(geo) == shapely.GeometryType.MULTIPOLYGON:
            polygons += _polygons_from_multi_polygon(geo)
        elif shapely.get_type_id(geo) == shapely.GeometryType.LINEARRING:
            polygons += _polygons_from_linear_ring(geo)
        elif shapely.get_type_id(geo) == shapely.GeometryType.POLYGON:
            polygons += _polygons_from_polygon(geo)
        else:
            raise GeometryError(
                f"Unexpected geometry type found in GeometryCollection: "
                f"{geo.geom_type}. Only Polygon types are allowed."
            )

    return polygons


def _internal_build_geometry(
    polygons: List[shapely.Polygon],
) -> py_jps.Geometry:
    geo_builder = py_jps.GeometryBuilder()

    for polygon in polygons:
        geo_builder.add_accessible_area(polygon.exterior.coords[:-1])
        for hole in polygon.interiors:
            geo_builder.exclude_from_accessible_area(hole.coords[:-1])
    return geo_builder.build()


def build_geometry(
    geometry: (
        list[tuple[float, float]]
        | shapely.GeometryCollection
        | shapely.Polygon
        | shapely.MultiPolygon
        | shapely.MultiPoint
        | str
    ),
    **kwargs: Any,
) -> Geometry:
    """Create a :class:`~jupedsim.geometry.Geometry` from different input representations.

    .. note ::
        The geometric data supplied need to form a single "simple" polygon with holes. In case
        the input contains multiple polygons this must hold true for the union of all polygons.

    Arguments:
        geometry: Data to create the geometry out of. Data may be supplied as:

            * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

            * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

            * :class:`~shapely.MultiPolygon`

            * :class:`~shapely.Polygon`

            * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

            * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

    Keyword Arguments:
        excluded_areas: describes exclusions
            from the walkable area. Only use this argument if `geometry` was
            provided as list[tuple[float, float]].
    """
    if isinstance(geometry, str):
        return _geometry_from_wkt(geometry)
    elif (
        isinstance(geometry, shapely.GeometryCollection)
        or isinstance(geometry, shapely.Polygon)
        or isinstance(geometry, shapely.MultiPolygon)
        or isinstance(geometry, shapely.MultiPoint)
    ):
        return _geometry_from_shapely(geometry)
    else:
        return _geometry_from_coordinates(
            geometry, excluded_areas=kwargs.get("excluded_areas")
        )
