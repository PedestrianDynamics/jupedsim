import argparse

parser = argparse.ArgumentParser()
parser.add_argument("merge", type=str, help="Merge the given Trajectories together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="outputfile.txt", help="define outputfile",
                    dest="output")
parser.add_argument('-d', action="store_true", help="activates debugmode", dest="debug")
args = parser.parse_args()


def FramefromLine(line):
    frame = line.split("\t")
    return int(frame[1])


def checkhead(trajecs):
    """checks if frame rate and geometries match"""
    # reads all framerates and geometries from the trajectories
    frame_rates = []
    geometries = []
    for trajec in trajecs:
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

    # compares if they are identical
    first_rate = frame_rates[0]
    first_geo = geometries[0]
    for rate in frame_rates:
        if rate != first_rate:
            return False
    for geo in geometries:
        if geo != first_geo:
            return False
    return True


def isComplete(file):
    """test if all frames are included"""
    with open(file, "r") as opened_file:
        lines = opened_file.readlines()
    not_yet_found = -5
    last_frame = not_yet_found
    for line in lines:
        if line != "\n" and not line.startswith("#"):
            current_frame = FramefromLine(line)
            if last_frame == not_yet_found:
                last_frame = current_frame
            elif last_frame != current_frame and last_frame + 1 != current_frame:
                print(f"{last_frame} & {current_frame}")
                return False
            else:
                last_frame = current_frame
    return True


def startingFrame(file):
    with open(file, "r") as opened_file:
        temp_line = opened_file.readlines()
    for line in temp_line:
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
            return False
        # a starting frame is invalid if it appears more than once
        if start_frames.count(start_frame) > 1:
            return False
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
    with open (inputfile, "r") as input:
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


if args.debug:
    print("it will be checked if the Trajectories match")
if not checkhead(args.merge):
    print("the Trajectories do not have matching Framerate or Geometry")
    exit()
if not checkdata(args.merge):
    print("the Trajectories do not have matching frames")
    exit()
if args.debug:
    print("Trajectories match")
addInfos(args.merge, args.output)
addDatas(args.merge, args.output, args.debug)
