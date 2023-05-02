import sqlite3
from pathlib import Path
from typing import Optional

import py_jupedsim as jps
from jupedsim.serialization import TrajectoryWriter


class SqliteTrajectoryWriter(TrajectoryWriter):
    """Write trajectory data into a sqlite db"""

    def __init__(self, output_file: Path):
        """SqliteTrajectoryWriter constructor

        Parameters
        ----------
        output_file : pathlib.Path
            name of the output file.
            Note: the file will not be written until the first call to 'begin_writing'

        Returns
        -------
        SqliteTrajectoryWriter
        """
        self._output_file = output_file
        self._frame = 0
        self._con: Optional[sqlite3.Connection] = None

    def begin_writing(self, fps: float) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as framerate etc...
        """
        self._con = sqlite3.connect(self._output_file, isolation_level=None)
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
                "CREATE TABLE metadata(key TEXT NOT NULL UNIQUE, value TEXT NOT NULL)"
            )
            cur.executemany(
                "INSERT INTO metadata VALUES(?, ?)",
                (("version", "1"), ("fps", fps)),
            )
            cur.execute("COMMIT")
        except sqlite3.Error as e:
            cur.execute("ROLLBACK")
            raise TrajectoryWriter.Exception(f"Error creating database: {e}")

    def write_iteration_state(self, simulation: jps.Simulation) -> None:
        """Write trajectory data of one simulation iteration.

        This method is intended to handle serialization of the trajectory data
        of a single iteration.
        """
        if not self._con:
            raise TrajectoryWriter.Exception("Database not opened.")

        cur = self._con.cursor()
        try:
            cur.execute("BEGIN")
            frame_data = [
                (
                    self._frame,
                    agent.id,
                    agent.position.x,
                    agent.position.y,
                    agent.orientation.x,
                    agent.orientation.y,
                )
                for agent in simulation.agents()
            ]
            cur.executemany(
                "INSERT INTO trajectory_data VALUES(?, ?, ?, ?, ?, ?)",
                frame_data,
            )
            cur.execute("COMMIT")
        except:
            cur.execute("ROLLBACK")
        self._frame += 1

    def end_writing(self) -> None:
        """End writing trajectory data.

        This method is intended to handle finalizing writing of trajectory
        data, e.g. write closing tags, or footer meta data.
        """
        if self._con:
            self._con.close()
