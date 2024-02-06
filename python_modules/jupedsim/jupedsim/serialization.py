# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
""" Serialization/deserialization support

In this file you will find interfaces and implementations to serialize and
deserialize different forms of input / output commonly used.
"""

import abc


class TrajectoryWriter(metaclass=abc.ABCMeta):
    """Interface for trajectory serialization"""

    @abc.abstractmethod
    def begin_writing(self, simulation) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as frame rate etc...

        """
        raise NotImplementedError

    @abc.abstractmethod
    def write_iteration_state(self, simulation) -> None:
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

    class Exception(Exception):
        """Represents exceptions specific to the trajectory writer."""

        pass
