import argparse

parser = argparse.ArgumentParser()
parser.add_argument("file_location", type=str, help="Merge the given Trajectory files together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt",
                    help="define outputfile, default path is outputfile.txt",
                    dest="output_path")
parser.add_argument('-d', action="store_true", help="activates debugmode and shows how many files remain", dest="debug")
args = parser.parse_args()


class IncorrectTrajectoryException(Exception):

    def __init__(self, message):
        self.message = message


def FramefromLine(line):
    frame = line.split("\t")
    return int(frame[1])


def checkhead(trajecs):
    """checks if frame rate and geometries match"""
    # reads all framerates and geometries from the trajectories
    frame_rates = []
    geometries = []
    for trajec in trajecs:
        try:
            with open(trajec, "r") as temp_trajec:
                lines = temp_trajec.readlines()
                counter = 0
                temp_line = lines[counter]
                while temp_line.startswith("#"):
                    if temp_line.startswith("#framerate"):
                        split_line = temp_line.split(": ")
                        frame_rates += [split_line[1]]
                    if temp_line.startswith("#geometry"):
                        split_line = temp_line.split(": ")
                        geometries += [split_line[1]]
                    counter += 1
                    temp_line = lines[counter]
        except FileNotFoundError as error:
            raise IncorrectTrajectoryException(f"the file {error.filename} can not be found")

    if len(frame_rates) != len(trajecs):
        raise IncorrectTrajectoryException("One file is missing a frame rate in the Header")
    if len(geometries) != len(trajecs):
        raise IncorrectTrajectoryException("One file is missing a Geometry path in the Header")

    # compares if they are identical
    first_rate = frame_rates[0]
    first_geo = geometries[0]
    for i, rate in enumerate(frame_rates):
        if rate != first_rate:
            raise IncorrectTrajectoryException(f"the framerate in file {trajecs[i]} is not matching")
    for i, geo in enumerate(geometries):
        if geo != first_geo:
            raise IncorrectTrajectoryException(f"the Geometry path in file {trajecs[i]} is not matching")
    return True


def isComplete(file):
    """test if all frames are included"""
    with open(file, "r") as opened_file:
        not_yet_found = -5
        last_frame = not_yet_found
        for line in opened_file:
            if line != "\n" and not line.startswith("#"):
                current_frame = FramefromLine(line)
                if last_frame == not_yet_found:
                    last_frame = current_frame
                elif last_frame != current_frame and last_frame + 1 != current_frame:
                    raise IncorrectTrajectoryException(
                        f"file {file} has missing frames ~ missed frame {last_frame+1}")
                else:
                    last_frame = current_frame
    return True


def startingFrame(file):
    with open(file, "r") as opened_file:
        for line in opened_file:
            if line != "\n" and not line.startswith("#"):
                return FramefromLine(line)


def endingFrame(file):
    with open(file, "r") as opened_file:
        lines = opened_file.readlines()
        lines.reverse()

    for line in lines:
        if line != "\n" and not line.startswith("#"):
            return FramefromLine(line)


def checkdata(files):
    """checks if files are matching one Trajectory"""
    end_frames = []
    start_frames = []
    for file in files:
        if not isComplete(file):
            return False
        start_frames.append(startingFrame(file))
        end_frames.append(endingFrame(file))
    for start_frame in start_frames:
        # a starting frame is considered valid if it is the first frame of the given trajectories
        # a starting frame is considered valid if it comes after an end frame
        if start_frame != min(start_frames) and start_frame - 1 not in end_frames:
            raise IncorrectTrajectoryException(f"the starting frame {start_frame} is not matching the other files")
        # a starting frame is invalid if it appears more than once
        if start_frames.count(start_frame) > 1:
            raise IncorrectTrajectoryException(f"multiple files start with the frame {start_frame}")
    return True


def addDatas(trajecs, output, debug):
    """will merge all data into the output-file
      "trajecs" is a list of all Trajectory files"""
    files = trajecs

    # creates a list with all first Frames
    first_frames = []
    for file in files:
        first_frames.append(startingFrame(file))

    debug_counter = 1
    debug_length = len(first_frames)
    # until all files are written
    while first_frames:
        # determines the lowest starting frame
        next_frame = min(first_frames)
        for i, frame in enumerate(first_frames):
            # adds all data from that file to the output file
            if frame == next_frame:
                if debug:
                    print(f"writing file {debug_counter}/{debug_length}")
                write(files[i], output)
                # removes file and the first line from the lists after the data has been written
                files.pop(i)
                first_frames.pop(i)
                debug_counter += 1
                break

    print(f'new File "{output}" was created.')


def write(inputfile, output):
    """writes all data from the inputfile onto the output file"""
    with open(inputfile, "r") as input:
        inputdata = input.readlines()
    with open(output, "a") as output_file:
        for line in inputdata:
            if line != "\n" and not line.startswith("#"):
                output_file.write(line)


def addInfos(trajecs, output):
    with open(trajecs[0], "r") as temp_trajec:
        lines = temp_trajec.readlines()
        counter = 0
        with open(output, "w") as output_file:
            temp_line = lines[counter]
            while temp_line.startswith("#") or temp_line == "\n":
                if temp_line.startswith("#description:"):
                    output_file.write(temp_line.strip('\n') + " merged with MergeTool\n")
                else:
                    output_file.write(temp_line)
                counter += 1
                temp_line = lines[counter]

try:
    if args.debug:
        print("it will be checked if the Trajectories match")
    checkhead(args.file_location)
    checkdata(args.file_location)
    if args.debug:
        print("Trajectories match")
except IncorrectTrajectoryException as error:
    print(error.message)
    print("Use -h for more info on how to use the Skript")
    exit()

addInfos(args.file_location, args.output_path)
addDatas(args.file_location, args.output_path, args.debug)
