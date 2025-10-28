# SPDX-License-Identifier: LGPL-3.0-or-later

import itertools
import sqlite3
from pathlib import Path
from typing import Final

from shapely import from_wkt

from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation

DATABASE_VERSION: Final = 2


def get_database_version(connection: sqlite3.Connection) -> int:
    cur = connection.cursor()
    return int(
        cur.execute(
            "SELECT value FROM metadata WHERE key = ?", ("version",)
        ).fetchone()[0]
    )


def uses_latest_database_version(connection: sqlite3.Connection) -> bool:
    version = get_database_version(connection)
    return version == DATABASE_VERSION


class SqliteTrajectoryWriter(TrajectoryWriter):
    """Write trajectory data into a sqlite db"""

    def __init__(
        self,
        *,
        output_file: Path,
        every_nth_frame: int = 4,
        buffer_in_memory: bool = False,
        max_buffered_frames: int = 1000,
    ) -> None:
        """SqliteTrajectoryWriter constructor

        Args:
            output_file : pathlib.Path
                name of the output file.
                Note: the file will not be written until the first call to :func:`begin_writing`
            every_nth_frame: int
                indicates interval between writes, 1 means every frame, 5 every 5th
            buffer_in_memory: bool
                if True, iteration data is kept in RAM and written only when the
                buffer reaches max_buffered_frames or when flush() / close() is called.
            max_buffered_frames: int
                maximum number of frames to keep in RAM before auto-flush.
        """
        self._output_file = output_file
        if every_nth_frame < 1:
            raise TrajectoryWriter.Exception("'every_nth_frame' has to be > 0")
        self._every_nth_frame = every_nth_frame
        # sqlite connection (with disabled autocommit mode); we still use explicit BEGIN/COMMIT
        self._con = sqlite3.connect(self._output_file)#, isolation_level=None)
        # Don't wait for the OS to persist data
        self._con.execute("PRAGMA synchronous=OFF;")
        # Don't allow rollbacks (we don't have need for it)
        self._con.execute("PRAGMA journal_mode=OFF;")
        #self._cur = self._con.cursor()

        # Buffering options and in-memory buffers
        self._buffer_in_memory = bool(buffer_in_memory)
        self._max_buffered_frames = int(max_buffered_frames) if max_buffered_frames > 0 else 1
        self._buffered_frame_count = 0

    def begin_writing(self, simulation: Simulation) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as framerate etc...
        """
        fps = 1 / simulation.delta_time() / self._every_nth_frame
        geo = simulation.get_geometry().as_wkt()

        cur = self._con.cursor()
        try:
            cur.execute("BEGIN")
            cur.execute("DROP TABLE IF EXISTS trajectory_data")
            cur.execute(
                "CREATE TABLE trajectory_data ("
                "   frame INTEGER NOT NULL,"
                "   id INTEGER NOT NULL,"
                "   pos_x REAL NOT NULL,"
                "   pos_y REAL NOT NULL,"
                "   ori_x REAL NOT NULL,"
                "   ori_y REAL NOT NULL)"
            )
            cur.execute("DROP TABLE IF EXISTS metadata")
            cur.execute(
                "CREATE TABLE metadata(key TEXT NOT NULL UNIQUE PRIMARY KEY, value TEXT NOT NULL)"
            )
            cur.executemany(
                "INSERT INTO metadata VALUES(?, ?)",
                (("version", DATABASE_VERSION), ("fps", fps)),
            )
            cur.execute("DROP TABLE IF EXISTS geometry")
            cur.execute(
                "CREATE TABLE geometry("
                "   hash INTEGER NOT NULL, "
                "   wkt TEXT NOT NULL)"
            )
            cur.execute("CREATE UNIQUE INDEX geometry_hash on geometry( hash)")
            cur.execute(
                "INSERT INTO geometry VALUES(?, ?)",
                (hash(geo), geo),
            )
            cur.execute("DROP TABLE IF EXISTS frame_data")
            cur.execute(
                "CREATE TABLE frame_data("
                "   frame INTEGER NOT NULL,"
                "   geometry_hash INTEGER NOT NULL)"
            )

            cur.execute(
                "CREATE INDEX frame_id_idx ON trajectory_data(frame, id)"
            )
            cur.execute("COMMIT")
        except sqlite3.Error as e:
            cur.execute("ROLLBACK")
            raise TrajectoryWriter.Exception(f"Error creating database: {e}")
        finally:
            if self._buffer_in_memory:
                # start a transaction for subsequent writes
                cur.execute("BEGIN")

    def write_iteration_state(self, simulation: Simulation) -> None:
        """Write trajectory data of one simulation iteration.

        This method is intended to handle serialization of the trajectory data
        of a single iteration. If buffering is enabled, data is only written when
        flush() is called (either manually or automatically when the buffer is full)
        """
        if not self._con:
            raise TrajectoryWriter.Exception("Database not opened.")

        iteration = simulation.iteration_count()
        if iteration % self.every_nth_frame() != 0:
            return
        frame = iteration / self.every_nth_frame()
        cur = self._con.cursor()
        try:
            if not self._buffer_in_memory:
                cur.execute("BEGIN")
            frame_data = [
                (
                    frame,
                    agent.id,
                    agent.position[0],
                    agent.position[1],
                    agent.orientation[0],
                    agent.orientation[1],
                )
                for agent in simulation.agents()
            ]
            cur.executemany(
                "INSERT INTO trajectory_data VALUES(?, ?, ?, ?, ?, ?)",
                frame_data,
            )

            geo_wkt = simulation.get_geometry().as_wkt()
            geo_hash = hash(geo_wkt)
            cur.execute(
                "INSERT OR IGNORE INTO geometry(hash, wkt) VALUES(?,?)",
                (geo_hash, geo_wkt),
            )
            cur.execute(
                "INSERT INTO frame_data VALUES(?, ?)",
                (frame, geo_hash),
            )

            xmin, ymin, xmax, ymax = from_wkt(geo_wkt).bounds

            old_xmin = self._x_min(cur)
            old_xmax = self._x_max(cur)
            old_ymin = self._y_min(cur)
            old_ymax = self._y_max(cur)

            cur.executemany(
                "INSERT OR REPLACE INTO metadata(key, value) VALUES(?,?)",
                [
                    ("xmin", str(min(xmin, float(old_xmin)))),
                    ("xmax", str(max(xmax, float(old_xmax)))),
                    ("ymin", str(min(ymin, float(old_ymin)))),
                    ("ymax", str(max(ymax, float(old_ymax)))),
                ],
            )
            if not self._buffer_in_memory:
                cur.execute("COMMIT")
            else:
                # Trigger flush if buffer full
                self._buffered_frame_count += 1
                if self._buffered_frame_count >= self._max_buffered_frames:
                    self.flush()
                return
        except sqlite3.Error as e:
            cur.execute("ROLLBACK")
            raise TrajectoryWriter.Exception(f"Error writing to database: {e}")


    def flush(self) -> None:
        """Flush any buffered frames to disk in a single transaction.

        Safe to call multiple times; if the buffer is empty, this won't execute.
        """
        if not self._buffer_in_memory:
            return
        if self._buffered_frame_count == 0:
            return
        cur = self._con.cursor()
        try:
            cur.execute("COMMIT")
        except sqlite3.Error as e:
            cur.execute("ROLLBACK")
            raise TrajectoryWriter.Exception(f"Error writing to database: {e}")
        finally:
            # Reset buffered frame count regardless of success/failure
            self._buffered_frame_count = 0

    def close(self) -> None:
        """Flush buffer and close DB connection. Call at simulation end."""
        # Flush pending data (if any)
        if self._buffer_in_memory:
            self.flush()
        if self._con:
            try:
                self._con.close()
            finally:
                self._con = None  # type: ignore[assignment]
    
    def every_nth_frame(self) -> int:
        return self._every_nth_frame

    def connection(self) -> sqlite3.Connection:
        return self._con

    def _value_or_default(self, cur, key, default: float | int | str):
        res = cur.execute(
            "SELECT value FROM metadata WHERE key = ?", (key,)
        ).fetchone()
        if res is None:
            return default
        else:
            text = res[0]
            return type(default)(text)

    def _x_min(self, cur):
        return self._value_or_default(cur, "xmin", float("inf"))

    def _x_max(self, cur):
        return self._value_or_default(cur, "xmax", float("-inf"))

    def _y_min(self, cur):
        return self._value_or_default(cur, "ymin", float("inf"))

    def _y_max(self, cur):
        return self._value_or_default(cur, "ymax", float("-inf"))


def update_database_to_latest_version(connection: sqlite3.Connection):
    version = get_database_version(connection)

    if version == 1:
        convert_database_v1_to_v2(connection)
        version = 2

    # if version == 2:
    #     convert_database_v2_to_v3
    #     version = 3
    # ... for future versions


def convert_database_v1_to_v2(connection: sqlite3.Connection):
    cur = connection.cursor()

    try:
        cur.execute("BEGIN")

        version = get_database_version(connection)
        if version != 1:
            raise RuntimeError(
                f"Internal Error: When converting from database version 1 to 2, encountered database version {version}."
            )

        cur.execute(
            "UPDATE metadata SET value = ? WHERE key = ?", (2, "version")
        )

        cur.execute(
            "CREATE TABLE frame_data("
            "   frame INTEGER NOT NULL,"
            "   geometry_hash INTEGER NOT NULL)"
        )

        res = cur.execute("SELECT wkt FROM geometry")
        wkt_str = res.fetchone()[0]
        wkt_hash = hash(wkt_str)

        cur.execute("ALTER TABLE geometry ADD hash INTEGER NOT NULL DEFAULT 0")
        cur.execute("UPDATE geometry SET hash = ?", (wkt_hash,))

        res = cur.execute("SELECT max(frame) FROM trajectory_data")
        frame_limit = res.fetchone()[0] + 1

        cur.executemany(
            "INSERT INTO frame_data VALUES(?, ?)",
            zip(range(frame_limit), itertools.repeat(wkt_hash)),
        )
        cur.execute("COMMIT")
        cur.execute("VACUUM")
    except sqlite3.Error as e:
        cur.execute("ROLLBACK")
        raise TrajectoryWriter.Exception(f"Error writing to database: {e}")