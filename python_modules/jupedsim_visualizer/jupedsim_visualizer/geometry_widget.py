# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import vtkmodules.vtkRenderingOpenGL2
from jupedsim_visualizer.config import Colors
from jupedsim_visualizer.geometry import Geometry, HoverInfo
from jupedsim_visualizer.grid import Grid
from jupedsim_visualizer.move_controller import MoveController
from PySide6.QtCore import Signal
from PySide6.QtGui import QPaintEvent
from PySide6.QtWidgets import QWidget
from vtkmodules.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import vtkRenderer

import jupedsim as jps
from jupedsim.internal.aabb import AABB


class RenderWidget(QVTKRenderWindowInteractor):
    on_hover_triangle = Signal(str)

    def __init__(
        self,
        geo: jps.Geometry,
        navi: jps.RoutingEngine,
        actor_sources,
        parent=None,
    ):
        QVTKRenderWindowInteractor.__init__(self, parent)
        self.navi = navi
        self.actor_sources = actor_sources

        self.ren = vtkRenderer()
        self.ren.SetBackground(Colors.d)
        self.GetRenderWindow().AddRenderer(self.ren)
        for source in self.actor_sources:
            for actor in source.get_actors():
                self.ren.AddActor(actor)
        self.iren = self.GetRenderWindow().GetInteractor()

        cam = self.ren.GetActiveCamera()
        cam.ParallelProjectionOn()

        style = vtkInteractorStyleUser()
        self.iren.SetInteractorStyle(style)
        self.iren.Initialize()

        self.move_controller = MoveController(style, cam)
        self.move_controller.set_navi(self.navi)
        self.hover_info = HoverInfo(geo, self.ren, style)
        self.hover_info.hovered.connect(self.on_hover_triangle)

        self.grid = Grid(self.ren, cam)
        self.reset_camera()

    def reset_camera(self):
        focal_pt_2d = (0, 0)
        scale = 10
        bounding_box = AABB.combine(
            *[s.get_bounds() for s in self.actor_sources]
        )
        focal_pt_2d = bounding_box.center
        (
            viewport_aspect_width,
            viewport_aspect_height,
        ) = self.ren.GetAspect()
        viewport_aspect_ratio = viewport_aspect_width / viewport_aspect_height
        scene_aspect_ratio = bounding_box.width / bounding_box.height

        if viewport_aspect_ratio > scene_aspect_ratio:
            scale = (bounding_box.height / 2) * 1.05
        else:
            scale = (bounding_box.width / 2) / viewport_aspect_ratio * 1.05

        cam = self.ren.GetActiveCamera()
        cam.SetParallelScale(scale)
        cam.SetFocalPoint(focal_pt_2d[0], focal_pt_2d[1], 0)
        cam.SetPosition(focal_pt_2d[0], focal_pt_2d[1], 100)
        cam.SetViewUp(0, 1, 0)
        cam.SetClippingRange(0, 200)
        self.iren.Render()

    def show_grid(self, state: bool):
        self.grid.show(state)

    def render(self):
        self.iren.Render()
