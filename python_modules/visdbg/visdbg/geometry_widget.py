import py_jupedsim
import vtkmodules.vtkRenderingOpenGL2
from PySide6.QtCore import Signal
from PySide6.QtWidgets import QWidget
from visdbg.config import Colors
from visdbg.geometry import Geometry, HoverInfo
from visdbg.grid import Grid
from visdbg.move_controller import MoveController
from vtkmodules.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import vtkRenderer


class GeometryWidget(QVTKRenderWindowInteractor):
    on_hover_triangle = Signal(str)

    def __init__(
        self,
        navi: py_jupedsim.experimental.RoutingEngine,
        parent=None,
    ):
        QVTKRenderWindowInteractor.__init__(self, parent)
        self.navi = navi
        self.geo = Geometry(self.navi)

        self.ren = vtkRenderer()
        self.ren.SetBackground(Colors.d)
        self.GetRenderWindow().AddRenderer(self.ren)
        for actor in self.geo.get_actors():
            self.ren.AddActor(actor)
        self.iren = self.GetRenderWindow().GetInteractor()

        cam = self.ren.GetActiveCamera()
        cam.ParallelProjectionOn()

        style = vtkInteractorStyleUser()
        self.iren.SetInteractorStyle(style)
        self.iren.Initialize()

        self.move_controller = MoveController(style, cam)
        self.move_controller.set_navi(self.navi)
        self.hover_info = HoverInfo(self.ren, style)
        self.hover_info.hoveredTriangle.connect(self.on_hover_triangle)
        self.hover_info.set_geo(self.geo)

        self.grid = Grid(self.ren, cam)
        self.reset_camera()

    def reset_camera(self):
        focal_pt_2d = (0, 0)
        scale = 10
        bounds = self.geo.get_bounds()
        width = bounds[1] - bounds[0]
        height = bounds[3] - bounds[2]
        focal_pt_2d = (bounds[0] + width / 2, bounds[2] + height / 2)

        (
            viewport_aspect_width,
            viewport_aspect_height,
        ) = self.ren.GetAspect()
        viewport_aspect_ratio = viewport_aspect_width / viewport_aspect_height
        scene_aspect_ratio = width / height

        if viewport_aspect_ratio > scene_aspect_ratio:
            scale = (height / 2) * 1.05
        else:
            scale = (width / 2) / viewport_aspect_ratio * 1.05

        cam = self.ren.GetActiveCamera()
        cam.SetParallelScale(scale)
        cam.SetFocalPoint(focal_pt_2d[0], focal_pt_2d[1], 0)
        cam.SetPosition(focal_pt_2d[0], focal_pt_2d[1], 100)
        cam.SetViewUp(0, 1, 0)
        cam.SetClippingRange(0, 200)
        self.iren.Render()
