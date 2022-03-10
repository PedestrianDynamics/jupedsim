import argparse

import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("merge", type=str, help="Merge the given Trajectories together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt", help="define outputfile",
                    dest="output")
parser.add_argument('-d', action="store_true", help="activates debugmode", dest="debug")
args = parser.parse_args()


def FramefromLine(line):
    frame = line.split("\t")
    return int(frame[1])


def checkhead(trajecs):
    """checks if frame rate and geometries match"""
    # reads all framerates and geometries from the trajectories
    index = 0
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

    # compares if the are identical
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
    data = np.loadtxt(file)
    i = 0
    frame = data[0][1]
    while data[i][1] < data[-1][1]:
        if frame == data[i][1]:
            frame = frame + 1
        else:
            if frame < data[i][1]:
                return False
        i = i + 1
    if frame == data[-1][1]:
        return True
    return False


def startingFrame(file):
    data = np.loadtxt(file)
    return data[0][1]


def endingFrame(file):
    data = np.loadtxt(file)
    return data[-1][1]


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
    return True


def addDatas(trajecs, output, debug):
    """will merge all data into the output-file
      "trajecs" is a list of all Trajectory files"""
    lines = []  # lines[i][j] | i - file | j - rows from that file
    for trajec in trajecs:
        with open(trajec, "r") as temp_trajec:
            temp_line = temp_trajec.readlines()
            lines.append(temp_line)

    # determines in which line the header ends
    # asumes that all trajectory heads start at the same line
    head_end = 0
    for line in lines[0]:
        if line.startswith("#") or line == "\n":
            head_end += 1
        else:  # now the trajectories start
            break

    # creates as many counters as files starting from row where trajec starts
    counts = []
    for each in trajecs:
        counts.append(head_end)

    with open(output, 'a') as output_file:
        while len(lines) > 0:
            found = False
            current_frames = []
            index = 0
            while index < len(lines):
                current_frames.append(FramefromLine(lines[index][(counts[index])]))
                index = index + 1
            next_frame = min(current_frames)
            if debug:
                print(f"current Frame : {next_frame} remaining Files: {len(lines)}")
            index = 0
            # adds row with next frame to the output file
            while index < len(lines):
                if FramefromLine(lines[index][(counts[index])]) == next_frame and found is False:
                    output_file.write(lines[index][counts[index]])
                    counts[index] += 1
                    found = True
                    # removes file and counter from lines if end has been reached
                    if counts[index] >= len(lines[index]):
                        counts.pop(index)
                        lines.pop(index)
                index = index + 1
    print(f'new File "{output}" was created.')


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
