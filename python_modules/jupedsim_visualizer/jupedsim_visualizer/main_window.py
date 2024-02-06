# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import shapely
from jupedsim_visualizer.geometry import Geometry
from jupedsim_visualizer.replay_widget import ReplayWidget
from jupedsim_visualizer.trajectory import Trajectory
from jupedsim_visualizer.view_geometry_widget import ViewGeometryWidget
from PySide6.QtCore import QSettings, QSize
from PySide6.QtStateMachine import QFinalState, QState, QStateMachine
from PySide6.QtWidgets import (
    QApplication,
    QFileDialog,
    QMainWindow,
    QMessageBox,
    QTabWidget,
)

import jupedsim as jps
from jupedsim.recording import Recording


class MainWindow(QMainWindow):
    def __init__(self, parent=None) -> None:
        QMainWindow.__init__(self, parent)
        self.settings = QSettings("jupedsim", "jupedsim_visualizer")
        self.setWindowTitle("jupedsim_visualizer")
        self._build_central_tabs_widget()
        self._build_menu_bar()
        self._build_state_machine()
        self.setVisible(True)

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
        self._show_triangulation = settings_menu.addAction(
            "show triangulation"
        )
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
            polygon = shapely.from_wkt(Path(file).read_text(encoding="UTF-8"))
            navi = jps.RoutingEngine(polygon)
            xmin, ymin, xmax, ymax = polygon.bounds
            info_text = f"Dimensions: {math.ceil(xmax - xmin)}m x {math.ceil(ymax - ymin)}m Triangles: {len(navi.mesh())}"
            name_text = f"Geometry: {file}"
            self.setUpdatesEnabled(False)
            geo = Geometry(navi)
            geo.show_triangulation(self._show_triangulation.isChecked())
            tab = ViewGeometryWidget(
                navi, geo, name_text, info_text, parent=self
            )
            tab.render_widget.show_grid(self._show_grid.isChecked())
            tab_idx = self.tabs.insertTab(0, tab, file.name)
            self.tabs.setCurrentIndex(tab_idx)
            self.setUpdatesEnabled(True)
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
            rec = Recording(file.as_posix())
            self.setUpdatesEnabled(False)
            navi = jps.RoutingEngine(rec.geometry())
            geo = Geometry(navi)
            geo.show_triangulation(self._show_triangulation.isChecked())
            trajectory = Trajectory(rec)
            tab = ReplayWidget(navi, rec, geo, trajectory, parent=self)
            tab.render_widget.show_grid(self._show_grid.isChecked())
            tab_idx = self.tabs.insertTab(0, tab, file.name)
            self.tabs.setCurrentIndex(tab_idx)
            self.setUpdatesEnabled(True)
            self.update()
        except Exception as e:
            import traceback

            traceback.print_exception(e)
            QMessageBox.critical(
                self,
                "Error importing simulation recording",
                f"Error importing simulation recording:\n{e}",
            )
            return
