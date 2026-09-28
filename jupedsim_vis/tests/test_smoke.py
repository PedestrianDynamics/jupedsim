# SPDX-License-Identifier: LGPL-3.0-or-later
"""Headless smoke tests. They also run in `.github/workflows/visualizer.yml`,
both with and without jupedsim installed.

Importing the GUI modules while jupedsim is unavailable is covered by
`tests/test_widgets.py::test_gui_modules_import_without_jupedsim`.
"""

import ast
from pathlib import Path

import pytest


def _imported_modules(source: str):
    """Every module name the source imports, `import x.y` and
    `from x.y import z` alike. Relative imports are skipped."""
    for node in ast.walk(ast.parse(source)):
        if isinstance(node, ast.Import):
            for alias in node.names:
                yield alias.name
        elif isinstance(node, ast.ImportFrom) and not node.level:
            if node.module:
                yield node.module


def _is_jupedsim(name: str) -> bool:
    """True for jupedsim and its sub-modules, but not for the packages
    whose name merely starts with it, such as jupedsim_vis."""
    return name == "jupedsim" or name.startswith("jupedsim.")


def _package_modules() -> list[Path]:
    """Every module file of the installed package."""
    import jupedsim_vis

    return sorted(Path(jupedsim_vis.__file__).parent.glob("*.py"))


def _jupedsim_imports(path: Path) -> list[str]:
    """The jupedsim modules imported by `path`, in source order."""
    source = path.read_text(encoding="utf-8")
    return [n for n in _imported_modules(source) if _is_jupedsim(n)]


def test_only_routing_imports_jupedsim():
    """jupedsim is an optional dependency reached through routing.py alone.
    ruff (TID251) enforces this on the source tree, this test enforces it on
    whatever is installed."""
    modules = _package_modules()
    assert [py.name for py in modules].count("routing.py") == 1

    offenders = {
        py.name: imported
        for py in modules
        if py.name != "routing.py" and (imported := _jupedsim_imports(py))
    }
    assert not offenders, f"only routing.py may import jupedsim: {offenders}"


def test_routing_uses_public_jupedsim_api_only():
    """routing.py may import jupedsim, but only its public API."""
    routing = next(py for py in _package_modules() if py.name == "routing.py")
    internal = [
        name
        for name in _jupedsim_imports(routing)
        if name == "jupedsim.internal" or name.startswith("jupedsim.internal.")
    ]
    assert not internal, f"routing.py imports {internal}"


def test_aabb_combine():
    from jupedsim_vis.aabb import AABB

    a = AABB(xmin=0, xmax=1, ymin=0, ymax=1)
    b = AABB(xmin=-1, xmax=0.5, ymin=2, ymax=3)
    c = AABB.combine(a, b)
    assert (c.xmin, c.xmax, c.ymin, c.ymax) == (-1, 1, 0, 3)
    assert c.center == (0, 1.5)


def test_aabb_rejects_inverted_bounds():
    from jupedsim_vis.aabb import AABB

    with pytest.raises(Exception):
        AABB(xmin=1, xmax=0, ymin=0, ymax=1)


def test_entry_point_importable():
    from jupedsim_vis.__main__ import main

    assert callable(main)


def test_main_window_constructs(monkeypatch):
    """Builds menus, tabs and the state machine under the offscreen
    platform; exits the event loop right away."""
    monkeypatch.setenv("QT_QPA_PLATFORM", "offscreen")
    from PySide6.QtCore import QTimer
    from PySide6.QtWidgets import QApplication

    from jupedsim_vis.main_window import MainWindow

    app = QApplication.instance() or QApplication([])
    window = MainWindow()
    QTimer.singleShot(0, app.quit)
    assert app.exec() == 0
    assert window.windowTitle() == "jupedsim_vis"
