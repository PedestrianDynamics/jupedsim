# SPDX-License-Identifier: LGPL-3.0-or-later
"""Context manager behaviour of the TrajectoryWriter interface."""

import jupedsim as jps
import shapely


def _square_simulation(writer):
    surface = jps.WalkableSurface()
    surface.add_region(
        polygon=shapely.Polygon([(0, 0), (5, 0), (5, 5), (0, 5)])
    )
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=surface,
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
