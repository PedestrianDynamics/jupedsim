import py_jupedsim
import vtkmodules.vtkRenderingOpenGL2
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)
from visdbg.config import Colors
from visdbg.geometry import Geometry, HoverInfo
from visdbg.grid import Grid
from visdbg.move_controller import MoveController
from vtkmodules.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import vtkRenderer


class GeometryWidget(QWidget):
    def __init__(
        self,
        navi: py_jupedsim.experimental.RoutingEngine,
        name_text: str,
        info_text: str,
        parent=None,
    ):
        QWidget.__init__(self, parent)
        self.navi = navi
        self.geo = Geometry(self.navi)

        self._build_content(name_text, info_text)

        self.ren = vtkRenderer()
        self.ren.SetBackground(Colors.d)
        self.vtk_rwi.GetRenderWindow().AddRenderer(self.ren)
        for actor in self.geo.get_actors():
            self.ren.AddActor(actor)
        self.iren = self.vtk_rwi.GetRenderWindow().GetInteractor()

        cam = self.ren.GetActiveCamera()
        cam.ParallelProjectionOn()

        self.showMaximized()
        style = vtkInteractorStyleUser()
        self.iren.SetInteractorStyle(style)
        self.iren.Initialize()

        self.move_controller = MoveController(style, cam)
        self.move_controller.set_navi(self.navi)
        self.hover_info = HoverInfo(self.ren, style)
        self.hover_info.hoveredTriangle.connect(self.hover_label.setText)
        self.hover_info.set_geo(self.geo)

        self.grid = Grid(self.ren, cam)
        self._reset_camera()

    def _build_content(self, name_text, info_text):
        geometry_label = QLabel(name_text)
        geometry_label.setAlignment(Qt.AlignmentFlag.AlignLeft)

        properties_label = QLabel(info_text)
        properties_label.setAlignment(Qt.AlignmentFlag.AlignRight)

        bottom_layout = QHBoxLayout()
        bottom_layout.addWidget(geometry_label, 1, Qt.AlignmentFlag.AlignLeft)
        bottom_layout.addWidget(
            properties_label, 1, Qt.AlignmentFlag.AlignRight
        )

        layout = QVBoxLayout()

        reset_cam_bt = QPushButton("Reset Camera")
        reset_cam_bt.clicked.connect(self._reset_camera)
        layout.addWidget(reset_cam_bt)

        self.vtk_rwi = QVTKRenderWindowInteractor(self)
        layout.addWidget(self.vtk_rwi)

        self.hover_label = QLabel("")
        self.hover_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.hover_label)

        layout.addLayout(bottom_layout)

        self.setLayout(layout)

    def _reset_camera(self):
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
