import numpy as np


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


test = ["src/traj_stairs_1copy.txt", "src/traj_stairs_2copy.txt"]
if checkhead(test):
    print("information match")
else:
    print("information don´t match")
if checkdata(test):
    print("data match")
else:
    print("data don´t match")
