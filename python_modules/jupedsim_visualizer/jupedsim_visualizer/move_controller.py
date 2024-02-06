# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from jupedsim_visualizer.config import ZLayers
from vtkmodules.vtkCommonCore import vtkCommand, vtkPoints
from vtkmodules.vtkCommonDataModel import (
    vtkCellArray,
    vtkPolyData,
    vtkPolyLine,
)
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import vtkActor, vtkCamera, vtkPolyDataMapper

from jupedsim import RoutingEngine


class MoveController:
    def __init__(
        self, interactor_style: vtkInteractorStyleUser, cam: vtkCamera
    ):
        self.route_from = None
        self.route_to = None
        self.lmb_pressed = False
        self.cam = cam
        self.navi = None
        self.actor = None
        self.interactor_style = interactor_style
        interactor_style.AddObserver(vtkCommand.CharEvent, self._on_char)

        interactor_style.AddObserver(
            vtkCommand.KeyPressEvent, self._ignore_evt
        )
        interactor_style.AddObserver(
            vtkCommand.KeyReleaseEvent, self._ignore_evt
        )
        interactor_style.AddObserver(
            vtkCommand.LeftButtonPressEvent, self._on_lmb_pressed
        )
        interactor_style.AddObserver(
            vtkCommand.LeftButtonReleaseEvent, self._on_lmb_released
        )
        interactor_style.AddObserver(
            vtkCommand.MouseMoveEvent, self._on_mouse_move
        )

    def set_navi(self, navi: RoutingEngine | None):
        self.navi = navi

    def _on_char(self, obj, evt):
        char = chr(obj.GetChar())

        def offset():
            return self.cam.GetParallelScale() * 0.035

        if char == "e":
            self.cam.SetParallelScale(self.cam.GetParallelScale() * 0.95)
        elif char == "q":
            self.cam.SetParallelScale(self.cam.GetParallelScale() * 1.05)
        elif char == "w":
            (x, y, z) = self.cam.GetPosition()
            y += offset()
            self.cam.SetPosition(x, y, z)
            self.cam.SetFocalPoint(x, y, 0)
        elif char == "s":
            (x, y, z) = self.cam.GetPosition()
            y -= offset()
            self.cam.SetPosition(x, y, z)
            self.cam.SetFocalPoint(x, y, 0)
        elif char == "a":
            (x, y, z) = self.cam.GetPosition()
            x -= offset()
            self.cam.SetPosition(x, y, z)
            self.cam.SetFocalPoint(x, y, 0)
        elif char == "d":
            (x, y, z) = self.cam.GetPosition()
            x += offset()
            self.cam.SetPosition(x, y, z)
            self.cam.SetFocalPoint(x, y, 0)
        obj.GetInteractor().Render()

    def _ignore_evt(self, obj, evt):
        pass

    def _on_lmb_pressed(self, obj, evt):
        if not self.navi:
            return
        interactor = obj.GetInteractor()
        if not self.lmb_pressed:
            self.lmb_pressed = True
            display_pos = interactor.GetEventPosition()
            world_pos = self._to_world_coordinate_2d(display_pos)
            self.route_from = world_pos

    def _on_mouse_move(self, obj, evt):
        interactor = obj.GetInteractor()
        if not self.lmb_pressed:
            return

        if not self.navi:
            return
        self.lmb_pressed = True
        display_pos = interactor.GetEventPosition()
        world_pos = self._to_world_coordinate_2d(display_pos)
        self.route_to = world_pos
        self._render_path()

    def _on_lmb_released(self, obj, evt):
        self.lmb_pressed = False
        self.route_from = None
        self.route_to = None

    def _render_path(self):
        if not self.navi:
            return

        interactor = self.interactor_style.GetInteractor()
        renderer = (
            interactor.GetRenderWindow().GetRenderers().GetFirstRenderer()
        )

        if (
            not self.route_to
            or not self.route_from
            or not self.navi.is_routable(self.route_from)
            or not self.navi.is_routable(self.route_to)
        ):
            if self.actor:
                renderer.RemoveActor(self.actor)
                self.actor = None
            return

        points = self.navi.compute_waypoints(self.route_from, self.route_to)
        vtk_points = vtkPoints()
        polyline = vtkPolyLine()
        polyline.GetPointIds().SetNumberOfIds(len(points))
        for idx, pt in enumerate(points):
            vtk_points.InsertNextPoint(pt[0], pt[1], ZLayers.nav_line)
            polyline.GetPointIds().SetId(idx, idx)
        poly_data = vtkPolyData()
        poly_data.SetPoints(vtk_points)
        cells = vtkCellArray()
        cells.InsertNextCell(polyline)
        poly_data.SetLines(cells)
        mapper = vtkPolyDataMapper()
        mapper.SetInputData(poly_data)
        if self.actor:
            renderer.RemoveActor(self.actor)
        self.actor = vtkActor()
        self.actor.SetMapper(mapper)
        self.actor.GetProperty().SetColor(1, 0, 0)
        self.actor.GetProperty().SetLineWidth(3)
        renderer.AddActor(self.actor)
        interactor.Render()

    def _to_world_coordinate_2d(
        self, display_pos: tuple[float, float]
    ) -> tuple[float, float]:
        renderer = (
            self.interactor_style.GetInteractor()
            .GetRenderWindow()
            .GetRenderers()
            .GetFirstRenderer()
        )
        renderer.SetDisplayPoint(display_pos[0], display_pos[1], 0)
        renderer.DisplayToWorld()
        world = renderer.GetWorldPoint()
        return (world[0] / world[3], world[1] / world[3])
