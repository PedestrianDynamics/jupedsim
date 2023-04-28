from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QApplication,
    QHBoxLayout,
    QPushButton,
    QSlider,
    QStyle,
    QVBoxLayout,
    QWidget,
)


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
        self.back = QPushButton(
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
        self.slider = QSlider()
        self.slider.setOrientation(Qt.Orientation.Horizontal)
        self.slider.setMaximum(60)
        self.slider.setPageStep(1)
        self.slider.setTracking(True)
        buttons = QHBoxLayout()
        buttons.addWidget(self.begin)
        buttons.addWidget(self.back)
        buttons.addWidget(self.play)
        buttons.addWidget(self.forward)
        buttons.addWidget(self.end)
        layout = QVBoxLayout()
        layout.addLayout(buttons)
        layout.addWidget(self.slider)
        self.setLayout(layout)


class ReplayWidget(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self._build_content()

    def _build_content(self):
        self.control = PlayerControlWidget()
        layout = QVBoxLayout()
        layout.addStretch(1)
        layout.addWidget(self.control)
        self.setLayout(layout)
