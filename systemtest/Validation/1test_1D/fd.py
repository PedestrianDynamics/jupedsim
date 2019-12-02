"""
measure the fundamental diagram in a 1d scenario.
return density, velocity
"""
import glob
import numpy as np
import os
import sys
from sys import *

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
path.append(utestdir)
from utils import *

# ======================= Measurement area ===================
left = 11
right = 13
down = -0.3
up = 0.3
# ======================== some dirs and files ==================
CWD = os.getcwd()
sim_dir = os.path.join(CWD, "Output", "Fundamental_Diagram", "Individual_FD")
exp_dir = "data"
trajfiles = glob.glob("trajectories/*.xml")
# ===============================================================
density = []
velocity = []
for trajfile in trajfiles:
    fps, n, traj = parse_file(trajfile)
    print "n: ", n
    frames = np.unique(traj[:, 1]).astype(int)
    for frame in frames:
        d = traj[traj[:, 1] == frame]
        print d
        inside = (d[:, 2] > left) & \
                    (d[:, 2] < right) & \
                    (d[:, 3] > down) & \
                    (d[:, 3] < up)
        peds_inside = d[inside]
        # for ped in peds_inside:
        #     density.append(get_density(d, ped))
        #     velocity.append(get_velocity(d, ped, fps))
            
        # raw_input()


def get_velocity(data, ped, left, right, fps):
    p = data[ data[:,0] == ped ] # matrix for pedestrian <ped>
    fin = min( p[ p[:,2] >= left ][:,1] ) # todo: check if empty
    fout = max( p[ p[:,2] <= right ][:,1] ) # todo: check if empty
    tin = fin / fps # get time in
    tout = fout / fps # get time out
    v = (right - left) / ( tout - tin )
    return v

def get_density(data, ped):
    front = {}
    behind = {}
    for other in np.unique(data[:, 0]).astype(int):
        if other == ped:
            continue

        dist = data[data[:, 0] == other][0, 2]
        
