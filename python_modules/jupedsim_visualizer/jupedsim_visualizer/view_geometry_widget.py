# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from jupedsim_visualizer.geometry import Geometry
from jupedsim_visualizer.geometry_widget import RenderWidget
from PySide6.QtCore import Qt
from PySide6.QtGui import QPaintEvent
from PySide6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

import jupedsim as jps


class ViewGeometryWidget(QWidget):
    def __init__(
        self,
        navi: jps.RoutingEngine,
        geo: Geometry,
        name_text: str,
        info_text: str,
        parent=None,
    ):
        QWidget.__init__(self, parent)
        self.geo = geo
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

        self.render_widget = RenderWidget(geo, navi, [geo], parent=self)
        layout.addWidget(self.render_widget)

        self.hover_label = QLabel("")
        self.hover_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.hover_label)

        layout.addLayout(bottom_layout)
        self.setLayout(layout)

        reset_cam_bt.clicked.connect(self.render_widget.reset_camera)
        self.render_widget.on_hover_triangle.connect(self.hover_label.setText)

    def render(self):
        self.render_widget.render()

    def paintEvent(self, event: QPaintEvent) -> None:
        self.render()
        return super().paintEvent(event)
