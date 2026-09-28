# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for ``MainWindow``: status bar and the load/view split.

``MainWindow`` itself is safe to build under the offscreen platform as long
as no tab is created: a tab owns a ``RenderWidget``, which segfaults without
a real OpenGL context. Therefore these tests only ever call the pure loading
half of the split (``load_wkt`` / ``load_recording`` /
``open_*_file(with_view=False)``) and never ``add_geometry_tab`` /
``add_recording_tab``.
"""

import os
import sys
from pathlib import Path

import pytest

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from make_fixture_recording import write_recording  # noqa: E402
from PySide6.QtWidgets import QApplication  # noqa: E402
from shapely.errors import GEOSException  # noqa: E402
from test_routing import (  # noqa: E402
    WorkingEngine,
    install_fake_jupedsim,
)

from jupedsim_vis.main_window import (  # noqa: E402
    GeometryDocument,
    MainWindow,
    RecordingDocument,
)
from jupedsim_vis.recording import RecordingError  # noqa: E402

DATA = Path(__file__).parent / "data"
SIMPLE_WKT = DATA / "simple.wkt"

NO_JUPEDSIM = "Cannot calculate shortest path: jupedsim is not installed."


@pytest.fixture
def windows():
    """Hands out ``MainWindow`` instances and closes them afterwards."""
    QApplication.instance() or QApplication([])
    created: list[MainWindow] = []

    def make() -> MainWindow:
        window = MainWindow()
        created.append(window)
        return window

    yield make

    for window in created:
        window.close()
        window.deleteLater()


@pytest.fixture
def no_jupedsim(monkeypatch):
    """``None`` in ``sys.modules`` makes ``import jupedsim`` raise."""
    monkeypatch.setitem(sys.modules, "jupedsim", None)


@pytest.fixture
def fake_jupedsim(monkeypatch):
    """A jupedsim whose ``RoutingEngine`` routes anything."""
    install_fake_jupedsim(monkeypatch, WorkingEngine)


def test_status_bar_reports_missing_jupedsim(no_jupedsim, windows) -> None:
    window = windows()

    assert window.statusBar().currentMessage() == NO_JUPEDSIM


def test_status_bar_is_empty_when_routing_works(fake_jupedsim, windows) -> None:
    window = windows()

    assert window.statusBar().currentMessage() == ""

    window.load_wkt(SIMPLE_WKT)

    assert window.statusBar().currentMessage() == ""
    assert window.tabs.count() == 0


def test_load_wkt_builds_a_document_without_a_tab(no_jupedsim, windows) -> None:
    window = windows()

    doc = window.load_wkt(SIMPLE_WKT)

    assert isinstance(doc, GeometryDocument)
    assert doc.path == SIMPLE_WKT
    assert doc.geo.num_polygons == 1
    assert doc.geo.num_triangles > 0
    assert doc.name_text == f"Geometry: {SIMPLE_WKT}"
    assert "Triangles:" in doc.info_text
    assert "Polygons: 1" in doc.info_text
    assert window.documents == [doc]
    assert window.tabs.count() == 0


def test_load_wkt_info_text_reports_dimensions(no_jupedsim, windows) -> None:
    window = windows()

    doc = window.load_wkt(SIMPLE_WKT)

    assert doc.info_text == (
        f"Dimensions: 20m x 20m  Polygons: {doc.geo.num_polygons}  "
        f"Triangles: {doc.geo.num_triangles}"
    )


def test_open_wkt_file_without_view_only_loads(no_jupedsim, windows) -> None:
    window = windows()

    doc = window.open_wkt_file(SIMPLE_WKT, with_view=False)

    assert isinstance(doc, GeometryDocument)
    assert window.documents == [doc]
    assert window.tabs.count() == 0


def test_load_recording_builds_a_document(no_jupedsim, windows, tmp_path):
    path = tmp_path / "recording_v3.sqlite"
    write_recording(path, version=3, frames=5, agents=4)
    window = windows()

    doc = window.load_recording(path)

    assert isinstance(doc, RecordingDocument)
    assert doc.path == path
    assert doc.recording.num_frames > 0
    assert doc.trajectory.num_frames == doc.recording.num_frames
    assert doc.geo.num_triangles > 0
    assert window.documents == [doc]
    assert window.tabs.count() == 0


def test_open_recording_file_without_view_only_loads(
    no_jupedsim, windows, tmp_path
):
    path = tmp_path / "recording_v3.sqlite"
    write_recording(path, version=3)
    window = windows()

    doc = window.open_recording_file(path, with_view=False)

    assert isinstance(doc, RecordingDocument)
    assert window.documents == [doc]
    assert window.tabs.count() == 0


def test_load_wkt_raises_on_garbage(no_jupedsim, windows, tmp_path) -> None:
    path = tmp_path / "broken.wkt"
    path.write_text("this is not wkt at all", encoding="UTF-8")
    window = windows()

    # shapely rejects the text before the triangulation ever sees it.
    with pytest.raises(GEOSException):
        window.load_wkt(path)

    assert window.documents == []


def test_load_wkt_raises_on_missing_file(no_jupedsim, windows, tmp_path):
    window = windows()

    with pytest.raises(OSError):
        window.load_wkt(tmp_path / "does_not_exist.wkt")

    assert window.documents == []


def test_load_recording_raises_on_missing_file(no_jupedsim, windows, tmp_path):
    window = windows()

    with pytest.raises(RecordingError):
        window.load_recording(tmp_path / "does_not_exist.sqlite")

    assert window.documents == []


def test_documents_accumulate(no_jupedsim, windows, tmp_path) -> None:
    path = tmp_path / "recording_v3.sqlite"
    write_recording(path, version=3)
    window = windows()

    geometry_doc = window.load_wkt(SIMPLE_WKT)
    recording_doc = window.load_recording(path)

    assert window.documents == [geometry_doc, recording_doc]
