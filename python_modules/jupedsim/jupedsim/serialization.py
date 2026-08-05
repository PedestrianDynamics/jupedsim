# SPDX-License-Identifier: LGPL-3.0-or-later
"""Serialization/deserialization support

In this file you will find interfaces and implementations to serialize and
deserialize different forms of input / output commonly used.
"""

import abc

import jupedsim.native as py_jps


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


def walkable_area_as_wkt(simulation) -> str:
    """The simulation's walkable area as WKT, for a writer's header.

    The trajectory formats JuPedSim ships are planar -- a WKT walkable area
    and (x, y) per agent -- and so are the tools reading them, PedPy among
    them. A simulation built from a surface mesh has no such area, and what a
    3D trajectory file should look like is still open. Rather than invent one
    here, the writers say so and stop.

    Raises:
        TrajectoryWriter.Exception: if the simulation was built from a surface
            mesh.
    """
    try:
        return simulation.get_geometry().as_wkt()
    except py_jps.SimulationError as e:
        raise TrajectoryWriter.Exception(
            "Cannot write trajectories for a simulation built from a surface "
            "mesh: the trajectory formats JuPedSim ships are 2D, and so are "
            "the tools reading them. Build the simulation from a polygon, or "
            "pass a trajectory_writer of your own."
        ) from e
