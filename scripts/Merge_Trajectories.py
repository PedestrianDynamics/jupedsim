#! /usr/bin/env python3
import argparse
import logging
import sys
import logging as log

log.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)
parser = argparse.ArgumentParser()
parser.add_argument("file_location", type=str, help="Merge the given Trajectory files together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt",
                    help="define outputfile, default path is outputfile.txt",
                    dest="output_path")
parser.add_argument('-v', action="store_true", help="activates verbosemode and shows how many files remain",
                    dest="verbose")
args = parser.parse_args()


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

    def setup(self):
        frame_rates = []
        geometries = []
        try:
            with open(self.path, "r") as trajec:
                frame_found = False
                geometry_found = False
                for line in trajec:
                    if line.startswith("#framerate"):
                        split_line = line.split(": ")
                        self.framerate = [split_line[1]]
                        frame_found = True
                    if line.startswith("#geometry"):
                        split_line = line.split(": ")
                        self.geometry = [split_line[1]]
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
    # reads all frame rates and geometries from the trajectories
    frame_rates = []
    geometries = []
    for trajec in trajecs:
        try:
            with open(trajec, "r") as temp_trajec:
                rate_found = False
                geometry_found = False
                for line in temp_trajec:
                    if not line.startswith('#'):
                        break
                    if line.startswith("#framerate"):
                        split_line = line.split(": ")
                        frame_rates += [split_line[1]]
                        rate_found = True
                    if line.startswith("#geometry"):
                        split_line = line.split(": ")
                        geometries += [split_line[1]]
                        geometry_found = True
                if not rate_found:
                    raise IncorrectTrajectory(f"file named '{trajec}' is missing a frame rate in the Header")
                if not geometry_found:
                    raise IncorrectTrajectory(f"file named '{trajec}' is missing a Geometry in the Header")

        except FileNotFoundError as error:
            raise IncorrectTrajectory(f"the file {error.filename} can not be found")

    # compares if the values are identical
    values = zip(trajecs, frame_rates, geometries)
    # Since all geometry filenames and fps values have to be the same we just pick the first one
    _, expected_frame_rate, expected_geometry_file = next(values)
    for trajectory_file, frame_rate, geometry_file in values:
        if frame_rate != expected_frame_rate:
            raise IncorrectTrajectory(f"the frame rate in file {trajectory_file} is not matching")
        if geometry_file != expected_geometry_file:
            raise IncorrectTrajectory(f"the Geometry path in file {trajectory_file} is not matching")
    return True


def is_complete(file):  # todo two separate for-loops won´t work
    """test if all frames are included"""
    last_frame = None
    count = 0
    with open(file, 'r') as f:
        for line in f:
            if line == "\n" or line.startswith("#"):
                # skip header and empty lines
                continue
            count += 1
            current_frame = Frame_from_Line(line)
            if last_frame is None:
                last_frame = current_frame
                continue
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


def check_data(files):
    """checks if files are matching one Trajectory"""
    end_frames = []
    start_frames = []
    for file in files:
        if not is_complete(file):
            return False
        start_frames.append(starting_Frame(file))
        end_frames.append(ending_Frame(file))
    for start_frame in start_frames:
        # a starting frame is considered valid if it is the first frame of the given trajectories
        # a starting frame is considered valid if it comes after an end frame
        if start_frame != min(start_frames) and start_frame - 1 not in end_frames:
            raise IncorrectTrajectory(f"the starting frame {start_frame} is not matching the other files")
        # a starting frame is invalid if it appears more than once
        if start_frames.count(start_frame) > 1:
            raise IncorrectTrajectory(f"multiple files start with the frame {start_frame}")
    return True


def merge_trajectories(trajecs, output, verbose):  # todo needs class-implementation
    """will merge all data into the output-file
      "trajecs" is a list of all Trajectory files"""
    files = trajecs

    # creates a list with all first Frames
    first_frames = []
    for file in files:
        first_frames.append(starting_Frame(file))

    debug_counter = 1
    debug_length = len(first_frames)
    # until all files are written
    while first_frames:
        # determines the lowest starting frame
        next_frame = min(first_frames)
        for i, frame in enumerate(first_frames):
            # adds all data from that file to the output file
            if frame == next_frame:
                if verbose:
                    log.debug(f"writing file {debug_counter}/{debug_length}")
                append_to_output(files[i], output)
                # removes file and the first line from the lists after the data has been written
                files.pop(i)
                first_frames.pop(i)
                debug_counter += 1
                break

    log.info(f'new File "{output}" was created.')


def append_to_output(input_file, output):  # todo readlines() could be removed
    """writes all data from the input-file onto the output-file"""
    with open(input_file, "r") as input:
        inputdata = input.readlines()
    with open(output, "a") as output_file:
        for line in inputdata:
            if line != "\n" and not line.startswith("#"):
                output_file.write(line)


def add_Header(trajecs, output):  # todo: change header
    with open(trajecs[0], "r") as temp_trajec, open(output, "w") as output_file:
        lines = temp_trajec.readlines()
        counter = 0
        temp_line = lines[counter]
        while temp_line.startswith("#") or temp_line == "\n":
            if temp_line.startswith("#description:"):
                output_file.write(temp_line.strip('\n') + " merged with MergeTool\n")
            else:
                output_file.write(temp_line)
            counter += 1
            temp_line = lines[counter]


try:
    if args.verbose:
        log.debug("it will be checked if the Trajectories match")
    check_header(args.file_location)
    check_data(args.file_location)
    if args.verbose:
        log.debug("Trajectories match")
except IncorrectTrajectory as error:
    log.error(error)
    log.info("Use -h for more info on how to use the Script")
    sys.exit(1)

add_Header(args.file_location, args.output_path)
merge_trajectories(args.file_location, args.output_path, args.verbose)

# todo if __name__ == __main__
