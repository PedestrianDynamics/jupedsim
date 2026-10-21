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


class _ClosingWriter(_RecordingWriter):
    """Records which of close() and close_after_error() ran."""

    def __init__(self):
        super().__init__()
        self.calls = []

    def close(self):
        self.calls.append("close")


def test_with_block_closes_a_writer():
    with _ClosingWriter() as writer:
        pass
    assert writer.calls == ["close"]


def test_with_block_ending_on_an_error_closes_after_error():
    class BlockError(Exception):
        pass

    writer = _ClosingWriter()
    try:
        with writer:
            raise BlockError
    except BlockError:
        pass
    # close_after_error() falls back to close() unless a writer overrides it.
    assert writer.calls == ["close"]

    class Overriding(_ClosingWriter):
        def close_after_error(self):
            self.calls.append("close_after_error")

    writer = Overriding()
    try:
        with writer:
            raise BlockError
    except BlockError:
        pass
    assert writer.calls == ["close_after_error"]
