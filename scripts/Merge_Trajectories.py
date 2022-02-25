import argparse
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("merge", type=str, help="Merge the given Trajectories together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt", help="define outputfile",
                    dest="output")
parser.add_argument('-d', action="store_true", help="activates debugmode", dest="debug")
args = parser.parse_args()


def FramefromLine(line):
    frame = line.split("	")
    return int(frame[1])

# checks if frame rate and geometries match
def checkhead(trajecs):
    # reads all framerates and geometries from the trajectories
    index = 0
    frame_rates = []
    geometries = []
    while index < len(trajecs):
        temp_trajec = open(trajecs[index], "r")
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
        temp_trajec.close()
        index = index + 1
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


# test if all frames are included
def checkcomplete(file):
    data = np.loadtxt(file)
    i = 0
    frame = data[0][1]
    while data[i][1] < data[-1][1]:
        if frame == data[i][1]:
            frame = frame + 1
        else:
            if frame < data[i][1]:
                return True
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


# checks if files are matching one Trajectory
def checkdata(files):
    end_frames = []
    start_frames = []
    for file in files:
        if not checkcomplete(file):
            return False
        start_frames += [startingFrame(file)]
        end_frames += [endingFrame(file)]
    for start_frame in start_frames:
        if start_frame != min(start_frames) and start_frame - 1 not in end_frames:
            return False
    return True

# merges Data for exactly two files (will be removed later)
def addData(trajec1, trajec2):
    input_file1 = open(trajec1, "r")
    lines1 = input_file1.readlines()
    input_file2 = open(trajec2, "r")
    lines2 = input_file2.readlines()
    count1 = 14
    count2 = 14
    output_file = open('src/newtesttabelle.txt', 'w')
    while count1 < len(lines1) - 1 and count2 < len(lines2) - 1:
        if (FramefromLine(lines1[count1])) < (FramefromLine(lines2[count2])):
            output_file.write(lines1[count1])
            count1 = count1 + 1
        if (FramefromLine(lines1[count1])) > (FramefromLine(lines2[count2])):
            output_file.write(lines2[count2])
            count2 = count2 + 1
        if (FramefromLine(lines1[count1])) == (FramefromLine(lines2[count2])):
            print("that did not work out")
            return
    while count1 < len(lines1) - 1:
        output_file.write(lines1[count1])
        count1 = count1 + 1
    while count2 < len(lines2) - 1:
        output_file.write(lines2[count2])
        count2 = count2 + 1
    output_file.close()


# will merge all data into the output-file
# "trajecs" is a list of all Trajectory files
def addDatas(trajecs, output, debug):
    lines = [[], []]  # lines[i][j] | i - index of file | j - row number
    index = 0
    for trajec in trajecs:
        temp_trajec = open(trajec, "r")
        temp_line = temp_trajec.readlines()
        lines += []
        lines[index] += temp_line
        temp_trajec.close()
        index = index + 1

    # determines in which line the header ends
    head_end = 0
    for line in lines[0]:
        if line.startswith("#") or line == "\n":
            head_end += 1
        else:  # now the trajectories start
            break

    # creates as many counters as files starting from row where trajec starts
    counts = []
    output_file = open(output, 'a')
    for each in trajecs:
        counts.append(head_end)

    # searches next frame
    while len(lines) > 0:
        found = False
        current_frames = []
        index = 0
        while index < len(lines):
            current_frames += [FramefromLine(lines[index][(counts[index])])]
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
    output_file.close()
    print(f'new File "{output}" was created.')

def addInfos(trajecs, output):
    temp_trajec = open(trajecs[0], "r")
    lines = temp_trajec.readlines()
    counter = 0
    output_file = open(output, "w")
    temp_line = lines[counter]
    while temp_line.startswith("#") or temp_line == "\n":
        if temp_line.startswith("#description:"):
            output_file.write(temp_line.strip('\n')+" merged with MergeTool\n")
        else:
            output_file.write(temp_line)
        counter += 1
        temp_line = lines[counter]
    temp_trajec.close()
    output_file.close()
if args.debug:
    print("it will be checked if the Trajectories match")
if not checkhead(args.merge) :
    print("the Trajectories do not have matching Framerate or Geometry")
    exit()
if not checkdata(args.merge):
    print("the Trajectories do not have matching frames")
    exit()
if args.debug:
    print("Trajectories match")
addInfos(args.merge, args.output)
addDatas(args.merge, args.output, args.debug)