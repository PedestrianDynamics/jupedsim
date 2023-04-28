import math
from pathlib import Path

import py_jupedsim
from jupedsim.serialization import parse_wkt
from PySide6.QtCore import QSize
from PySide6.QtStateMachine import QFinalState, QState, QStateMachine
from PySide6.QtWidgets import (
    QApplication,
    QFileDialog,
    QMainWindow,
    QMessageBox,
    QTabWidget,
)
from visdbg.replay_widget import ReplayWidget
from visdbg.util import build_jps_geometry
from visdbg.view_geometry_widget import ViewGeometryWidget


def full_stack():
    import sys
    import traceback

    exc = sys.exc_info()[0]
    if exc is not None:
        f = sys.exc_info()[-1].tb_frame.f_back
        stack = traceback.extract_stack(f)
    else:
        stack = traceback.extract_stack()[
            :-1
        ]  # last one would be full_stack()
    trc = "Traceback (most recent call last):\n"
    stackstr = trc + "".join(traceback.format_list(stack))
    if exc is not None:
        stackstr += "  " + traceback.format_exc().lstrip(trc)
    return stackstr


class MainWindow(QMainWindow):
    def __init__(self, parent=None) -> None:
        QMainWindow.__init__(self, parent)
        self.setWindowTitle("visdbg")
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

    def _open_wkt(self):
        file, _ = QFileDialog.getOpenFileName(
            self, caption="Open WKT file", dir=str(Path("~").expanduser())
        )
        if not file:
            return
        file = Path(file)
        try:
            wkt = parse_wkt(Path(file).read_text(encoding="UTF-8"))
            navi = py_jupedsim.experimental.RoutingEngine(
                build_jps_geometry(wkt)
            )
            xmin, ymin, xmax, ymax = wkt.bounds
            info_text = f"Dimensions: {math.ceil(xmax - xmin)}m x {math.ceil(ymax-ymin)}m Triangles: {len(navi.mesh())}"
            name_text = f"Geometry: {file}"
            self.setUpdatesEnabled(False)
            tab = ViewGeometryWidget(navi, name_text, info_text)
            tab_idx = self.tabs.insertTab(0, tab, file.name)
            self.tabs.setCurrentIndex(tab_idx)
            self.setUpdatesEnabled(True)
        except Exception as e:
            QMessageBox.critical(
                self,
                "Error importing WKT geometry",
                f"Error importing WKT geometry:\n{e}\n{full_stack()}",
            )
            return

    def _open_replay(self):
        self.setUpdatesEnabled(False)
        tab = ReplayWidget()
        tab_idx = self.tabs.insertTab(0, tab, "replay")
        self.tabs.setCurrentIndex(tab_idx)
        self.setUpdatesEnabled(True)
        self.update()
