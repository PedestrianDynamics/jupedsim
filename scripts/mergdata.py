import argparse

parser = argparse.ArgumentParser()
parser.add_argument("merge", type=str, help="Merge the given Trajectories together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt", help="define outputfile",
                    dest="output")
parser.add_argument('-d', action="store_true", help="activates debugmode", dest="debug")
args = parser.parse_args()


def FramefromLine(line):
    frame = line.split("	")
    return int(frame[1])


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
    while index < len(trajecs):
        temp_trajec = open(trajecs[index], "r")
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
    output_file = open(output, 'w')
    index = 0
    while index < len(trajecs):
        counts += [head_end]
        index = index + 1

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


addDatas(args.merge, args.output, args.debug)