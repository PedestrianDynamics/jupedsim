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
        """initialises frame rate and geometry, the starting Frame and the last Frame,
         checks if all Frames are included in between the start and end Frame"""
        self.path = path
        try:
            with open(self.path, "r") as trajec:
                frame_found = False
                geometry_found = False
                for line in trajec:
                    if line == "\n" or line.startswith("#"):
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
                        continue
                    else:
                        value = Frame_from_Line(line)
                        self.start_frame = value
                        break
                if not frame_found:
                    raise IncorrectTrajectory(f"file named '{self.path}' is missing a frame rate in the Header")
                if not geometry_found:
                    raise IncorrectTrajectory(f"file named '{self.path}' is missing a Geometry in the Header")

                # this will continue iteration where we left off the last time
                # All headers and empty lines are skipped only frame lines remain
                last_frame = self.start_frame
                for line in trajec:
                    current_frame = Frame_from_Line(line)
                    if current_frame != last_frame and current_frame != (last_frame + 1):
                        raise IncorrectTrajectory(
                            f"file {self.path} has missing frames ~ missed frame {last_frame + 1}")
                    last_frame = current_frame
                # if the file is completely iterated through all Frames are included
                value = last_frame
                self.end_frame = value

        except FileNotFoundError as error:
            raise IncorrectTrajectory(f"the file '{error.filename}' can not be found")
        except IndexError:
            # if this Error occurs no Frame could be found in a line
            raise IncorrectTrajectory(f"the file '{self.path}' is no correct Trajectory-file")

    def __lt__(self, other):
        return self.start_frame < other.start_frame


def Frame_from_Line(line):
    frame = line.split("\t")
    return int(frame[1])


def check_header(trajecs):
    """checks if frame rates and geometries match"""
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


def check_data(trajecs):
    """checks if starting Frames and end Frames are matching one Trajectory"""
    end_frames = []
    start_frames = []
    for trajec in trajecs:
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
      "trajecs" is a list of all Trajectory files in Class format"""

    # Trajectories are sorted after starting Frame
    trajecs.sort()
    debug_length = len(trajecs)

    for i, trajectory in enumerate(trajecs):
        if verbose:
            log.debug(f"writing file {i + 1}/{debug_length}")
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


def configuration(args):
    """sets the Logger and Argumentparser up"""
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
    return args


def main():
    args = None
    args = configuration(args)

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
