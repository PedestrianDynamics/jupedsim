import sqlite3
from dataclasses import dataclass

import shapely

from jupedsim.aabb import AABB


@dataclass
class RecordingAgent:
    id: int
    position: tuple[float, float]
    orientation: tuple[float, float]


@dataclass
class RecordingFrame:
    index: int
    agents: list[RecordingAgent]


class Recording:
    __supported_database_version = 1
    """Provides access to a simulation recording in a sqlite database"""

    def __init__(self, db_connection_str: str, uri=False) -> None:
        self.db = sqlite3.connect(
            db_connection_str, uri=uri, isolation_level=None
        )
        self._check_version_compatible()

    def frame(self, index: int) -> RecordingFrame:
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
        cur = self.db.cursor()
        res = cur.execute("SELECT wkt FROM geometry")
        wkt_str = res.fetchone()[0]
        return shapely.from_wkt(wkt_str)

    def bounds(self) -> AABB:
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
        cur = self.db.cursor()
        res = cur.execute("SELECT MAX(frame) FROM trajectory_data")
        return res.fetchone()[0]

    @property
    def fps(self) -> float:
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
