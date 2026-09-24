# SPDX-License-Identifier: LGPL-3.0-or-later
"""Entry point: ``jupedsim-visualizer`` (console script) and
``python -m jupedsim_visualizer``."""

import sys


def main() -> int:
    from PySide6.QtWidgets import QApplication

    from jupedsim_visualizer.main_window import MainWindow

    app = QApplication(sys.argv)
    _window = MainWindow()
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
