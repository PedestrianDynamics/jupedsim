# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
from PySide6.QtCore import Qt
from PySide6.QtGui import QPaintEvent
from PySide6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from jupedsim_visualizer.geometry import Geometry
from jupedsim_visualizer.geometry_widget import RenderWidget


class ViewGeometryWidget(QWidget):
    def __init__(
        self,
        navi: jps.TAStarRoutingEngine,
        geo: Geometry,
        name_text: str,
        info_text: str,
        parent=None,
    ):
        QWidget.__init__(self, parent)
        self.geo = geo
        # Engines selectable from the toolbar combo, keyed by display name.
        # Engines exposing a ``mesh()`` route on the triangulation and may
        # show it; DirectPath has no mesh and never does.
        self._engines = {
            "AStar": navi,
            # Pure-Python oracle on the *same* nav-mesh as native AStar.
            "PythonTAStar": jps.PythonTAStarRoutingEngine.from_mesh(
                *navi.mesh()
            ),
            "DirectPath": jps.DirectPathRoutingEngine(),
        }
        self._show_triangulation_requested = False

        bottom_layout = QHBoxLayout()
        geometry_label = QLabel(name_text)
        geometry_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        bottom_layout.addWidget(geometry_label, 1, Qt.AlignmentFlag.AlignLeft)

        properties_label = QLabel(info_text)
        properties_label.setAlignment(Qt.AlignmentFlag.AlignRight)
        bottom_layout.addWidget(
            properties_label, 1, Qt.AlignmentFlag.AlignRight
        )

        toolbar_layout = QHBoxLayout()

        routing_combo = QComboBox()
        routing_combo.addItems(list(self._engines))
        toolbar_layout.addWidget(routing_combo, 1)
        self._routing_combo = routing_combo

        reset_cam_bt = QPushButton("Reset Camera")
        toolbar_layout.addWidget(reset_cam_bt, 2)

        layout = QVBoxLayout()
        layout.addLayout(toolbar_layout)

        self.render_widget = RenderWidget(geo, navi, [geo], parent=self)
        layout.addWidget(self.render_widget)

        self.hover_label = QLabel("")
        self.hover_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.hover_label)

        layout.addLayout(bottom_layout)
        self.setLayout(layout)

        reset_cam_bt.clicked.connect(self.render_widget.reset_camera)
        self.render_widget.on_hover_triangle.connect(self.hover_label.setText)
        routing_combo.currentTextChanged.connect(
            self._on_routing_engine_changed
        )

    def _on_routing_engine_changed(self, engine_name: str) -> None:
        navi = self._engines[engine_name]
        # Only mesh-based engines have a triangulation worth showing.
        show = self._show_triangulation_requested and hasattr(navi, "mesh")
        self.geo.show_triangulation(show)
        self.render_widget.set_routing_engine(navi)

    def set_triangulation_visible(self, state: bool) -> None:
        self._show_triangulation_requested = state
        navi = self._engines[self._routing_combo.currentText()]
        if hasattr(navi, "mesh"):
            self.geo.show_triangulation(state)

    def render(self):
        self.render_widget.render()

    def paintEvent(self, event: QPaintEvent) -> None:
        self.render()
        return super().paintEvent(event)
