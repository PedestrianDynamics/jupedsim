// SPDX-License-Identifier: LGPL-3.0-or-later
import sys

from jupedsim import RoutingEngine
from jupedsim.internal.aabb import AABB
from PySide6.QtCore import QObject, Signal
from vtkmodules.vtkCommonCore import vtkCommand, vtkPoints
from vtkmodules.vtkCommonDataModel import vtkCellArray, vtkPolyData, vtkPolygon
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCellPicker,
    vtkPolyDataMapper,
    vtkRenderer,
)

from jupedsim_visualizer.config import Colors, ZLayers


class Geometry:
    def __init__(self, navi: RoutingEngine):
        self.navi = navi
        vertices = vtkPoints()
        polygons = vtkCellArray()

        mesh = self.navi.mesh()

        for v in mesh[0]:
            vertices.InsertNextPoint(v[0], v[1], ZLayers.geo)

        for idx, p in enumerate(mesh[1]):
            poly = vtkPolygon()
            poly.GetPointIds().SetNumberOfIds(len(p))
            for idx, idx_p in enumerate(p):
                poly.GetPointIds().SetId(idx, idx_p)
            polygons.InsertNextCell(poly)

        poly_data = vtkPolyData()
        poly_data.SetPoints(vertices)
        poly_data.SetPolys(polygons)

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
        cell_text = str(f"Nav ID: {cell_id}" if cell_id != -1 else "")
        path_text = ""
        if self.move_controller:
            path_text = str(f"Path length: {self.move_controller.dist}")
        text = f"x: {x:.2f} y: {y:.2f} {cell_text} {path_text}"
        self.hovered.emit(text)
