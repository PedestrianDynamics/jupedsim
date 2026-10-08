# SPDX-License-Identifier: LGPL-3.0-or-later
"""Strictly read-only access to jupedsim sqlite recordings.

Understands the schema versions 1, 2 and 3 directly instead of migrating the
file to the latest version like ``jupedsim.recording`` does: the visualizer is
a viewer and must never modify a recording it is asked to display.

The differences between the versions are small:

* v1: single-row ``geometry(wkt)``, no ``frame_data`` table, ``trajectory_data``
  carries the (now unused) ``ori_x`` / ``ori_y`` columns.
* v2: ``geometry(hash, wkt)`` plus a ``frame_data(frame, geometry_hash)``
  table; ``trajectory_data`` still carries ``ori_x`` / ``ori_y``.
* v3: like v2, without the orientation columns.

Selecting an explicit column list makes the orientation columns a non-issue,
so only the frame count has to be handled per version.
"""

from __future__ import annotations

import math
import sqlite3
from dataclasses import dataclass
from pathlib import Path
from types import TracebackType

import shapely

from jupedsim_vis.aabb import AABB

#: Schema versions this reader understands.
SUPPORTED_VERSIONS = (1, 2, 3)


class RecordingError(Exception):
    """Raised for anything that keeps a recording from being read."""


@dataclass
class RecordingAgent:
    """Data for a single agent at a single frame."""

    id: int
    position: tuple[float, float]


@dataclass
class RecordingFrame:
    """A single frame from the simulation."""

    index: int
    agents: list[RecordingAgent]


class Recording:
    """A simulation recording, opened read-only.

    The file is opened with sqlite's ``mode=ro`` URI flag, so neither this
    class nor sqlite itself can write to it or create journal side-car files.
    """

    def __init__(self, path: str | Path) -> None:
        self._path = Path(path)
        self._db: sqlite3.Connection | None = None
        if not self._path.is_file():
            raise RecordingError(f"No such recording file: {self._path}")
        try:
            uri = f"{self._path.resolve().as_uri()}?mode=ro"
            db = sqlite3.connect(uri, uri=True, isolation_level=None)
        except sqlite3.Error as e:
            raise RecordingError(
                f"Cannot open recording {self._path}: {e}"
            ) from e
        self._db = db
        try:
            # sqlite opens lazily, force it so a file that is not a database
            # is reported here and not on the first read.
            self._query_one("SELECT 1")
            self._version = self._read_version()
        except BaseException:
            self.close()
            raise

    @property
    def path(self) -> Path:
        """Path of the recording file."""
        return self._path

    @property
    def version(self) -> int:
        """Schema version of the recording file."""
        return self._version

    def frame(self, index: int) -> RecordingFrame:
        """Access a single frame of the recording.

        Args:
            index: index of the frame to access.

        Returns:
            The frame; its agent list is empty if no such frame exists.

        """
        rows = self._query_all(
            "SELECT id, pos_x, pos_y FROM trajectory_data "
            "WHERE frame == ? ORDER BY id ASC",
            (index,),
        )
        return RecordingFrame(
            index, [RecordingAgent(r[0], (r[1], r[2])) for r in rows]
        )

    def geometry(self) -> shapely.Geometry:
        """Access this recording's geometry.

        Returns:
            Walkable area of the simulation that created this recording.

        """
        rows = self._query_all("SELECT wkt FROM geometry")
        try:
            geometries = [shapely.from_wkt(r[0]) for r in rows]
        except shapely.errors.GEOSException as e:
            raise RecordingError(
                f"Recording {self._path} contains invalid geometry: {e}"
            ) from e
        return shapely.union_all(geometries)

    def bounds(self) -> AABB:
        """Bounding box of all positions and geometry in this recording.

        Taken from the metadata; recordings written without bounds metadata
        fall back to the bounds of the geometry. Raises `RecordingError` if
        neither source yields a finite box.
        """
        values = [
            self._metadata(key) for key in ("xmin", "xmax", "ymin", "ymax")
        ]
        if all(v is not None for v in values):
            try:
                xmin, xmax, ymin, ymax = (float(v) for v in values)
            except (TypeError, ValueError) as e:
                raise RecordingError(
                    f"Recording {self._path} has invalid bounds metadata: {e}"
                ) from e
        else:
            geometry = self.geometry()
            if geometry.is_empty:
                raise RecordingError(
                    f"Recording {self._path} has no bounds metadata and no "
                    "geometry to derive bounds from. It is likely empty."
                )
            xmin, ymin, xmax, ymax = geometry.bounds
        # An empty geometry yields (nan, nan, nan, nan) rather than an empty
        # tuple, and a recording without any written frame keeps the writer's
        # infinite defaults in its metadata. Neither makes an AABB.
        if not all(math.isfinite(v) for v in (xmin, xmax, ymin, ymax)):
            raise RecordingError(
                f"Recording {self._path} has no usable position bounds "
                f"(xmin={xmin}, xmax={xmax}, ymin={ymin}, ymax={ymax}). "
                "It is likely empty."
            )
        return AABB(xmin=xmin, xmax=xmax, ymin=ymin, ymax=ymax)

    @property
    def num_frames(self) -> int:
        """Number of frames stored in this recording."""
        if self._version == 1:
            # v1 has no frame_data table.
            row = self._query_one("SELECT max(frame) FROM trajectory_data")
            if row is None or row[0] is None:
                return 0
            return int(row[0]) + 1
        row = self._query_one("SELECT count(*) FROM frame_data")
        return int(row[0])

    @property
    def fps(self) -> float:
        """How many frames are stored per second."""
        value = self._metadata("fps")
        if value is None:
            raise RecordingError(f"Recording {self._path} has no fps metadata.")
        try:
            return float(value)
        except (TypeError, ValueError) as e:
            raise RecordingError(
                f"Recording {self._path}: metadata fps is not a number. "
                f"Value found: {value!r}"
            ) from e

    def close(self) -> None:
        """Close the database connection. Safe to call more than once."""
        if self._db is not None:
            try:
                self._db.close()
            finally:
                self._db = None

    def __enter__(self) -> "Recording":
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc: BaseException | None,
        tb: TracebackType | None,
    ) -> None:
        self.close()

    def _read_version(self) -> int:
        row = self._query_one(
            "SELECT value FROM metadata WHERE key == 'version'"
        )
        if row is None:
            raise RecordingError(
                f"Recording {self._path} has no version metadata. "
                "It is not a jupedsim recording."
            )
        value = row[0]
        try:
            version = int(value)
        except (TypeError, ValueError) as e:
            raise RecordingError(
                f"Recording {self._path}: metadata version is not an "
                f"integer. Value found: {value!r}"
            ) from e
        if version not in SUPPORTED_VERSIONS:
            supported = ", ".join(str(v) for v in SUPPORTED_VERSIONS)
            raise RecordingError(
                f"Unsupported recording version {version}. This viewer "
                f"reads versions {supported}. ({self._path})"
            )
        return version

    def _metadata(self, key: str) -> str | None:
        row = self._query_one(
            "SELECT value FROM metadata WHERE key == ?", (key,)
        )
        return None if row is None else row[0]

    def _query_one(self, sql: str, parameters: tuple = ()):
        return self._execute(sql, parameters).fetchone()

    def _query_all(self, sql: str, parameters: tuple = ()) -> list:
        return self._execute(sql, parameters).fetchall()

    def _execute(self, sql: str, parameters: tuple) -> sqlite3.Cursor:
        if self._db is None:
            raise RecordingError(f"Recording {self._path} is closed.")
        try:
            return self._db.execute(sql, parameters)
        except sqlite3.Error as e:
            raise RecordingError(
                f"Cannot read recording {self._path}: {e}"
            ) from e
