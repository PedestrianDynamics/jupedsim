# SPDX-License-Identifier: LGPL-3.0-or-later
import sys

import shapely
from PySide6.QtCore import QObject, Signal
from vtkmodules.vtkCommonCore import vtkCommand, vtkPoints
from vtkmodules.vtkCommonDataModel import vtkCellArray, vtkPolyData
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCellPicker,
    vtkPolyDataMapper,
    vtkRenderer,
)

from jupedsim_vis.aabb import AABB
from jupedsim_vis.config import Colors, ZLayers
from jupedsim_vis.triangulation import Triangulation, triangulate


def to_polydata(triangulation: Triangulation, z: float) -> vtkPolyData:
    """Turn an indexed triangle mesh into flat VTK polygons at height ``z``.

    The winding of the triangles is taken as it comes out of the
    triangulation: the scene is flat and nothing is culled.
    """
    points = vtkPoints()
    # Shapely computes in double precision; VTK's default float32 storage
    # would quantise the coordinates behind our back. Has to be set before
    # the array is allocated.
    points.SetDataTypeToDouble()
    points.SetNumberOfPoints(len(triangulation.points))
    for index, (x, y) in enumerate(triangulation.points):
        points.SetPoint(index, x, y, z)

    polys = vtkCellArray()
    for triangle in triangulation.triangles:
        polys.InsertNextCell(3)
        for index in triangle:
            polys.InsertCellPoint(index)

    poly_data = vtkPolyData()
    poly_data.SetPoints(points)
    poly_data.SetPolys(polys)
    return poly_data


def _count_polygons(geometry: shapely.Geometry) -> int:
    """Count the ``Polygon`` parts of ``geometry``, recursively."""
    if isinstance(geometry, shapely.Polygon):
        return 1
    if isinstance(geometry, shapely.MultiPolygon):
        return len(geometry.geoms)
    if isinstance(geometry, shapely.GeometryCollection):
        return sum(_count_polygons(part) for part in geometry.geoms)
    return 0


class Geometry:
    def __init__(self, geometry: shapely.Geometry):
        self.geometry = geometry
        triangulation = triangulate(geometry)
        self.num_polygons = _count_polygons(geometry)
        self.num_triangles = len(triangulation.triangles)

        poly_data = to_polydata(triangulation, ZLayers.geo)

        mapper = vtkPolyDataMapper()
        mapper.SetInputData(poly_data)

        actor = vtkActor()
        actor.SetMapper(mapper)
        actor.GetProperty().SetColor(Colors.c)
        actor.GetProperty().SetEdgeColor(Colors.a)
        self.actor = actor

    def get_actors(self):
        return [self.actor]

    def get_bounds(self) -> AABB:
        xmin = sys.maxsize
        ymin = sys.maxsize
        xmax = ~sys.maxsize
        ymax = ~sys.maxsize
        for actor in self.get_actors():
            bounds = actor.GetBounds()
            xmin = min(xmin, bounds[0])
            xmax = max(xmax, bounds[1])
            ymin = min(ymin, bounds[2])
            ymax = max(ymax, bounds[3])
        return AABB(xmin=xmin, ymin=ymin, xmax=xmax, ymax=ymax)

    def show_triangulation(self, state: bool) -> None:
        self.actor.GetProperty().SetEdgeVisibility(state)
        self.actor.Modified()


def _hover_text(x: float, y: float, cell_id: int, dist: float | None) -> str:
    """Build the hover label shown in the status bar.

    The optional parts are joined, not interpolated, so a missing triangle
    or path length leaves no double or trailing space behind.
    """
    parts = [f"x: {x:.2f} y: {y:.2f}"]
    if cell_id != -1:
        parts.append(f"Triangle: {cell_id}")
    if dist is not None:
        parts.append(f"Path length: {dist:.2f}m")
    return " ".join(parts)


class HoverInfo(QObject):
    hovered = Signal(str)

    def __init__(
        self,
        geo: Geometry,
        renderer: vtkRenderer,
        interactor_style: vtkInteractorStyleUser,
        move_controller=None,
    ):
        QObject.__init__(self)
        self.geo = geo
        self.renderer = renderer
        self.picker = vtkCellPicker()
        self.picker.PickFromListOn()
        interactor_style.AddObserver(
            vtkCommand.MouseMoveEvent, self.on_mouse_move
        )
        self.picker.InitializePickList()
        self.picker.AddPickList(self.geo.actor)
        self.move_controller = move_controller

    def on_mouse_move(self, obj, evt):
        interactor = obj.GetInteractor()
        pos = interactor.GetEventPosition()
        self.picker.Pick(pos[0], pos[1], 0, self.renderer)
        cell_id = self.picker.GetCellId()
        x, y, _ = self.picker.GetPickPosition()
        dist = None
        if (
            self.move_controller is not None
            and self.move_controller.router is not None
        ):
            dist = self.move_controller.dist
        self.hovered.emit(_hover_text(x, y, cell_id, dist))
