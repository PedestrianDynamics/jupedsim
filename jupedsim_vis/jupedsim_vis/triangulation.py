# SPDX-License-Identifier: LGPL-3.0-or-later
"""Constrained Delaunay triangulation of walkable geometry.

This module turns any areal shapely geometry into an indexed triangle mesh.
It depends on shapely only; keep it free of VTK, Qt and jupedsim imports.
"""

from dataclasses import dataclass

import shapely


class TriangulationError(Exception):
    """Raised when a geometry cannot be turned into a triangle mesh."""


@dataclass(frozen=True)
class Triangulation:
    """An indexed triangle mesh.

    ``points`` holds the deduplicated vertices, ``triangles`` holds index
    triples into ``points``.
    """

    points: list[tuple[float, float]]
    triangles: list[tuple[int, int, int]]


def triangulate(geometry: shapely.Geometry) -> Triangulation:
    """Triangulate the areal parts of ``geometry``.

    Holes, ``MultiPolygon`` and ``GeometryCollection`` inputs are handled by
    GEOS itself. Raises :class:`TriangulationError` for empty, non-areal or
    invalid input.
    """
    if geometry is None or geometry.is_empty:
        raise TriangulationError("Cannot triangulate an empty geometry.")

    try:
        mesh = shapely.constrained_delaunay_triangles(geometry)
    except shapely.errors.GEOSException as e:
        raise TriangulationError(
            f"Could not triangulate the geometry: {e}"
        ) from e

    if mesh is None or mesh.is_empty:
        raise TriangulationError(
            "The geometry contains no polygonal area to triangulate."
        )

    points: list[tuple[float, float]] = []
    index_of: dict[tuple[float, float], int] = {}
    triangles: list[tuple[int, int, int]] = []

    for triangle in mesh.geoms:
        indices = []
        # The exterior ring of a triangle is closed: 4 coordinates, the
        # last one repeating the first.
        for x, y in triangle.exterior.coords[:3]:
            point = (x, y)
            index = index_of.get(point)
            if index is None:
                index = len(points)
                index_of[point] = index
                points.append(point)
            indices.append(index)
        triangles.append((indices[0], indices[1], indices[2]))

    return Triangulation(points=points, triangles=triangles)
