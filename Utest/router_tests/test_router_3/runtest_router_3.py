#!/usr/bin/env python3

"""
180 degree turn around a wall

- Paper: Optimized Automatic Wayfinding for Pedestrian Simulations
- Length of the wall: 50 m
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest3(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:,0]).astype(int)
    e2 = [9, 16.5, 58] # x, y1, y2

    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if not PassedLineX(traj1, e2):
            logging.critical("ped %d did not pass exit" % ped)
            failure = True

    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(3, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest3)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
