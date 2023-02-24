""" Serialization support

In this file you will find interfaces and impementations to serialize and
deserialize different forms of input / ouput commonly used.

"""

import abc
import math
from io import TextIOWrapper
from pathlib import Path
from typing import Optional, Tuple

import py_jupedsim as jps


class TrajectoryWriter(metaclass=abc.ABCMeta):
    """Interface for trajectory serialization"""

    @abc.abstractmethod
    def begin_writing(self) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be writte. E.g. Meta information
        such as framerate etc...
        """
        raise NotImplementedError

    @abc.abstractmethod
    def write_iteration_state(self, simulation: jps.Simulation) -> None:
        """Write trajectory data of one simulation iteration.

        This method is intended to handle serialization of the trajectory data
        of a single iteration.
        """
        raise NotImplementedError

    @abc.abstractmethod
    def end_writing(self) -> None:
        """End writing trajectory data.

        This method is intended to handle finalizing wirting of trajectory
        data, e.g. write closing tags, or footer meta data.
        """
        raise NotImplementedError

    class Exception(Exception):
        """Represents exceptions specific to the trjectory writer."""

        pass


class JpsCoreStyleTrajectoryWriter(TrajectoryWriter):
    """Writes jpscore / jpsvis compatible trjectory files w.o. a referenced geometry.

    This implementation tracks the number of calls to 'write_iteration_state'
    and inserts the appropiate frame number, to write a usefull file header the
    fps the data is written in needs to be supplied on construction
    """

    def __init__(self, output_file: Path):
        """JpsCoreStyleTrajectoryWriter constructor

        Parameters
        ----------
        output_file : pathlib.Path
            name of the output file.
            Note: the file will not be written until the first call to 'begin_writing'

        Returns
        -------
        JpsCoreStyleTrajectoryWriter
        """
        self._output_file = output_file
        self._out: Optional[TextIOWrapper] = None
        self._frame = 0

    def begin_writing(self, fps: float) -> None:
        """Writes trajectory file header information

        Parameters
        ----------
        fps: float
            fps of the data to be written

        Raises
        ------
        IOError
            Opens the output file with pathlib.Path.open(). Any exception passed on.
        """
        self._out = self._output_file.open(mode="w", encoding="utf-8")
        self._out.writelines(
            [
                "# written by JpsCoreStyleTrajectoryWriter\n",
                f"#framerate: {fps}\n",
                "#unit: m\n",
            ]
        )
        self._out.flush()

    def write_iteration_state(self, simulation: jps.Simulation) -> None:
        """Writes trajectory information for a single iteration.

        Parameters
        ----------
        simulation : jupedsim.Simulation
            The simulation object to get the trajectory data from

        Raises
        ------
        TrajectoryWriter.Exception
            Will be raised if the output file is not yet opened, i.e.
            'being_writing' has not been called yet.

        """
        if not self._out:
            raise TrajectoryWriter.Exception("Output file not opened")

        def agent_orientaion_as_degrees(agent: jps.Agent) -> float:
            vec2 = (agent.orientation_x, agent.orientation_y)
            return JpsCoreStyleTrajectoryWriter._orientation_to_angle(
                JpsCoreStyleTrajectoryWriter._normalize(vec2)
            )

        self._out.writelines(
            map(
                lambda agent: f"{agent.id}\t{self._frame}\t{agent.x}\t{agent.y}\t{0}\t{0.3}\t{0.3}\t{agent_orientaion_as_degrees(agent)}\t{0.4}\n",
                simulation.agents(),
            ),
        )
        self._frame += 1

    def end_writing(self) -> None:
        """End writing trajectory information

        Will close the file handle and end writing.
        """
        if self._out:
            self._out.close()

    # TODO(kkratz): This should be externalized
    @staticmethod
    def _normalize(vec2: Tuple[float, float]) -> Tuple[float, float]:
        len = math.sqrt(vec2[0] * vec2[0] + vec2[1] * vec2[1])
        return (vec2[0] / len, vec2[1] / len)

    # TODO(kkratz): This should be externalized
    @staticmethod
    def _orientation_to_angle(vec2: Tuple[float, float]) -> float:
        vec2 = JpsCoreStyleTrajectoryWriter._normalize(vec2)
        return math.degrees(math.atan2(vec2[1], vec2[0]))
