# SPDX-License-Identifier: LGPL-3.0-or-later
"""Headless smoke tests. They run in the in-tree test suite (against the
jupedsim built from the same commit) and in the release workflow (against
the jupedsim from PyPI that satisfies the declared dependency floor)."""

import ast
from pathlib import Path

import pytest


def _imported_names(source: str):
    for node in ast.walk(ast.parse(source)):
        if isinstance(node, ast.Import):
            for alias in node.names:
                yield alias.name
        elif isinstance(node, ast.ImportFrom) and node.module:
            yield node.module


def test_uses_public_jupedsim_api_only():
    """The package is released separately from jupedsim and must only use
    the public API. ruff (TID251) enforces this on the source tree; this
    test enforces it on whatever is installed."""
    import jupedsim_visualizer

    pkg_dir = Path(jupedsim_visualizer.__file__).parent
    for py in pkg_dir.glob("*.py"):
        for name in _imported_names(py.read_text(encoding="utf-8")):
            assert not name.startswith("jupedsim.internal"), (
                f"{py.name} imports {name}"
            )


def test_imports_headless():
    """Importing the GUI modules must not require a display."""
    import jupedsim_visualizer.main_window  # noqa: F401
    import jupedsim_visualizer.replay_widget  # noqa: F401


def test_aabb_combine():
    from jupedsim_visualizer.aabb import AABB

    a = AABB(xmin=0, xmax=1, ymin=0, ymax=1)
    b = AABB(xmin=-1, xmax=0.5, ymin=2, ymax=3)
    c = AABB.combine(a, b)
    assert (c.xmin, c.xmax, c.ymin, c.ymax) == (-1, 1, 0, 3)
    assert c.center == (0, 1.5)


def test_aabb_rejects_inverted_bounds():
    from jupedsim_visualizer.aabb import AABB

    with pytest.raises(Exception):
        AABB(xmin=1, xmax=0, ymin=0, ymax=1)


def test_entry_point_importable():
    from jupedsim_visualizer.__main__ import main

    assert callable(main)


def test_main_window_constructs(monkeypatch):
    """Builds menus, tabs and the state machine under the offscreen
    platform; exits the event loop right away."""
    monkeypatch.setenv("QT_QPA_PLATFORM", "offscreen")
    from PySide6.QtCore import QTimer
    from PySide6.QtWidgets import QApplication

    from jupedsim_visualizer.main_window import MainWindow

    app = QApplication.instance() or QApplication([])
    window = MainWindow()
    QTimer.singleShot(0, app.quit)
    assert app.exec() == 0
    assert window.windowTitle() == "jupedsim_visualizer"
