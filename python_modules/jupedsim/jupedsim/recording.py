# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import sqlite3
from dataclasses import dataclass

import shapely

from jupedsim.internal.aabb import AABB
from jupedsim.sqlite_serialization import update_database_to_latest_version


@dataclass
class RecordingAgent:
    """Data for a single agent at a single frame."""

    id: int
    position: tuple[float, float]
    orientation: tuple[float, float]


@dataclass
class RecordingFrame:
    """A single frame from the simulation."""

    index: int
    agents: list[RecordingAgent]


class Recording:
    __supported_database_version = 2
    """Provides access to a simulation recording in a sqlite database"""

    def __init__(self, db_connection_str: str, uri=False) -> None:
        self.db = sqlite3.connect(
            db_connection_str, uri=uri, isolation_level=None
        )
        update_database_to_latest_version(self.db)
        self._check_version_compatible()

    def frame(self, index: int) -> RecordingFrame:
        """Access a single frame of the recording.

        Arguments:
            index (int): index of the frame to access.

        Returns:
            A single frame.

        """

        def agent_row(cursor, row):
            return RecordingAgent(row[0], (row[1], row[2]), (row[3], row[4]))

        cur = self.db.cursor()
        cur.row_factory = agent_row
        res = cur.execute(
            "SELECT id, pos_x, pos_y, ori_x, ori_y FROM trajectory_data WHERE frame == (?) ORDER BY id ASC",
            (index,),
        )
        return RecordingFrame(index, res.fetchall())

    def geometry(self) -> shapely.GeometryCollection:
        """Access this recordings' geometry.

        Returns:
            walkable area of the simulation that created this recording.

        """
        cur = self.db.cursor()
        res = cur.execute("SELECT wkt FROM geometry")
        geometries = [shapely.from_wkt(s) for s in res.fetchall()]
        return shapely.union_all(geometries)

    def geometry_id_for_frame(self, frame_id) -> int:
        cur = self.db.cursor()
        res = cur.execute(
            "SELECT geometry_hash from frame_data WHERE frame == ?",
            (frame_id,),
        )
        return res.fetchone()[0]

    def bounds(self) -> AABB:
        """Get bounds of the position data contained in this recording."""
        cur = self.db.cursor()
        res = cur.execute("SELECT value FROM metadata WHERE key == 'xmin'")
        xmin = float(res.fetchone()[0])
        res = cur.execute("SELECT value FROM metadata WHERE key == 'xmax'")
        xmax = float(res.fetchone()[0])
        res = cur.execute("SELECT value FROM metadata WHERE key == 'ymin'")
        ymin = float(res.fetchone()[0])
        res = cur.execute("SELECT value FROM metadata WHERE key == 'ymax'")
        ymax = float(res.fetchone()[0])
        return AABB(xmin=xmin, xmax=xmax, ymin=ymin, ymax=ymax)

    @property
    def num_frames(self) -> int:
        """Access the number of frames stored in this recording.

        Returns:
            Number of frames in this recording.

        """
        cur = self.db.cursor()
        res = cur.execute("SELECT count(*) FROM frame_data")
        return res.fetchone()[0]

    @property
    def fps(self) -> float:
        """How many frames are stored per second.

        Returns:
            Frames per second of this recording.

        """
        cur = self.db.cursor()
        res = cur.execute("SELECT value from metadata WHERE key == 'fps'")
        return float(res.fetchone()[0])

    def _check_version_compatible(self) -> None:
        cur = self.db.cursor()
        res = cur.execute("SELECT value FROM metadata WHERE key == 'version'")
        version_string = res.fetchone()[0]
        try:
            version_in_database = int(version_string)
            if version_in_database != self.__supported_database_version:
                raise Exception(
                    f"Incompatible database version. The database supplied is version {version_in_database}. "
                    f"This Program supports version {self.__supported_database_version}"
                )
        except ValueError:
            raise Exception(
                f"Database error, metadata version not an integer. Value found: {version_string}"
            )
