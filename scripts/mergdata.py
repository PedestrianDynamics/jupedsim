import argparse

parser = argparse.ArgumentParser()
parser.add_argument("merge", type=str,  help="Merge the given Trajectories together", nargs='+')
parser.add_argument('-o', type=str, required=False, default="src/outputfile.txt", help="define outputfile", dest="output")
args = parser.parse_args()
def FramefromLine(line):
    frame = line.split("	")
    return (int(frame[1]))


def addData(trajec1, trajec2): # merges Data for exactly two files (will be removed later)
    inputfile1 = open(trajec1, "r")
    lines1 = inputfile1.readlines()
    inputfile2 = open(trajec2, "r")
    lines2 = inputfile2.readlines()
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


def addDatas(trajecs, output):  # will merge all data into the output-file
    lines = [[], []]        # lines[i][j] | i - index of file | j - row number
    index = 0
    while (index < len(trajecs)):
        temptrajec = open(trajecs[index], "r")
        templine = temptrajec.readlines()
        lines += []
        lines[index] += templine
        temptrajec.close()
        index = index + 1

    head_end = 0
    for line in lines[0]:
        if line.startswith("#") or line == "\n":
            head_end += 1
        else: # now the trajectories start
            break
    counts = []
    output_file = open(output, 'w')
    index = 0
    while index < len(trajecs): # creates as many counters as files starting from row where trajec starts
        counts += [head_end]
        index = index + 1

    while len(lines) > 0:
        found = False
        currentframes = []
        index = 0
        while index < len(lines):   # searches next frame
            currentframes += [FramefromLine(lines[index][(counts[index])])]
            index = index + 1
        nextframe = min(currentframes)
        print(f"der aktuelle Frame : {nextframe} verbleibende Dateien: {len(lines)}")
        index = 0
        while index < len(lines): # adds row with next frame
            if FramefromLine(lines[index][(counts[index])]) == nextframe and found is False:
                output_file.write(lines[index][counts[index]])
                counts[index] += 1
                found = True
                if ((counts[index] >= len(lines[index]))): # removes file and counter from lines if end has been reached
                    counts.pop(index)
                    lines.pop(index)
            index = index + 1
    output_file.close()
    print(f"neue Datei {output} wurde erstellt.")

addDatas(args.merge, args.output)

