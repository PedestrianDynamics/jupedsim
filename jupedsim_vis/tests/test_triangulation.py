# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the shapely based constrained Delaunay triangulation.

These tests are pure geometry: they need neither Qt nor VTK nor jupedsim.
"""

import dataclasses

import pytest
import shapely
from shapely.geometry import Polygon

from jupedsim_vis.triangulation import (
    Triangulation,
    TriangulationError,
    triangulate,
)

UNIT_SQUARE = "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))"
SQUARE_WITH_HOLE = (
    "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(3 3, 6 3, 6 6, 3 6, 3 3))"
)
TWO_SQUARES = (
    "MULTIPOLYGON(((0 0, 1 0, 1 1, 0 1, 0 0)),((3 0, 4 0, 4 1, 3 1, 3 0)))"
)
BOWTIE = "POLYGON((0 0, 2 2, 2 0, 0 2, 0 0))"


def as_polygons(result: Triangulation) -> list[Polygon]:
    """Rebuild the triangles as shapely polygons from points/indices."""
    return [
        Polygon([result.points[i] for i in triangle])
        for triangle in result.triangles
    ]


def total_area(result: Triangulation) -> float:
    return sum(polygon.area for polygon in as_polygons(result))


def assert_dedup_invariants(result: Triangulation) -> None:
    assert len(result.points) == len(set(result.points))
    for triangle in result.triangles:
        assert len(triangle) == 3
        assert len(set(triangle)) == 3
        for index in triangle:
            assert 0 <= index < len(result.points)


def test_unit_square_yields_two_triangles():
    result = triangulate(shapely.from_wkt(UNIT_SQUARE))

    assert len(result.triangles) == 2
    assert len(result.points) == 4
    assert total_area(result) == pytest.approx(1.0)
    assert set(result.points) == {
        (0.0, 0.0),
        (1.0, 0.0),
        (1.0, 1.0),
        (0.0, 1.0),
    }


def test_polygon_with_hole_keeps_area_and_avoids_the_hole():
    geometry = shapely.from_wkt(SQUARE_WITH_HOLE)

    result = triangulate(geometry)

    assert total_area(result) == pytest.approx(91.0)
    hole = Polygon(geometry.interiors[0])
    for polygon in as_polygons(result):
        assert polygon.intersection(hole).area == pytest.approx(0.0)


def test_multi_polygon_triangulates_every_part():
    result = triangulate(shapely.from_wkt(TWO_SQUARES))

    assert len(result.triangles) == 4
    assert total_area(result) == pytest.approx(2.0)
    assert_dedup_invariants(result)


def test_geometry_collection_as_stored_in_recordings():
    result = triangulate(shapely.from_wkt(f"GEOMETRYCOLLECTION({UNIT_SQUARE})"))

    assert len(result.triangles) == 2
    assert len(result.points) == 4
    assert total_area(result) == pytest.approx(1.0)


def test_vertices_are_deduplicated():
    result = triangulate(shapely.from_wkt(SQUARE_WITH_HOLE))

    assert_dedup_invariants(result)
    # The 8 corners of the two rings, shared by all triangles.
    assert len(result.points) == 8
    assert len(result.triangles) > len(result.points) // 3


def test_result_is_immutable():
    result = triangulate(shapely.from_wkt(UNIT_SQUARE))

    with pytest.raises(dataclasses.FrozenInstanceError):
        result.points = []


def test_none_geometry_is_rejected():
    with pytest.raises(TriangulationError, match="empty"):
        triangulate(None)


def test_empty_geometry_is_rejected():
    with pytest.raises(TriangulationError, match="empty"):
        triangulate(shapely.from_wkt("POLYGON EMPTY"))


def test_line_string_has_no_polygonal_area():
    with pytest.raises(TriangulationError, match="no polygonal area"):
        triangulate(shapely.from_wkt("LINESTRING(0 0, 1 1, 2 0)"))


def test_point_has_no_polygonal_area():
    with pytest.raises(TriangulationError, match="no polygonal area"):
        triangulate(shapely.from_wkt("MULTIPOINT((0 0), (1 1))"))


def test_self_intersecting_polygon_wraps_the_geos_error():
    with pytest.raises(TriangulationError) as exc_info:
        triangulate(shapely.from_wkt(BOWTIE))

    assert "Could not triangulate the geometry" in str(exc_info.value)
    assert isinstance(exc_info.value.__cause__, shapely.errors.GEOSException)
