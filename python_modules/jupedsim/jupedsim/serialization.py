# SPDX-License-Identifier: LGPL-3.0-or-later
"""Serialization/deserialization support

In this file you will find interfaces and implementations to serialize and
deserialize different forms of input / output commonly used.
"""

from __future__ import annotations

import abc
import types
import typing

if typing.TYPE_CHECKING:
    from jupedsim.simulation import Simulation


class TrajectoryWriter(metaclass=abc.ABCMeta):
    """Interface for trajectory serialization

    Writers are context managers: leaving a ``with`` block calls
    :meth:`close`, also when the block raises::

        with jps.Hdf5TrajectoryWriter(output_file=path) as writer:
            sim = jps.Simulation(..., trajectory_writer=writer)
            while sim.agent_count() > 0:
                sim.iterate()
    """

    def __enter__(self) -> typing.Self:
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: types.TracebackType | None,
    ) -> None:
        self.close()

    def close(self) -> None:
        """Write all buffered data and release the writer's resources.

        Called when leaving a ``with`` block. Must be safe to call more than
        once. Does nothing by default; override it if the writer buffers data
        or holds resources such as open files.
        """

    @abc.abstractmethod
    def begin_writing(self, simulation: Simulation) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as frame rate etc...

        """
        raise NotImplementedError

    @abc.abstractmethod
    def write_iteration_state(self, simulation: Simulation) -> None:
        """Write trajectory data of one simulation iteration.

        This method is intended to handle serialization of the trajectory data
        of a single iteration.

        """
        raise NotImplementedError

    @abc.abstractmethod
    def every_nth_frame(self) -> int:
        """Returns the interval of this writer in frames between writes.

        1 indicates all frames are written, 10 indicates every 10th frame is
        writen and so on.

        Returns:
            Number of frames between writes as int

        """

    class Error(Exception):
        """Represents exceptions specific to the trajectory writer."""

        pass
