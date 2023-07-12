# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
"""
Provides functionality to store performance measurements in SQLite
"""

import sqlite3

import py_jupedsim as jps


class StatsWriter:
    """
    StatsWriter will recreate perf_statistics table.
    New entries can be added with write stats
    """

    def __init__(self, connection: sqlite3.Connection):
        """
        Parameters
        ----------
        connection : sqlite3.Connection
            Opened connection to a sqlite database.
            'perf_statistics' table will be recreated.

        Returns
        -------
        StatsWriter
        """
        self.con = connection
        self._recreate_table()

    def _recreate_table(self):
        """
        Recreates perf_statistics table to ensure no other data is
        present in the table or the schema is not matching
        """
        cur = self.con.cursor()
        cur.execute("DROP TABLE IF EXISTS perf_statistics")
        cur.execute(
            "CREATE TABLE perf_statistics ("
            "   frame INTEGER NOT NULL,"
            "   iteration_loop_us INTEGER NOT NULL,"
            "   operational_level_us INTEGER NOT NULL,"
            "   agent_count INTEGER NOT NULL)"
        )
        cur.close()

    def write_stats(self, frame_idx: int, agent_count: int, stats: jps.Trace):
        """
        Adds one set of measurements to the database.

        Parameters
        ----------
        frame_idx : int
            index of the frame that created the measurements
        agent_count : int
            numer of agents simulated in this frame
        stats : py_jupedsim.Trace
            time measurements from one iteration of the simulation
        """
        self.con.cursor().execute(
            "INSERT INTO perf_statistics VALUES(?,?,?,?)",
            (
                frame_idx,
                stats.iteration_duration,
                stats.operational_level_duration,
                agent_count,
            ),
        )
