#! /usr/bin/env python3
import argparse
import logging
import sys
import logging as log


class IncorrectTrajectory(Exception):

    def __init__(self, message):
        self.message = message


class Trajectory:

    def __init__(self, path):
        self.path = path
        self.start_frame = starting_Frame(path)
        self.end_frame = ending_Frame(path)
        self.framerate = ""
        self.geometry = ""
        self.setup()

    def __lt__(self, other):
        return self.start_frame < other.start_frame

    def setup(self):
        """initialises framerate and geometry"""
        try:
            with open(self.path, "r") as trajec:
                frame_found = False
                geometry_found = False
                for line in trajec:
                    if line.startswith("#framerate"):
                        split_line = line.split(": ")
                        value = split_line[1]
                        value = value.strip("\n")
                        self.framerate = value
                        frame_found = True
                    if line.startswith("#geometry"):
                        split_line = line.split(": ")
                        value = split_line[1]
                        value = value.strip("\n")
                        self.geometry = value
                        geometry_found = True
                    if geometry_found and frame_found:
                        break
            if not frame_found:
                raise IncorrectTrajectory(f"file named '{trajec}' is missing a frame rate in the Header")
            if not geometry_found:
                raise IncorrectTrajectory(f"file named '{trajec}' is missing a Geometry in the Header")

        except FileNotFoundError as error:
            raise IncorrectTrajectory(f"the file {error.filename} can not be found")


def Frame_from_Line(line):
    frame = line.split("\t")
    return int(frame[1])


def check_header(trajecs):
    """checks if frame rate and geometries match"""
    # reads all frame rates and geometries from the class
    frame_rates = []
    geometries = []
    for trajec in trajecs:
        frame_rates.append(trajec.framerate)
        geometries.append(trajec.geometry)
    # compares if the values are identical
    values = zip(trajecs, frame_rates, geometries)
    # Since all geometry filenames and fps values have to be the same we just pick the first one
    _, expected_frame_rate, expected_geometry_file = next(values)
    for trajectory, frame_rate, geometry_file in values:
        if frame_rate != expected_frame_rate:
            raise IncorrectTrajectory(f"the frame rate in file {trajectory.path} is not matching")
        if geometry_file != expected_geometry_file:
            raise IncorrectTrajectory(f"the Geometry path in file {trajectory.path} is not matching")
    return True


def is_complete(trajectory):
    """test if all frames are included"""
    last_frame = None
    file = trajectory.path
    with open(file, 'r') as f:
        for line in f:
            if line != "\n" or not line.startswith("#"):
                # skip header and empty lines
                continue
            last_frame = Frame_from_Line(line)
            break
        # this will continue iteration where we left off the last time
        # All headers and empty lines are skipped only frame lines remain
        for line in f:
            current_frame = Frame_from_Line(line)
            if current_frame != last_frame and current_frame != (last_frame + 1):
                raise IncorrectTrajectory(f"file {file} has missing frames ~ missed frame {last_frame + 1}")
            last_frame = current_frame
    return True


def starting_Frame(file):
    with open(file, "r") as opened_file:
        for line in opened_file:
            if line != "\n" and not line.startswith("#"):
                return Frame_from_Line(line)


def ending_Frame(file):  # todo maybe iterate through instead
    with open(file, "r") as opened_file:
        lines = opened_file.readlines()
        lines.reverse()

    for line in lines:
        if line != "\n" and not line.startswith("#"):
            return Frame_from_Line(line)


def check_data(trajecs):
    """checks if files are matching one Trajectory"""
    end_frames = []
    start_frames = []
    for trajec in trajecs:
        if not is_complete(trajec):
            return False
        start_frames.append(trajec.start_frame)
        end_frames.append(trajec.end_frame)
    for start_frame in start_frames:
        # a starting frame is considered valid if it is the first frame of the given trajectories
        # a starting frame is considered valid if it comes after an end frame
        if start_frame != min(start_frames) and start_frame - 1 not in end_frames:
            raise IncorrectTrajectory(f"the starting frame {start_frame} is not matching the other files")
        # a starting frame is invalid if it appears more than once
        if start_frames.count(start_frame) > 1:
            raise IncorrectTrajectory(f"multiple files start with the frame {start_frame}")
    return True


def merge_trajectories(trajecs, output, verbose):
    """will merge all data into the output-file
      "trajecs" is a list of all Trajectory files"""
    trajecs.sort()
    debug_length = len(trajecs)

    for i, trajectory in enumerate(trajecs):
        if verbose:
            log.debug(f"writing file {i+1}/{debug_length}")
            append_to_output(trajectory, output)

    log.info(f'new File "{output}" was created.')


def append_to_output(trajectory, output):
    """writes all data from the input-file onto the output-file"""
    with open(trajectory.path, "r") as input_file, open(output, "a") as output_file:
        for line in input_file:
            if line != "\n" and not line.startswith("#"):
                output_file.write(line)


def add_Header(trajecs, output):
    with open(output, "w") as output_file:
        output_file.write("#merged with MergeTool\n")
        output_file.write(f"#framerate: {trajecs[0].framerate}\n")
        output_file.write(f"#geometry: {trajecs[0].geometry}\n\n")


def main():
    log.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=str, help="Merge the given Trajectory files together",
                        nargs='+')
    parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt",
                        help="define output file, default path is outputfile.txt",
                        dest="output_path")
    parser.add_argument('-v', action="store_true", help="activates verbose mode and shows how many files remain",
                        dest="verbose")
    args = parser.parse_args()

    try:
        if args.verbose:
            log.debug("it will be checked if the Trajectories match")

        # creates a list with the file in Class form
        trajecs = []
        for file in args.file:
            trajecs.append(Trajectory(file))

        check_header(trajecs)
        check_data(trajecs)
        if args.verbose:
            log.debug("Trajectories match")
    except IncorrectTrajectory as error:
        log.error(error)
        log.info("Use -h for more info on how to use the Script")
        sys.exit(1)
    add_Header(trajecs, args.output_path)
    merge_trajectories(trajecs, args.output_path, args.verbose)


if __name__ == '__main__':
    main()
