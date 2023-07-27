""" Serialization/deserialization support

In this file you will find interfaces and implementations to serialize and
deserialize different forms of input / output commonly used.

"""

import abc
import math
from io import TextIOWrapper
from pathlib import Path
from typing import Optional, Tuple

import shapely

from jupedsim import (
    GCFMModelAgentParameters,
    Simulation,
    VelocityModelAgentParameters,
)


class TrajectoryWriter(metaclass=abc.ABCMeta):
    """Interface for trajectory serialization"""

    @abc.abstractmethod
    def begin_writing(self) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as framerate etc...
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
    def end_writing(self) -> None:
        """End writing trajectory data.

        This method is intended to handle finalizing writing of trajectory
        data, e.g. write closing tags, or footer meta data.
        """
        raise NotImplementedError

    class Exception(Exception):
        """Represents exceptions specific to the trajectory writer."""

        pass


class JpsCoreStyleTrajectoryWriter(TrajectoryWriter):
    """Writes jpscore / jpsvis compatible trajectory files w.o. a referenced geometry.

    This implementation tracks the number of calls to 'write_iteration_state'
    and inserts the appropriate frame number, to write a useful file header the
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

    def write_iteration_state(self, simulation: Simulation) -> None:
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

        def agent_orientaion_as_degrees(
            agent: GCFMModelAgentParameters | VelocityModelAgentParameters,
        ) -> float:
            return JpsCoreStyleTrajectoryWriter._orientation_to_angle(
                agent.orientation
            )

        self._out.writelines(
            map(
                lambda agent: f"{agent.id}\t{self._frame}\t{agent.position.x}\t{agent.position.y}\t{0}\t{0.3}\t{0.3}\t{agent_orientaion_as_degrees(agent)}\t{0.4}\n",
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


class ParseException(Exception):
    pass


def parse_dlr_ad_hoc(input: str) -> shapely.GeometryCollection:
    """
    This function parses data from an ad-hoc file format as it was used by the DLR to
    specify accessible areas, E.g:
        Lane :J1_w0_0
        98.5
        5
        98.5
        -5
        101.5
        -1.5
        101.5
        1.5
        Lane :J2_w0_0
        198.5
        1.5
        198.5
        -1.5
        201.5
        -5
        201.5
        5
    Identifiers are followed by x and y coordinates, each specifying a polygon.

    Parameters
    ----------
    input : str
        text in the above mentioned format

    Returns
    -------
    shapely.GeometryCollection that only contains polygons
    """
    polygons = []
    laneName = None
    laneCoordinates = []
    for line in input.splitlines():
        if line.startswith("Lane"):
            if laneName:
                polygons.append(laneCoordinates)
                laneCoordinates = []
            laneName = line[5:-1]
        else:
            laneCoordinates.append(float(line))
    polygons.append(laneCoordinates)

    def into_poly(values: list[float]) -> shapely.Polygon:
        points = [(x, y) for x, y in zip(values[::2], values[1::2])]
        return shapely.Polygon(points)

    return shapely.GeometryCollection(geoms=[into_poly(p) for p in polygons])


def parse_wkt(input: str) -> shapely.GeometryCollection:
    """
    Creates a Geometry collection from a WKT collection

    Parameters
    ----------
    input : str
        text containing one WKT GEOMETRYCOLLECTION

    Raises
    ------
    ParseException will be raised on any errors parsing the input

    Returns
    -------
    A shapely.GeometryCollection that only contains polygons
    """

    result = None
    try:
        result = shapely.from_wkt(input)
    except Exception as e:
        raise ParseException(f"Error parsing input: {e}")
    if not isinstance(result, shapely.GeometryCollection):
        raise ParseException(
            f"Expected a WKT containing exactly one GeometryCollection"
        )
    return result
