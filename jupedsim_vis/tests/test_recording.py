# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the read-only sqlite recording reader.

The reader understands the recording schema versions 1, 2 and 3 without ever
migrating or otherwise writing to the file.
"""

import dataclasses
import hashlib
import sqlite3
import subprocess
import sys

import pytest
from make_fixture_recording import (
    DEFAULT_BOUNDS,
    ORIENTATION,
    agent_id,
    agent_position,
    write_recording,
)

from jupedsim_vis.aabb import AABB
from jupedsim_vis.recording import (
    Recording,
    RecordingAgent,
    RecordingError,
    RecordingFrame,
)

FRAMES = 3
AGENTS = 3


def _sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _set_metadata(path, key, value):
    """Change a metadata value with a second, writable connection."""
    con = sqlite3.connect(path)
    with con:
        con.execute("UPDATE metadata SET value = ? WHERE key = ?", (value, key))
    con.close()


def test_reads_frames_of_every_supported_version(recording_db):
    path, _ = recording_db
    with Recording(path) as rec:
        assert rec.num_frames == FRAMES
        assert rec.fps == 25.0
        for index in range(FRAMES):
            frame = rec.frame(index)
            assert isinstance(frame, RecordingFrame)
            assert frame.index == index
            assert [a.id for a in frame.agents] == [
                agent_id(i) for i in range(AGENTS)
            ]
            assert [a.position for a in frame.agents] == [
                agent_position(index, i) for i in range(AGENTS)
            ]


def test_frame_positions_have_the_expected_values(recording_db):
    path, _ = recording_db
    with Recording(path) as rec:
        frame = rec.frame(1)
    assert [(a.id, a.position) for a in frame.agents] == [
        (1, (1.0, 0.0)),
        (2, (1.5, 1.5)),
        (3, (2.0, 3.0)),
    ]


def test_frame_beyond_the_end_is_empty(recording_db):
    path, _ = recording_db
    with Recording(path) as rec:
        assert rec.frame(FRAMES).agents == []


def test_geometry_of_every_supported_version(recording_db):
    path, _ = recording_db
    with Recording(path) as rec:
        geometry = rec.geometry()
    assert not geometry.is_empty
    assert geometry.area == pytest.approx(100.0)


def test_invalid_geometry_wkt_is_rejected(tmp_path):
    path = tmp_path / "bad_geometry.sqlite"
    write_recording(path, version=3, wkt="THIS IS NOT WKT")
    with Recording(path) as rec:
        with pytest.raises(RecordingError) as e:
            rec.geometry()
    assert str(path) in str(e.value)


def test_bounds_of_every_supported_version(recording_db):
    path, _ = recording_db
    with Recording(path) as rec:
        bounds = rec.bounds()
    assert isinstance(bounds, AABB)
    xmin, xmax, ymin, ymax = DEFAULT_BOUNDS
    assert (bounds.xmin, bounds.xmax, bounds.ymin, bounds.ymax) == (
        xmin,
        xmax,
        ymin,
        ymax,
    )


def test_bounds_fall_back_to_the_geometry(tmp_path):
    """A geometry that is wider than it is tall: a box of 0 0 -> 30 10, so
    swapped x and y bounds cannot pass unnoticed."""
    path = tmp_path / "no_bounds.sqlite"
    write_recording(
        path,
        version=3,
        wkt="GEOMETRYCOLLECTION(POLYGON((0 0, 0 10, 30 10, 30 0, 0 0)))",
        bounds=(-1.0, 11.0, -2.0, 12.0),
    )
    con = sqlite3.connect(path)
    with con:
        con.execute(
            "DELETE FROM metadata WHERE key IN ('xmin','xmax','ymin','ymax')"
        )
    con.close()
    with Recording(path) as rec:
        bounds = rec.bounds()
    assert (bounds.xmin, bounds.xmax, bounds.ymin, bounds.ymax) == (
        0.0,
        30.0,
        0.0,
        10.0,
    )


def test_bounds_of_an_empty_recording_are_rejected(tmp_path):
    """No bounds metadata and no geometry: shapely reports
    (nan, nan, nan, nan), which must not become an AABB."""
    path = tmp_path / "empty.sqlite"
    write_recording(path, version=3, frames=0)
    con = sqlite3.connect(path)
    with con:
        con.execute(
            "DELETE FROM metadata WHERE key IN ('xmin','xmax','ymin','ymax')"
        )
        con.execute("DELETE FROM geometry")
    con.close()
    with Recording(path) as rec:
        assert rec.geometry().is_empty
        with pytest.raises(RecordingError) as e:
            rec.bounds()
    assert str(path) in str(e.value)


def test_infinite_bounds_metadata_is_rejected(tmp_path):
    """A recording to which no frame was ever written keeps the writer's
    infinite defaults in its metadata."""
    path = tmp_path / "infinite_bounds.sqlite"
    write_recording(path, version=3, frames=0)
    for key, value in (
        ("xmin", "inf"),
        ("xmax", "-inf"),
        ("ymin", "inf"),
        ("ymax", "-inf"),
    ):
        _set_metadata(path, key, value)
    with Recording(path) as rec:
        with pytest.raises(RecordingError) as e:
            rec.bounds()
    assert str(path) in str(e.value)


def test_reading_does_not_modify_the_file(recording_db):
    path, _ = recording_db
    before_hash = _sha256(path)
    before_mtime = path.stat().st_mtime_ns
    siblings_before = sorted(p.name for p in path.parent.iterdir())

    rec = Recording(path)
    rec.num_frames
    rec.fps
    rec.frame(0)
    rec.geometry()
    rec.bounds()
    rec.close()

    assert _sha256(path) == before_hash
    assert path.stat().st_mtime_ns == before_mtime
    assert sorted(p.name for p in path.parent.iterdir()) == siblings_before
    assert not (path.parent / f"{path.name}-wal").exists()
    assert not (path.parent / f"{path.name}-journal").exists()


def test_connection_is_opened_read_only(recording_db):
    path, _ = recording_db
    before_hash = _sha256(path)
    # Reaches into the connection on purpose: the guarantee under test is
    # that *this* connection, the one all reads go through, cannot write.
    with Recording(path) as rec:
        with pytest.raises(sqlite3.OperationalError):
            rec._db.execute("CREATE TABLE scribble(x INTEGER)")
        with pytest.raises(sqlite3.OperationalError):
            rec._db.execute("INSERT INTO metadata VALUES('scribble', '1')")
        with pytest.raises(sqlite3.OperationalError):
            rec._db.execute("DROP TABLE trajectory_data")
    assert _sha256(path) == before_hash


def test_unsupported_version_is_rejected(tmp_path):
    path = tmp_path / "v4.sqlite"
    write_recording(path, version=3)
    _set_metadata(path, "version", "4")
    with pytest.raises(RecordingError) as e:
        Recording(path)
    assert "Unsupported recording version 4" in str(e.value)
    assert "1, 2, 3" in str(e.value)


def test_non_integer_version_is_rejected(tmp_path):
    path = tmp_path / "abc.sqlite"
    write_recording(path, version=3)
    _set_metadata(path, "version", "abc")
    with pytest.raises(RecordingError) as e:
        Recording(path)
    assert "not an integer" in str(e.value)
    assert "abc" in str(e.value)


def test_missing_version_metadata_is_rejected(tmp_path):
    path = tmp_path / "no_version.sqlite"
    write_recording(path, version=3)
    con = sqlite3.connect(path)
    with con:
        con.execute("DELETE FROM metadata WHERE key = 'version'")
    con.close()
    with pytest.raises(RecordingError) as e:
        Recording(path)
    assert str(path) in str(e.value)


def test_missing_file_is_rejected(tmp_path):
    path = tmp_path / "does_not_exist.sqlite"
    with pytest.raises(RecordingError) as e:
        Recording(path)
    assert str(path) in str(e.value)


def test_non_sqlite_file_is_rejected(tmp_path):
    path = tmp_path / "not_a_db.sqlite"
    path.write_text("this is a text file, not a recording\n" * 64)
    with pytest.raises(RecordingError) as e:
        Recording(path)
    assert str(path) in str(e.value)


def test_database_without_recording_tables_is_rejected(tmp_path):
    path = tmp_path / "metadata_only.sqlite"
    con = sqlite3.connect(path)
    with con:
        con.execute(
            "CREATE TABLE metadata(key TEXT NOT NULL, value TEXT NOT NULL)"
        )
        con.executemany(
            "INSERT INTO metadata VALUES(?, ?)",
            (("version", "3"), ("fps", "25.0")),
        )
    con.close()
    with Recording(path) as rec:
        with pytest.raises(RecordingError) as e:
            rec.frame(0)
        assert str(path) in str(e.value)
        with pytest.raises(RecordingError):
            rec.num_frames
        with pytest.raises(RecordingError):
            rec.geometry()


def test_missing_fps_metadata_is_rejected(tmp_path):
    path = tmp_path / "no_fps.sqlite"
    write_recording(path, version=3)
    con = sqlite3.connect(path)
    with con:
        con.execute("DELETE FROM metadata WHERE key = 'fps'")
    con.close()
    with Recording(path) as rec:
        with pytest.raises(RecordingError):
            rec.fps


@pytest.mark.parametrize("version", [1, 2, 3])
@pytest.mark.parametrize("frames", [0, 1, 5])
def test_num_frames_counts_written_frames(tmp_path, version, frames):
    path = tmp_path / f"v{version}_{frames}.sqlite"
    write_recording(path, version=version, frames=frames)
    with Recording(path) as rec:
        assert rec.num_frames == frames


def test_v1_orientation_columns_are_ignored(tmp_path):
    path = tmp_path / "v1_ori.sqlite"
    write_recording(path, version=1)
    with Recording(path) as rec:
        agents = rec.frame(0).agents
    assert [f.name for f in dataclasses.fields(RecordingAgent)] == [
        "id",
        "position",
    ]
    assert [a.position for a in agents] == [
        agent_position(0, i) for i in range(AGENTS)
    ]
    assert all(a.position != ORIENTATION for a in agents)


def test_close_is_idempotent_and_context_manager_closes(recording_db):
    path, _ = recording_db
    rec = Recording(path)
    rec.close()
    rec.close()
    with pytest.raises(RecordingError):
        rec.frame(0)

    with Recording(path) as ctx:
        assert ctx.num_frames == FRAMES
    with pytest.raises(RecordingError):
        ctx.frame(0)


def test_does_not_import_gui_libraries():
    """Importing the reader must not drag in Qt or VTK. Checked in a fresh
    interpreter because other tests may already have imported them."""
    script = (
        "import sys\n"
        "import jupedsim_vis.recording\n"
        "gui = sorted(\n"
        "    m for m in sys.modules\n"
        "    if m.split('.')[0] in ('PySide6', 'vtk', 'vtkmodules')\n"
        ")\n"
        "assert not gui, gui\n"
    )
    subprocess.run(
        [sys.executable, "-c", script], check=True, capture_output=True
    )
