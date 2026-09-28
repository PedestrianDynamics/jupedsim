# SPDX-License-Identifier: LGPL-3.0-or-later
import os
import sys
from pathlib import Path

import pytest

# Nothing in the test suite may open a real window: a render widget without
# an OpenGL context crashes the interpreter. Set before the first Qt import.
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

# The tests are collected with --import-mode=importlib, which does not put the
# test directory on sys.path. Do it here so test modules (and this file) can
# import the fixture writer, which doubles as a CI script.
sys.path.insert(0, str(Path(__file__).parent))

from make_fixture_recording import write_recording  # noqa: E402


@pytest.fixture(autouse=True)
def isolated_qsettings(tmp_path, monkeypatch):
    """Keep ``QSettings`` out of the developer's real configuration.

    ``MainWindow`` reads and writes ``QSettings``; without this the test
    suite would pick up (and overwrite) whatever the machine has stored.

    Redirecting the ini format is not enough on macOS, where the two
    argument ``QSettings(organization, application)`` constructor keeps
    using the native plist regardless of ``setDefaultFormat()``; hence the
    explicit format and scope for the one place that builds a ``QSettings``.
    """
    from PySide6.QtCore import QSettings

    import jupedsim_vis.main_window as main_window

    QSettings.setDefaultFormat(QSettings.Format.IniFormat)
    QSettings.setPath(
        QSettings.Format.IniFormat,
        QSettings.Scope.UserScope,
        str(tmp_path),
    )

    def ini_settings(organization: str, application: str = "") -> QSettings:
        return QSettings(
            QSettings.Format.IniFormat,
            QSettings.Scope.UserScope,
            organization,
            application,
        )

    monkeypatch.setattr(main_window, "QSettings", ini_settings)
    yield


@pytest.fixture(autouse=True)
def drain_qt_events():
    """Run pending deletions between tests.

    ``QApplication`` is a process singleton shared by all tests, so windows
    that only got a ``deleteLater()`` (or that went out of scope inside a
    call to ``main()``) must be reaped before the next test runs.
    """
    yield

    from PySide6.QtWidgets import QApplication

    app = QApplication.instance()
    if app is not None:
        app.processEvents()


@pytest.fixture
def qapplication():
    """The process wide ``QApplication``, created on first use."""
    from PySide6.QtWidgets import QApplication

    return QApplication.instance() or QApplication([])


@pytest.fixture(params=[1, 2, 3])
def recording_db(request, tmp_path) -> tuple[Path, int]:
    """A recording fixture, once per supported schema version.

    Returns ``(path, version)``.
    """
    version = request.param
    path = tmp_path / f"recording_v{version}.sqlite"
    write_recording(path, version=version)
    return path, version
