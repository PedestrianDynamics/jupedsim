#!/usr/bin/env python3

import argparse
import logging as log
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np
from pandas import read_csv


def check_positive_int(value):
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError(
            f"{value} is an invalid value. Positive value required."
        )
    return ivalue


def extract_info(file_obj):
    """Extract 'unit' and 'fps' from Petrack file.

    Then append a description, a geometry and finally the
    jpscore header with the fps information.
    Keep the original header, if existing.
    # ---------------------------------------
    Assumptions being made:
    1. Header contains a line with the words 'PeTrack' or '<number>'
    2. Header contains a line with "x/unit or <x> [in unit]
    3. Header contains a line with fps information as follows:
       # framerate: N fps
    # ---------------------------------------
    :param file_obj: file object
    :returns: header,
              fps,
              and unit

    """
    unit = None
    fps = None
    for line in file_obj:
        if not line.startswith("#"):  # now the trajectories start
            break

        if "x/" in line:
            unit = line.split("x/")[-1].split()[0]

        if "<x>" in line:
            # <number> <frame> <x> [in m] <y> [in m] <z> [in m]
            unit = line.split("<x>")[-1].split()[1].strip("]")

        if "framerate" in line:
            fps = line.split("fps")[0].split()[-1]
            try:
                fps = int(float(fps))  # in some files we get float like 2.0
            except ValueError:
                raise Exception(
                    f"fps <{fps}> in header can not be converted to int"
                )

    return fps, unit


def build_header(program_name, fps):
    return (
        f"description: trajectories converted by {program_name}\n"
        + f"framerate: {fps}\n"
        + "geometry: geometry.xml\n"
        + "ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR"
    )


def compute_speed_and_angle(traj, df, fps):
    """Calculates the speed and the angle from the trajectory points.

    Using the forward formula
    speed(f) = (X(f+df) - X(f))/df [1]
    note: The last df frames are not calculated using [1].
    It is assumes that the speed in the last frames
    does not change
    :param traj: trajectory of ped (x, y). 2D array
    :param df: number of frames forwards
    :param fps: frames per seconds

    :returns: speed, angle

    example:
    df=4, S=10
         0 1 2 3 4 5 6 7 8 9
       X * * * * * * * * * *
       V + + + + + +
         *       *
           *       *      X[df:]
    X[:S-df] *       *       │
    │          *       *   ◄─┘
    └────────►   *       *
                   *       *
    """
    size = traj.shape[0]
    speed = np.ones(size)
    angle = np.zeros(size)
    if size < df:
        log.warning(
            f"""The number of frames used to calculate the speed {df}
            exceeds the total amount of frames ({size}) in this trajectory."""
        )
        return (speed, angle)

    delta = traj[df:, :] - traj[: size - df, :]
    delta_x = delta[:, 0]
    delta_y = delta[:, 1]
    delta_square = np.square(delta)
    delta_x_square = delta_square[:, 0]
    delta_y_square = delta_square[:, 1]
    angle[: size - df] = np.arctan2(delta_y, delta_x) * 180 / np.pi
    s = np.sqrt(delta_x_square + delta_y_square)
    speed[: size - df] = s / df * fps
    speed[size - df :] = speed[size - df - 1]
    angle[size - df :] = angle[size - df - 1]
    return (speed, angle)


def write_geometry(data, _unit, geo_file):
    """Creates a bounding box around the trajectories

    :param data: 2D-array
    :param Unit: Unit of the trajectories (cm or m)
    :param geo_file: write geometry in this file
    :returns: geometry file names geometry.xml

    """
    # ----------
    delta = 100 if _unit == "cm" else 1
    # 1 m around to better contain the trajectories
    xmin = np.min(data[:, 2]) - delta
    xmax = np.max(data[:, 2]) + delta
    ymin = np.min(data[:, 3]) - delta
    ymax = np.max(data[:, 3]) + delta
    # --------
    # create_geo_header
    data = ET.Element("geometry")
    data.set("version", "0.8")
    data.set("caption", "experiment")
    data.set("unit", "m")  # jpsvis does not support another unit!
    # make room/subroom
    rooms = ET.SubElement(data, "rooms")
    room = ET.SubElement(rooms, "room")
    room.set("id", "0")
    room.set("caption", "room")
    subroom = ET.SubElement(room, "subroom")
    subroom.set("id", "0")
    subroom.set("caption", "subroom")
    subroom.set("class", "subroom")
    subroom.set("A_x", "0")
    subroom.set("B_y", "0")
    subroom.set("C_z", "0")
    # poly1
    polygon = ET.SubElement(subroom, "polygon")
    polygon.set("caption", "wall")
    polygon.set("type", "internal")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmin}")
    vertex.set("py", f"{ymin}")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmax}")
    vertex.set("py", f"{ymin}")
    # poly2
    polygon = ET.SubElement(subroom, "polygon")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmax}")
    vertex.set("py", f"{ymin}")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmax}")
    vertex.set("py", f"{ymax}")
    # poly3
    polygon = ET.SubElement(subroom, "polygon")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmax}")
    vertex.set("py", f"{ymax}")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmin}")
    vertex.set("py", f"{ymax}")
    # poly4
    polygon = ET.SubElement(subroom, "polygon")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmin}")
    vertex.set("py", f"{ymax}")
    vertex = ET.SubElement(polygon, "vertex")
    vertex.set("px", f"{xmin}")
    vertex.set("py", f"{ymin}")
    b_xml = ET.tostring(data, encoding="utf8", method="xml")
    with open(geo_file, "wb") as f:
        f.write(b_xml)


def extend_data(data, _unit, a_in, b_in):
    """Append some columns to the trajectories

    For visualisation purposes. These columns are:
    - height: if missing in the trajectory file
    - A: Semi-axis of the agent
    - B: Semi-axis of the agent.
    - ANGLE: angle of the agent.
    - COLOR: Color of the agent.
    :param data: input trajectories
    :type data: np.array
    :returns: np.array

    :param data:
    :param _unit:

    """
    rows, cols = data.shape
    h = 1.5 * np.ones((rows, 1)) * _unit  # height
    a = (
        a_in * np.ones((rows, 1)) * _unit
    )  # semi-axis of ellipse in moving direction
    b = (
        b_in * np.ones((rows, 1)) * _unit
    )  # semi-axis of ellipse in shoulder direction
    angle = np.zeros((rows, 1))  # angle in degree
    color = 100 * np.ones((rows, 1))  # will be set wrt. speed
    if cols == 4:  # some trajectories do not have Z
        data = np.hstack((data, h, a, b, angle, color))
    else:
        data = np.hstack((data, a, b, angle, color))

    return data


def write_trajectories(result, header, file_path):
    """write the resulting trajectories in a file

    :param result: trajectories
    :type result: np.array
    :param header: Header
    :type header: str
    :param file_path: input file
    :type file_path: Path
    :returns: the name of the file is jps_Filename

    """
    filename = file_path.parent.joinpath("jps_" + file_path.stem + ".txt")
    np.savetxt(
        filename,
        result[1:, :],
        # skip the first line (initialization)
        fmt=[
            "%d",  # id
            "%d",  # frame
            "%1.2f",  # x
            "%1.2f",  # y
            "%1.2f",  # z
            "%1.2f",  # A
            "%1.2f",  # B
            "%1.2f",  # Angle
            "%d",  # Color
        ],
        header=header,
        comments="#",
        delimiter="\t",
    )
    size_mb = Path(filename).stat().st_size / 1024 / 1024
    log.info(f"output: {filename} ({size_mb:,.2f} MB)")


def write_info_msg(traj_file, fps, df, unit_s, a, b):
    size_mb = traj_file.stat().st_size / 1024 / 1024
    log.info(f"input : {traj_file} ({size_mb:,.2f} MiB)")
    log.info(f"unit  : {unit_s}")
    log.info(f"fps   : {fps}")
    log.info(f"df    : {df}")
    log.info(f"a     : {a} {unit_s}")
    log.info(f"b     : {b} {unit_s}")


def parse_commandline_arguments():
    parser = argparse.ArgumentParser(
        description="""Modify trajectory-files to be
        visualized with jpsvis.
        New columns 'A', 'B' representing ellipses major axis (Agent Ellipsis),
        'ANGLE' representing the Agent's orientation and
        'COLOR' representing the Agent's speed, are added.
        'A' and 'B' are constants.
        'COLOR' and 'ANGLE' are calculated based on the speed assuming
        a maximal speed of v0=1.5m/s (color = v/v0*255).
        The name of the output file is the name of the input file
        prefixed with 'jps_' and ends with '.txt'
        """
    )
    parser.add_argument("file", type=Path, help="Petrack trajectory file")
    parser.add_argument(
        "--unit",
        type=str,
        choices=["m", "cm"],
        help="""unit of the coordinates and size of pedestrians.
        Specify a unit for trajectories with no header.
        Specifying this option for an input file with a valid header is an error.
        """,
    )
    parser.add_argument(
        "--fps",
        type=check_positive_int,
        help="""frames per seconds.
        specify fps information for trajectories with no header.
        Specifying this option for an input file with a valid header is an error.
        """,
    )
    parser.add_argument(
        "--df",
        default="10",
        dest="df",
        type=check_positive_int,
        help="""number of frames forward
        to calculate the speed (default: 10)""",
    )
    parser.add_argument(
        "-a",
        default="0.2",
        type=float,
        help="""Semi-axis in moving direction (default: 0.2m)""",
    )
    parser.add_argument(
        "-b",
        default="0.3",
        type=float,
        help="""Semi-axis in shoulder direction (default: 0.3m)""",
    )
    return parser.parse_args()


def main():
    log.basicConfig(level=log.DEBUG, format="%(levelname)s : %(message)s")
    args = parse_commandline_arguments()
    input_file = args.file
    df = args.df
    try:
        with open(input_file, encoding="utf8") as finput:
            fps, unit_s = extract_info(finput)

            if fps and args.fps:
                log.error(
                    "Input file contains fps information. Do not provide --fps via the commandline."
                )
                sys.exit(1)

            if not fps and not args.fps:
                log.error(
                    "No fps information found in input file. Specify fps with '--fps'"
                )
                sys.exit(1)

            if not fps:
                fps = args.fps

            if unit_s and args.unit:
                log.error(
                    "Input file contains unit information. Do not provide --unit via the commandline."
                )
                sys.exit(1)

            if not unit_s and not args.unit:
                log.error(
                    "No unit information found in input file. Specify unit with '--unit'"
                )
                sys.exit(1)

            if not unit_s:
                unit_s = args.unit

            header = build_header(sys.argv[0], fps)

            unit = 100 if unit_s == "cm" else 1
            v0 = 1.5 * unit  # max. speed (assumed) [unit/s]
            write_info_msg(input_file, fps, df, unit_s, args.a, args.b)
            data = read_csv(
                input_file, sep=r"\s+", dtype=np.float64, comment="#"
            ).values

            columns = data.shape[1]
            log.info(f"Got {columns} columns")
            nframes = np.unique(data[:, 1]).size
            if columns > 5:
                # keep just the following cols: id fr x y z
                # and ignore the rest.
                data = data[:, :5]

            data = data[data[:, 1].argsort()]  # sort data by frame
            data = extend_data(data, unit, args.a, args.b)
            geometry_file = input_file.parent.joinpath("geometry.xml")
            write_geometry(data, unit_s, geometry_file)
            agents = np.unique(data[:, 0]).astype(int)
            log.info(f"Got {agents.size} pedestrians and {nframes} frames.")
            for agent in agents:
                ped = data[data[:, 0] == agent]
                speed, angle = compute_speed_and_angle(ped[:, 2:4], df, fps)
                data[data[:, 0] == agent, -1] = speed / v0 * 255
                data[data[:, 0] == agent, -2] = angle

            write_trajectories(data, header, input_file)

    except Exception as err:
        log.error(err)
        sys.exit(1)


if __name__ == "__main__":
    main()
