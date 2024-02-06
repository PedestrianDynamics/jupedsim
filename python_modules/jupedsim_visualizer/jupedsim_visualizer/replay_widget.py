# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math

from jupedsim_visualizer.geometry import Geometry
from jupedsim_visualizer.geometry_widget import RenderWidget
from jupedsim_visualizer.trajectory import Trajectory
from PySide6.QtCore import QSignalBlocker, Qt, QTimer
from PySide6.QtGui import QFont, QPaintEvent
from PySide6.QtStateMachine import QState, QStateMachine
from PySide6.QtWidgets import (
    QApplication,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QSlider,
    QSpinBox,
    QStyle,
    QVBoxLayout,
    QWidget,
)

from jupedsim import RoutingEngine
from jupedsim.recording import Recording


class PlayerControlWidget(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.play = QPushButton(
            QApplication.style().standardIcon(
                QStyle.StandardPixmap.SP_MediaPlay
            ),
            "",
        )
        self.play.setCheckable(True)
        self.begin = QPushButton(
            QApplication.style().standardIcon(
                QStyle.StandardPixmap.SP_MediaSkipBackward
            ),
            "",
        )
        self.backward = QPushButton(
            QApplication.style().standardIcon(
                QStyle.StandardPixmap.SP_MediaSeekBackward
            ),
            "",
        )
        self.forward = QPushButton(
            QApplication.style().standardIcon(
                QStyle.StandardPixmap.SP_MediaSeekForward
            ),
            "",
        )
        self.end = QPushButton(
            QApplication.style().standardIcon(
                QStyle.StandardPixmap.SP_MediaSkipForward
            ),
            "",
        )
        self.speed_selector = QSpinBox()
        self.speed_selector.setRange(1, 10)
        self.speed_selector.setValue(1)
        self.speed_selector.setSuffix("x")
        self.slider = QSlider()
        self.slider.setOrientation(Qt.Orientation.Horizontal)
        self.slider.setMaximum(60)
        self.slider.setPageStep(1)
        self.slider.setTracking(True)
        self.replay_time = QLabel("00:00:00.000")
        font = QFont("monospace")
        font.setStyleHint(QFont.StyleHint.Monospace)
        self.replay_time.setFont(font)
        row1 = QHBoxLayout()
        row1.addStretch()
        row1.addWidget(self.begin)
        row1.addWidget(self.backward)
        row1.addWidget(self.play)
        row1.addWidget(self.forward)
        row1.addWidget(self.end)
        row1.addWidget(self.speed_selector)
        row1.addStretch()
        row2 = QHBoxLayout()
        row2.addWidget(self.slider, 1)
        row2.addWidget(self.replay_time)
        layout = QVBoxLayout()
        layout.addLayout(row1)
        layout.addLayout(row2)
        self.setLayout(layout)
        self._build_state_machine()

    def _build_state_machine(self) -> None:
        sm = QStateMachine(self)

        replay_paused = QState()
        sm.addState(replay_paused)

        replay_playing = QState()
        replay_playing.entered.connect(lambda: self.play.setChecked(True))
        replay_playing.exited.connect(lambda: self.play.setChecked(False))

        sm.addState(replay_playing)

        sm.setInitialState(replay_paused)

        replay_paused.addTransition(self.play.clicked, replay_playing)

        replay_playing.addTransition(self.play.clicked, replay_paused)
        replay_playing.addTransition(self.forward.clicked, replay_paused)
        replay_playing.addTransition(self.backward.clicked, replay_paused)
        replay_playing.addTransition(self.begin.clicked, replay_paused)
        replay_playing.addTransition(self.end.clicked, replay_paused)
        replay_playing.addTransition(self.slider.valueChanged, replay_paused)

        sm.start()
        self.state_machine = sm
        self.replay_paused = replay_paused
        self.replay_playing = replay_playing

    def update_replay_time(self, time_in_seconds: float) -> None:
        hh = int(math.floor(time_in_seconds / 3600))
        time_in_seconds = time_in_seconds - hh * 3600
        mm = int(math.floor(time_in_seconds / 60))
        time_in_seconds = time_in_seconds - mm * 60
        ss = int(math.floor(time_in_seconds))
        time_in_seconds = time_in_seconds - ss
        ms = int(time_in_seconds * 1000)
        self.replay_time.setText(f"{hh:02d}:{mm:02d}:{ss:02d}.{ms:03d}")


class ReplayWidget(QWidget):
    def __init__(
        self,
        navi: RoutingEngine,
        rec: Recording,
        geo: Geometry,
        trajectory: Trajectory,
        parent=None,
    ):
        QWidget.__init__(self, parent)
        self.rec = rec
        self.trajectory = trajectory
        self.control = PlayerControlWidget(parent=self)
        self.render_widget = RenderWidget(
            geo, navi, [geo, trajectory], parent=self
        )
        self.geo = geo
        layout = QVBoxLayout()
        layout.addWidget(self.render_widget, 1)
        layout.addWidget(self.control)
        self.setLayout(layout)
        self.control.play.toggled.connect(self.play)
        self.control.forward.clicked.connect(self.frame_forward)
        self.control.backward.clicked.connect(self.frame_backward)
        self.control.slider.setMaximum(self.rec.num_frames - 1)
        self.control.slider.valueChanged.connect(self.goto_frame)
        self.control.begin.clicked.connect(lambda: self.goto_frame(0))
        self.control.end.clicked.connect(
            lambda: self.goto_frame(self.trajectory.num_frames - 1)
        )

    def frame_forward(self):
        self.trajectory.advance_frame(self.control.speed_selector.value())
        self.control.update_replay_time(
            self.trajectory.current_index * (1 / self.rec.fps)
        )
        self.render_widget.render()
        with QSignalBlocker(self.control.slider):
            self.control.slider.setValue(self.trajectory.current_index)

    def frame_backward(self):
        self.trajectory.advance_frame(-self.control.speed_selector.value())
        self.control.update_replay_time(
            self.trajectory.current_index * (1 / self.rec.fps)
        )
        self.render_widget.render()
        with QSignalBlocker(self.control.slider):
            self.control.slider.setValue(self.trajectory.current_index)

    def goto_frame(self, index: int):
        self.trajectory.goto_frame(index)
        self.control.update_replay_time(
            self.trajectory.current_index * (1 / self.rec.fps)
        )
        self.render_widget.render()
        with QSignalBlocker(self.control.slider):
            self.control.slider.setValue(self.trajectory.current_index)

    def play(self, checked: bool):
        if checked:
            self.timer = QTimer()
            self.timer.setInterval(int(1000.0 / self.rec.fps))
            self.timer.timeout.connect(self.frame_forward)
            self.timer.start()
        else:
            if self.timer:
                self.timer.stop()

    def render(self):
        self.render_widget.render()

    def paintEvent(self, event: QPaintEvent) -> None:
        self.render()
        return super().paintEvent(event)
