# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import vtkmodules.vtkRenderingOpenGL2  # noqa: F401
from jupedsim.internal.aabb import AABB
from PySide6.QtCore import QTimer, Signal
from PySide6.QtGui import QShowEvent
from vtkmodules.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import vtkRenderer

from jupedsim_visualizer.config import Colors
from jupedsim_visualizer.geometry import HoverInfo
from jupedsim_visualizer.grid import Grid
from jupedsim_visualizer.move_controller import MoveController


class RenderWidget(QVTKRenderWindowInteractor):
    on_hover_triangle = Signal(str)

    def __init__(
        self,
        geo: jps.Geometry,
        navi: jps.RoutingEngine,
        actor_sources,
        parent=None,
    ):
        print(f"DEBUG RenderWidget.__init__ start, self={id(self)}, parent={id(parent) if parent else None}")
        QVTKRenderWindowInteractor.__init__(self)
        print("DEBUG RenderWidget.__init__ after QVTKRenderWindowInteractor")
        self.navi = navi
        self.actor_sources = actor_sources
        self._vtk_initialized = False

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
        print("DEBUG RenderWidget.__init__ before defer Initialize")
        QTimer.singleShot(1, self._deferred_init)
        print("DEBUG RenderWidget.__init__ after defer Initialize")

        self.move_controller = MoveController(style, cam)
        self.hover_info = HoverInfo(geo, self.ren, style, self.move_controller)
        self.hover_info.hovered.connect(self.on_hover_triangle)

        self.grid = Grid(self.ren, cam)
        print("DEBUG RenderWidget.__init__ end")

    def _deferred_init(self):
        print(f"DEBUG RenderWidget._deferred_init start, window: {self.winId()}")
        self.iren.Initialize()
        print("DEBUG RenderWidget._deferred_init after iren.Initialize()")

    def showEvent(self, event: QShowEvent) -> None:
        print(f"DEBUG RenderWidget.showEvent, self={id(self)}, _vtk_initialized={self._vtk_initialized}")
        if not self._vtk_initialized:
            print("DEBUG RenderWidget.showEvent calling reset_camera")
            self.reset_camera()
            self._vtk_initialized = True
        return super().showEvent(event)

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
        print(f"DEBUG RenderWidget.render, _vtk_initialized={self._vtk_initialized}")
        if self._vtk_initialized:
            self.iren.Render()
