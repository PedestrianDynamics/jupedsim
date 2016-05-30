#!/usr/bin/env python3
"""
Test description
================
1000 pedestrians are distributed in a room with 4 exits.
scenario 1: All 4 exits are open
scenario 2: 2 exits are closed. The remaining 2 are still open

The flow should nearly be doubled in scenario 1.

Remarks
=======
Use new dedicated python console if you run this code with spyder

For simplicity we simulate two identical rooms:
1. room left: with 4 exits. All of them are open
2. room right with 4 exits. two of them are closed
3. We write the trajectory in plain txt-format, to avoid a long lasting xml-parsing
Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdf
"""

import os
import sys
import matplotlib.pyplot as plt
from sys import *

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))

sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

#  ==================== todo: read these variables from the geometry.xml
r1_left = 0
r1_right = 30

r2_left = r1_left + 31
r2_right = r1_right + 31

down = 0
up = 20

tolerance = 0.5
# =========================================================================


def eval_results(results):
    results = np.array(results)
    mean_r1 = np.mean(results[:, 0])
    mean_r2 = np.mean(results[:, 1])
    std_r1 = np.std(results[:, 0])
    std_r2 = np.std(results[:, 1])
    logging.info("mean left: %.2f (+- %.2f), mean right: %.2f (+- %.2f)",
                 mean_r1, std_r1, mean_r2, std_r2)
    plt.plot(results[:, 0], "o-", lw=2, label="Room left")
    plt.plot(results[:, 1], "o-", lw=2, label="Room right")
    plt.ylabel("evac time")
    plt.xlabel("# runs")
    plt.legend(loc="best")
    plt.savefig("evac_times.png")
    rel_error = np.abs(mean_r2 - 2*mean_r1)/(2*mean_r1)
    if rel_error > tolerance:
        logging.critical("%s exists with FAILURE. rel_error = %.2f, tolerance = %.2f",
                         argv[0], rel_error, tolerance)
        exit(FAILURE)

def run_rimea_test9(inifile, trajfile):
    max_frame_r1 = []
    max_frame_r2 = []
    #fps, numpeds, traj = parse_file(trajfile)
    fps = 8
    traj = np.loadtxt(trajfile)
    numpeds = np.max(traj[:, 0])
    max_frame = np.max(traj[:, 1])
    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, max_frame: %d, fps: %d", numpeds, max_frame, fps)
    for ped in peds:
        ptraj = traj[traj[:, 0] == ped]
        if is_inside(ptraj, r1_left, r1_right, down, up):
            max_frame_r1.append(np.max(ptraj[:, 1]))
        elif is_inside(ptraj, r2_left, r2_right, down, up):
            max_frame_r2.append(np.max(ptraj[:, 1]))
        else:
            logging.warning("*** ped: %d is nowhere! ***", ped)

    time_r1 = np.max(max_frame_r1)/fps
    time_r2 = np.max(max_frame_r2)/fps

    logging.info("max time room left: %.2f, max time room right: %.2f", time_r1, time_r2)
    return (time_r1, time_r2)

if __name__ == "__main__":

    test = JPSRunTestDriver(9, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    results = test.run_test(testfunction=run_rimea_test9)
    eval_results(results)

    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)










