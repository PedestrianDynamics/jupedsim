# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
"""
Provides functionality to store performance measurements in SQLite
"""

import platform

import jupedsim as jps


class StatsWriter(jps.TrajectoryWriter):
    """
    StatsWriter will recreate perf_statistics table.
    New entries can be added with write stats
    """

    def __init__(
        self,
        trajectory_writer: jps.SqliteTrajectoryWriter,
        description: str = "N/A",
    ):
        self._trajectory_writer = trajectory_writer
        self._description = description
        self._con = trajectory_writer._con

    def begin_writing(self, simulation) -> None:
        simulation.set_tracing(True)
        self._trajectory_writer.begin_writing(simulation)
        self._recreate_table()
        self.write_metadata()

    def write_iteration_state(self, simulation) -> None:
        self._trajectory_writer.write_iteration_state(simulation)
        self.write_stats(simulation)

    def end_writing(self, simulation) -> None:
        self._trajectory_writer.end_writing(simulation)

    def every_nth_frame(self) -> int:
        return self._trajectory_writer.every_nth_frame()

    def _recreate_table(self):
        """
        Recreates perf_statistics table to ensure no other data is
        present in the table or the schema is not matching
        """
        cur = self._con.cursor()
        cur.execute("DROP TABLE IF EXISTS perf_statistics")
        cur.execute(
            "CREATE TABLE perf_statistics ("
            "   frame INTEGER NOT NULL,"
            "   iteration_loop_us INTEGER NOT NULL,"
            "   operational_level_us INTEGER NOT NULL,"
            "   agent_count INTEGER NOT NULL)"
        )
        cur.close()

    def write_metadata(self):
        cur = self._con.cursor()
        cur.execute(
            "INSERT INTO metadata VALUES(?, ?)",
            ("commit_id", jps.get_build_info().git_commit_hash),
        )
        cur.execute(
            "INSERT INTO metadata VALUES(?, ?)",
            ("hostname", platform.node()),
        )
        cur.execute(
            "INSERT INTO metadata VALUES(?, ?)",
            ("description", self._description),
        )

    def write_stats(self, simulation):
        iteration = simulation.iteration_count()
        if iteration % self.every_nth_frame() != 0:
            return
        frame_idx = iteration / self.every_nth_frame()
        stats = simulation.get_last_trace()
        agent_count = simulation.agent_count()
        self._con.cursor().execute(
            "INSERT INTO perf_statistics VALUES(?,?,?,?)",
            (
                frame_idx,
                stats.iteration_duration,
                stats.operational_level_duration,
                agent_count,
            ),
        )
