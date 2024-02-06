# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import sys

from jupedsim_visualizer.config import Colors, ZLayers
from PySide6.QtCore import QObject, Signal
from vtkmodules.vtkCommonCore import vtkCommand, vtkIntArray, vtkPoints
from vtkmodules.vtkCommonDataModel import (
    vtkCellArray,
    vtkPolyData,
    vtkTriangle,
)
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCellPicker,
    vtkPolyDataMapper,
    vtkRenderer,
)

from jupedsim import RoutingEngine
from jupedsim.internal.aabb import AABB


class Geometry:
    def __init__(self, navi: RoutingEngine):
        self.navi = navi
        triangle_points = vtkPoints()
        triangles = vtkCellArray()
        triangle_data = vtkIntArray()

        for idx, tri in enumerate(self.navi.mesh()):
            triangle_points.InsertNextPoint(tri[0][0], tri[0][1], ZLayers.geo)
            triangle_points.InsertNextPoint(tri[1][0], tri[1][1], ZLayers.geo)
            triangle_points.InsertNextPoint(tri[2][0], tri[2][1], ZLayers.geo)
            triangle = vtkTriangle()
            triangle.GetPointIds().SetId(0, idx * 3)
            triangle.GetPointIds().SetId(1, idx * 3 + 1)
            triangle.GetPointIds().SetId(2, idx * 3 + 2)
            triangles.InsertNextCell(triangle)
            triangle_data.InsertNextValue(idx)

        triangle_poly_data = vtkPolyData()
        triangle_poly_data.SetPoints(triangle_points)
        triangle_poly_data.SetPolys(triangles)
        triangle_poly_data.GetCellData().AddArray(triangle_data)

        triangle_mapper = vtkPolyDataMapper()
        triangle_mapper.SetInputData(triangle_poly_data)

        actor = vtkActor()
        actor.SetMapper(triangle_mapper)
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


class HoverInfo(QObject):
    hovered = Signal(str)

    def __init__(
        self,
        geo: Geometry,
        renderer: vtkRenderer,
        interactor_style: vtkInteractorStyleUser,
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

    def on_mouse_move(self, obj, evt):
        interactor = obj.GetInteractor()
        pos = interactor.GetEventPosition()
        self.picker.Pick(pos[0], pos[1], 0, self.renderer)
        cell_id = self.picker.GetCellId()
        x, y, _ = self.picker.GetPickPosition()
        cell_text = str(f"Nav ID: {cell_id}" if cell_id != -1 else "")
        text = f"x: {x:.2f} y: {y:.2f} {cell_text}"
        self.hovered.emit(text)
