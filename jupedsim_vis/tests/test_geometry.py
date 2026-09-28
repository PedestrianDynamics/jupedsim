# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the shapely backed geometry actor.

These tests build VTK data objects, mappers and actors, which needs no
OpenGL context. No render window and no ``RenderWidget`` is created here.
"""

import pytest
import shapely
from vtkmodules.util.vtkConstants import VTK_DOUBLE
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import vtkCamera

from jupedsim_vis.config import ZLayers
from jupedsim_vis.geometry import Geometry, _hover_text, to_polydata
from jupedsim_vis.move_controller import MoveController
from jupedsim_vis.triangulation import (
    TriangulationError,
    triangulate,
)

UNIT_SQUARE = "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))"
SQUARE_WITH_HOLE = (
    "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(4 4, 6 4, 6 6, 4 6, 4 4))"
)
TWO_SQUARES = (
    "MULTIPOLYGON(((0 0, 1 0, 1 1, 0 1, 0 0)),((3 0, 4 0, 4 1, 3 1, 3 0)))"
)
COLLECTION_WITH_ONE_POLYGON = (
    "GEOMETRYCOLLECTION(POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)))"
)
COLLECTION_WITH_MULTIPOLYGON = (
    "GEOMETRYCOLLECTION(MULTIPOLYGON(((0 0, 1 0, 1 1, 0 1, 0 0)),"
    "((3 0, 4 0, 4 1, 3 1, 3 0))),POINT(9 9))"
)


def test_geometry_triangulates_polygon_with_hole():
    geo = Geometry(shapely.from_wkt(SQUARE_WITH_HOLE))

    assert geo.num_triangles == 8
    assert geo.num_polygons == 1


def test_geometry_keeps_the_input_geometry():
    geometry = shapely.from_wkt(SQUARE_WITH_HOLE)

    geo = Geometry(geometry)

    assert geo.geometry is geometry


def test_geometry_bounds_match_the_input_bounds():
    geo = Geometry(shapely.from_wkt(SQUARE_WITH_HOLE))

    bounds = geo.get_bounds()

    assert bounds.xmin == pytest.approx(0.0)
    assert bounds.xmax == pytest.approx(10.0)
    assert bounds.ymin == pytest.approx(0.0)
    assert bounds.ymax == pytest.approx(10.0)


def test_geometry_actors_contain_the_geometry_actor():
    geo = Geometry(shapely.from_wkt(UNIT_SQUARE))

    assert geo.get_actors() == [geo.actor]


def test_show_triangulation_toggles_edge_visibility():
    geo = Geometry(shapely.from_wkt(UNIT_SQUARE))

    geo.show_triangulation(True)
    assert geo.actor.GetProperty().GetEdgeVisibility()

    geo.show_triangulation(False)
    assert not geo.actor.GetProperty().GetEdgeVisibility()


def test_num_polygons_counts_multi_polygon_parts():
    geo = Geometry(shapely.from_wkt(TWO_SQUARES))

    assert geo.num_polygons == 2


def test_num_polygons_counts_polygons_in_a_collection():
    geo = Geometry(shapely.from_wkt(COLLECTION_WITH_ONE_POLYGON))

    assert geo.num_polygons == 1


def test_num_polygons_counts_polygons_nested_in_a_collection():
    geo = Geometry(shapely.from_wkt(COLLECTION_WITH_MULTIPOLYGON))

    assert geo.num_polygons == 2


def test_geometry_propagates_triangulation_error_for_empty_input():
    with pytest.raises(TriangulationError):
        Geometry(shapely.from_wkt("POLYGON EMPTY"))


def test_to_polydata_builds_points_and_polys():
    triangulation = triangulate(shapely.from_wkt(UNIT_SQUARE))

    poly_data = to_polydata(triangulation, 3.5)

    assert poly_data.GetNumberOfPoints() == 4
    assert poly_data.GetNumberOfPolys() == 2
    for i in range(poly_data.GetNumberOfPoints()):
        assert poly_data.GetPoint(i)[2] == pytest.approx(3.5)


def test_to_polydata_keeps_point_coordinates_and_indices():
    triangulation = triangulate(shapely.from_wkt(UNIT_SQUARE))

    poly_data = to_polydata(triangulation, ZLayers.geo)

    # Double precision storage, so the coordinates come back bit for bit.
    assert poly_data.GetPoints().GetDataType() == VTK_DOUBLE
    for index, (x, y) in enumerate(triangulation.points):
        point = poly_data.GetPoint(index)
        assert (point[0], point[1]) == (x, y)

    cell = poly_data.GetCell(0)
    assert cell.GetNumberOfPoints() == 3
    ids = [cell.GetPointId(i) for i in range(3)]
    assert tuple(ids) == triangulation.triangles[0]


def test_hover_text_without_pick_has_no_triangle():
    text = _hover_text(1.0, 2.0, -1, None)

    assert "Triangle" not in text
    # No leftover separator where the optional parts would have gone.
    assert text == "x: 1.00 y: 2.00"


def test_hover_text_with_pick_shows_the_triangle_id():
    text = _hover_text(1.0, 2.0, 3, None)

    assert text == "x: 1.00 y: 2.00 Triangle: 3"


def test_hover_text_without_distance_has_no_path_length():
    text = _hover_text(1.0, 2.0, 3, None)

    assert "Path length" not in text


def test_hover_text_with_distance_shows_two_decimals():
    text = _hover_text(1.0, 2.0, 3, 12.3456)

    assert text == "x: 1.00 y: 2.00 Triangle: 3 Path length: 12.35m"


def test_hover_text_with_a_distance_but_no_pick():
    text = _hover_text(1.0, 2.0, -1, 12.3456)

    assert text == "x: 1.00 y: 2.00 Path length: 12.35m"


def test_move_controller_without_router_ignores_lmb():
    class ExplodingObject:
        def GetInteractor(self):
            raise AssertionError(
                "MoveController must not touch the interactor without a router"
            )

    controller = MoveController(vtkInteractorStyleUser(), vtkCamera())
    controller.set_router(None)

    assert controller.router is None
    controller._on_lmb_pressed(ExplodingObject(), None)


def test_move_controller_set_router_stores_the_router():
    class StubRouter:
        def is_routable(self, p):
            return True

        def compute_waypoints(self, frm, to):
            return [frm, to]

    router = StubRouter()
    controller = MoveController(vtkInteractorStyleUser(), vtkCamera())
    controller.set_router(router)

    assert controller.router is router
