from jupedsim_visualizer.geometry import Geometry
from jupedsim_visualizer.geometry_widget import RenderWidget
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

import jupedsim.py_jupedsim as jps


class ViewGeometryWidget(QWidget):
    def __init__(
        self,
        navi: jps.experimental.RoutingEngine,
        geo: Geometry,
        name_text: str,
        info_text: str,
        parent=None,
    ):
        QWidget.__init__(self, parent)
        bottom_layout = QHBoxLayout()
        geometry_label = QLabel(name_text)
        geometry_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        bottom_layout.addWidget(geometry_label, 1, Qt.AlignmentFlag.AlignLeft)

        properties_label = QLabel(info_text)
        properties_label.setAlignment(Qt.AlignmentFlag.AlignRight)
        bottom_layout.addWidget(
            properties_label, 1, Qt.AlignmentFlag.AlignRight
        )

        layout = QVBoxLayout()

        reset_cam_bt = QPushButton("Reset Camera")
        layout.addWidget(reset_cam_bt)

        self.geo_widget = RenderWidget(geo, navi, [geo])
        layout.addWidget(self.geo_widget)

        self.hover_label = QLabel("")
        self.hover_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.hover_label)

        layout.addLayout(bottom_layout)
        self.setLayout(layout)

        reset_cam_bt.clicked.connect(self.geo_widget.reset_camera)
        self.geo_widget.on_hover_triangle.connect(self.hover_label.setText)
