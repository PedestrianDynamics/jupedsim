# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from typing import Any, List, Optional, Tuple

import shapely

from jupedsim.native.geometry import Geometry, GeometryBuilder


class GeometryError(Exception):
    """Class reflecting errors when creating JuPedSim geometry objects."""

    def __init__(self, message):
        """Create GeometryError with the given message.

        Args:
            message: Error message
        """
        self.message = message


def geometry_from_wkt(wkt_input: str) -> Geometry:
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
    return _build_geometry(polygons)


def geometry_from_shapely(
    geometry_input: shapely.Polygon
    | shapely.MultiPolygon
    | shapely.GeometryCollection
    | shapely.MultiPoint,
) -> Geometry:
    polygons = _polygons_from_geometry_collection(
        shapely.GeometryCollection([geometry_input])
    )
    return _build_geometry(polygons)


def geometry_from_coordinates(
    coordinates: List[Tuple], excluded_areas: Optional[List[Tuple]]
) -> Geometry:
    polygon = shapely.Polygon(coordinates, holes=excluded_areas)
    return _build_geometry([polygon])


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


def _build_geometry(polygons: List[shapely.Polygon]) -> Geometry:
    geo_builder = GeometryBuilder()

    for polygon in polygons:
        geo_builder.add_accessible_area(polygon.exterior.coords[:-1])
        for hole in polygon.interiors:
            geo_builder.exclude_from_accessible_area(hole.coords[:-1])
    return geo_builder.build()
