# SPDX-License-Identifier: LGPL-3.0-or-later
"""Context manager behaviour of the TrajectoryWriter interface."""

import sqlite3
from contextlib import closing

import jupedsim as jps
import shapely
from shapely import GeometryCollection


def _square_simulation(writer):
    area = GeometryCollection(shapely.Polygon([(0, 0), (5, 0), (5, 5), (0, 5)]))
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=area,
        trajectory_writer=writer,
        dt=0.01,
    )


class _RecordingWriter(jps.TrajectoryWriter):
    """Minimal writer that does not override close()."""

    def __init__(self):
        self.frames = 0

    def begin_writing(self, simulation):
        pass

    def write_iteration_state(self, simulation):
        self.frames += 1

    def every_nth_frame(self):
        return 1


def test_custom_writer_without_close_is_context_manager():
    with _RecordingWriter() as writer:
        sim = _square_simulation(writer)
        sim.iterate()
    assert writer.frames == 2


def test_sqlite_writer_with_block(tmp_path):
    out = tmp_path / "traj.sqlite"
    with jps.SqliteTrajectoryWriter(
        output_file=out, every_nth_frame=1
    ) as writer:
        sim = _square_simulation(writer)
        for _ in range(3):
            sim.iterate()
    writer.close()  # closing again after the block must not raise

    with closing(sqlite3.connect(out)) as con:
        (count,) = con.execute(
            "SELECT COUNT(DISTINCT frame) FROM frame_data"
        ).fetchone()
    assert count == 4
