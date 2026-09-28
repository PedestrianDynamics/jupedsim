# SPDX-License-Identifier: LGPL-3.0-or-later
"""Create jupedsim recording sqlite files in schema version 1, 2 or 3.

Imported by the test suite (``write_recording``) and run directly by CI::

    python tests/make_fixture_recording.py PATH --version 3 --frames 10 --agents 4

Only the standard library may be used here: the fixtures are created with a
bare interpreter, without the visualizer, jupedsim or shapely installed.
"""

from __future__ import annotations

import argparse
import sqlite3
from pathlib import Path

#: Walkable area written into every fixture, a 10 x 10 box.
DEFAULT_WKT = "GEOMETRYCOLLECTION(POLYGON((0 0, 0 10, 10 10, 10 0, 0 0)))"

#: ``(xmin, xmax, ymin, ymax)`` of :data:`DEFAULT_WKT`. Passed in explicitly
#: because computing it would require shapely.
DEFAULT_BOUNDS = (0.0, 10.0, 0.0, 10.0)

#: Schema versions 2 and 3 link ``frame_data`` to ``geometry`` by hash. The
#: value is arbitrary, a constant keeps the generated files reproducible.
GEOMETRY_HASH = 1

#: Orientation written to the ``ori_x`` / ``ori_y`` columns of schema
#: versions 1 and 2. A reader must ignore these.
ORIENTATION = (0.9, 0.1)

SUPPORTED_VERSIONS = (1, 2, 3)


def agent_id(index: int) -> int:
    """Id of the ``index``-th agent (ids start at one, as in jupedsim)."""
    return index + 1


def agent_position(frame: int, index: int) -> tuple[float, float]:
    """Position of the ``index``-th agent in ``frame``.

    Deterministic and exactly representable as a float so tests can compare
    positions without a tolerance.
    """
    return (frame + index * 0.5, index * 1.5)


def write_recording(
    path: str | Path,
    *,
    version: int,
    frames: int = 3,
    agents: int = 3,
    fps: float = 25.0,
    wkt: str = DEFAULT_WKT,
    bounds: tuple[float, float, float, float] = DEFAULT_BOUNDS,
) -> Path:
    """Write a recording in schema ``version`` to ``path``, replacing it."""
    if version not in SUPPORTED_VERSIONS:
        raise ValueError(f"Unsupported schema version: {version}")
    path = Path(path)
    if path.exists():
        path.unlink()
    con = sqlite3.connect(path)
    try:
        with con:
            _write_metadata(con, version, fps, bounds)
            _write_geometry(con, version, wkt)
            _write_trajectory_data(con, version, frames, agents)
            if version >= 2:
                _write_frame_data(con, frames)
    finally:
        con.close()
    return path


def _write_metadata(
    con: sqlite3.Connection,
    version: int,
    fps: float,
    bounds: tuple[float, float, float, float],
) -> None:
    xmin, xmax, ymin, ymax = bounds
    con.execute("CREATE TABLE metadata(key TEXT NOT NULL, value TEXT NOT NULL)")
    con.executemany(
        "INSERT INTO metadata VALUES(?, ?)",
        (
            ("version", str(version)),
            ("fps", str(fps)),
            ("xmin", str(xmin)),
            ("xmax", str(xmax)),
            ("ymin", str(ymin)),
            ("ymax", str(ymax)),
        ),
    )


def _write_geometry(con: sqlite3.Connection, version: int, wkt: str) -> None:
    if version == 1:
        con.execute("CREATE TABLE geometry(wkt TEXT NOT NULL)")
        con.execute("INSERT INTO geometry VALUES(?)", (wkt,))
    else:
        con.execute(
            "CREATE TABLE geometry("
            "   hash INTEGER NOT NULL, "
            "   wkt TEXT NOT NULL)"
        )
        con.execute("INSERT INTO geometry VALUES(?, ?)", (GEOMETRY_HASH, wkt))


def _write_trajectory_data(
    con: sqlite3.Connection, version: int, frames: int, agents: int
) -> None:
    with_orientation = version in (1, 2)
    columns = (
        "   frame INTEGER NOT NULL,"
        "   id INTEGER NOT NULL,"
        "   pos_x REAL NOT NULL,"
        "   pos_y REAL NOT NULL"
    )
    if with_orientation:
        columns += ",   ori_x REAL NOT NULL,   ori_y REAL NOT NULL"
    con.execute(f"CREATE TABLE trajectory_data ({columns})")

    rows = []
    for frame in range(frames):
        for index in range(agents):
            pos_x, pos_y = agent_position(frame, index)
            row = (frame, agent_id(index), pos_x, pos_y)
            if with_orientation:
                row += ORIENTATION
            rows.append(row)
    placeholders = "?,?,?,?,?,?" if with_orientation else "?,?,?,?"
    con.executemany(f"INSERT INTO trajectory_data VALUES({placeholders})", rows)


def _write_frame_data(con: sqlite3.Connection, frames: int) -> None:
    con.execute(
        "CREATE TABLE frame_data("
        "   frame INTEGER NOT NULL,"
        "   geometry_hash INTEGER NOT NULL)"
    )
    con.executemany(
        "INSERT INTO frame_data VALUES(?, ?)",
        [(frame, GEOMETRY_HASH) for frame in range(frames)],
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Create a jupedsim recording sqlite fixture."
    )
    parser.add_argument("path", type=Path, help="file to write")
    parser.add_argument(
        "--version",
        type=int,
        choices=SUPPORTED_VERSIONS,
        default=3,
        help="schema version to write (default: 3)",
    )
    parser.add_argument(
        "--frames", type=int, default=3, help="number of frames (default: 3)"
    )
    parser.add_argument(
        "--agents",
        type=int,
        default=3,
        help="number of agents per frame (default: 3)",
    )
    args = parser.parse_args()
    write_recording(
        args.path,
        version=args.version,
        frames=args.frames,
        agents=args.agents,
    )
    print(
        f"wrote {args.path} (version {args.version}, "
        f"{args.frames} frames, {args.agents} agents)"
    )


if __name__ == "__main__":
    main()
