# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from dataclasses import dataclass
from pathlib import Path

import shapely
from PySide6.QtCore import QSettings, QSize
from PySide6.QtStateMachine import QFinalState, QState, QStateMachine
from PySide6.QtWidgets import (
    QApplication,
    QFileDialog,
    QMainWindow,
    QMessageBox,
    QStatusBar,
    QTabWidget,
)

from jupedsim_vis.geometry import Geometry
from jupedsim_vis.recording import Recording
from jupedsim_vis.replay_widget import ReplayWidget
from jupedsim_vis.routing import Routing, check_available, create_router
from jupedsim_vis.trajectory import Trajectory
from jupedsim_vis.view_geometry_widget import ViewGeometryWidget


@dataclass
class GeometryDocument:
    """A loaded WKT geometry, everything a geometry tab needs."""

    path: Path
    geometry: shapely.Geometry
    geo: Geometry
    routing: Routing
    name_text: str
    info_text: str


@dataclass
class RecordingDocument:
    """A loaded recording, everything a replay tab needs."""

    path: Path
    recording: Recording
    geo: Geometry
    trajectory: Trajectory
    routing: Routing


class MainWindow(QMainWindow):
    def __init__(self, parent=None) -> None:
        QMainWindow.__init__(self, parent)
        self.settings = QSettings("jupedsim", "jupedsim_vis")
        self.setWindowTitle("jupedsim_vis")
        self.documents: list[GeometryDocument | RecordingDocument] = []
        #: Why routing is unavailable regardless of any geometry, or None.
        self._global_routing_reason = check_available()
        self._build_central_tabs_widget()
        self._build_menu_bar()
        self._build_status_bar()
        self._build_state_machine()
        self.setVisible(True)
        self._update_status_bar()

    def _build_central_tabs_widget(self):
        tabs = QTabWidget(self)
        tabs.setMinimumSize(QSize(640, 480))
        tabs.setMovable(True)
        tabs.setDocumentMode(True)
        tabs.setTabsClosable(True)
        tabs.setTabBarAutoHide(True)
        tabs.tabCloseRequested.connect(tabs.removeTab)
        self.setCentralWidget(tabs)
        self.tabs = tabs

    def _build_menu_bar(self) -> None:
        menu = self.menuBar()
        open_menu = menu.addMenu("File")
        open_wkt_act = open_menu.addAction("Open wkt file")
        open_wkt_act.triggered.connect(self._open_wkt)
        open_replay_act = open_menu.addAction("Open replay file")
        open_replay_act.triggered.connect(self._open_replay)
        settings_menu = menu.addMenu("Settings")
        self._show_triangulation = settings_menu.addAction("show triangulation")
        self._show_triangulation.setCheckable(True)
        self._show_triangulation.toggled.connect(self._toggle_triangulation)
        self._show_triangulation.setChecked(
            bool(
                self.settings.value(
                    "show_triangulation", type=bool, defaultValue=False
                )
            )
        )
        self._show_grid = settings_menu.addAction("show grid")
        self._show_grid.setCheckable(True)
        self._show_grid.toggled.connect(self._toggle_grid)
        self._show_grid.setChecked(
            bool(
                self.settings.value("show_grid", type=bool, defaultValue=False)
            )
        )

    def _build_status_bar(self) -> None:
        self.setStatusBar(QStatusBar(self))
        self.tabs.currentChanged.connect(self._update_status_bar)

    def _update_status_bar(self, *_) -> None:
        """Show why routing is unavailable, for the current tab or globally.

        The message is persistent (no timeout); it disappears as soon as a
        tab with a working router becomes current.
        """
        tab = self.tabs.currentWidget()
        reason = getattr(tab, "routing_status", None)
        if not reason:
            reason = self._global_routing_reason
        if reason:
            self.statusBar().showMessage(reason)
        else:
            self.statusBar().clearMessage()

    def _build_state_machine(self) -> None:
        sm = QStateMachine(self)
        sm.finished.connect(QApplication.quit)

        start = self._build_start_state()
        sm.addState(start)

        exit = self._build_exit_state()
        sm.addState(exit)

        # start.addTransition(self.button.clicked, exit)

        sm.setInitialState(start)
        sm.start()
        self.state_machine = sm

    def _build_start_state(self) -> QState:
        state = QState()
        return state

    def _build_show_wkt_state(self) -> QState:
        state = QState()
        return state

    def _build_exit_state(self) -> QFinalState:
        state = QFinalState()
        return state

    def _toggle_triangulation(self, state: bool) -> None:
        self.settings.setValue("show_triangulation", state)
        for idx in range(self.tabs.count()):
            self.tabs.widget(idx).geo.show_triangulation(state)
        self.repaint()

    def _toggle_grid(self, state: bool) -> None:
        self.settings.setValue("show_grid", state)
        for idx in range(self.tabs.count()):
            self.tabs.widget(idx).render_widget.show_grid(state)
        self.repaint()

    def load_wkt(self, path: Path) -> GeometryDocument:
        """Load a WKT geometry file. No Qt widget, no OpenGL context.

        Raises whatever shapely or the triangulation raise on bad input.
        """
        path = Path(path)
        geometry = shapely.from_wkt(path.read_text(encoding="UTF-8"))
        xmin, ymin, xmax, ymax = geometry.bounds
        geo = Geometry(geometry)
        doc = GeometryDocument(
            path=path,
            geometry=geometry,
            geo=geo,
            routing=create_router(geometry),
            name_text=f"Geometry: {path}",
            info_text=(
                f"Dimensions: {math.ceil(xmax - xmin)}m x "
                f"{math.ceil(ymax - ymin)}m  "
                f"Polygons: {geo.num_polygons}  "
                f"Triangles: {geo.num_triangles}"
            ),
        )
        self.documents.append(doc)
        return doc

    def load_recording(self, path: Path) -> RecordingDocument:
        """Load a recording. No Qt widget, no OpenGL context.

        Raises ``RecordingError`` for anything the reader rejects.
        """
        path = Path(path)
        recording = Recording(path)
        geometry = recording.geometry()
        doc = RecordingDocument(
            path=path,
            recording=recording,
            geo=Geometry(geometry),
            trajectory=Trajectory(recording),
            routing=create_router(geometry),
        )
        self.documents.append(doc)
        return doc

    def add_geometry_tab(self, doc: GeometryDocument) -> int:
        """Create the view tab for ``doc``. Needs an OpenGL context."""
        self.setUpdatesEnabled(False)
        try:
            doc.geo.show_triangulation(self._show_triangulation.isChecked())
            tab = ViewGeometryWidget(
                doc.geo,
                doc.name_text,
                doc.info_text,
                routing=doc.routing,
                parent=self,
            )
            tab.render_widget.show_grid(self._show_grid.isChecked())
            tab_idx = self.tabs.insertTab(0, tab, doc.path.name)
            self.tabs.setCurrentIndex(tab_idx)
        finally:
            self.setUpdatesEnabled(True)
        self._update_status_bar()
        return tab_idx

    def add_recording_tab(self, doc: RecordingDocument) -> int:
        """Create the replay tab for ``doc``. Needs an OpenGL context."""
        self.setUpdatesEnabled(False)
        try:
            doc.geo.show_triangulation(self._show_triangulation.isChecked())
            tab = ReplayWidget(
                doc.recording,
                doc.geo,
                doc.trajectory,
                routing=doc.routing,
                parent=self,
            )
            tab.render_widget.show_grid(self._show_grid.isChecked())
            tab_idx = self.tabs.insertTab(0, tab, doc.path.name)
            self.tabs.setCurrentIndex(tab_idx)
        finally:
            self.setUpdatesEnabled(True)
        self.update()
        self._update_status_bar()
        return tab_idx

    def open_wkt_file(
        self, path: Path, *, with_view: bool = True
    ) -> GeometryDocument:
        """Load a WKT geometry file and, unless told otherwise, show it."""
        doc = self.load_wkt(path)
        if with_view:
            self.add_geometry_tab(doc)
        return doc

    def open_recording_file(
        self, path: Path, *, with_view: bool = True
    ) -> RecordingDocument:
        """Load a recording and, unless told otherwise, show it."""
        doc = self.load_recording(path)
        if with_view:
            self.add_recording_tab(doc)
        return doc

    def _open_wkt(self):
        base_path_obj = self.settings.value(
            "files/last_wkt_location",
            type=str,
            defaultValue=Path("~").expanduser(),
        )
        base_path = Path(str(base_path_obj))
        file, _ = QFileDialog.getOpenFileName(
            self, caption="Open WKT file", dir=str(base_path)
        )
        if not file:
            return
        file = Path(file)
        self.settings.setValue("files/last_wkt_location", str(file.parent))
        try:
            self.open_wkt_file(file)
        except Exception as e:
            QMessageBox.critical(
                self,
                "Error importing WKT geometry",
                f"Error importing WKT geometry:\n{e}",
            )
            return

    def _open_replay(self):
        base_path_obj = self.settings.value(
            "files/last_replay_location",
            type=str,
            defaultValue=Path("~").expanduser(),
        )
        base_path = Path(str(base_path_obj))
        file, _ = QFileDialog.getOpenFileName(
            self, caption="Open recording", dir=str(base_path)
        )
        if not file:
            return
        file = Path(file)
        self.settings.setValue("files/last_replay_location", str(file.parent))
        try:
            self.open_recording_file(file)
        except Exception as e:
            QMessageBox.critical(
                self,
                "Error importing simulation recording",
                f"Error importing simulation recording:\n{e}",
            )
            return
