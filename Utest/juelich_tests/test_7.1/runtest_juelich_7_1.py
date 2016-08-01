#!/usr/bin/env python3

"""
- 100 peds distributed in a room with 2 exits:
  - All  peds  with router "global_shortest"

- All peds should leave from the exit e1
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest71(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    peds = np.unique(traj[:,0]).astype(int)
    e1 = [26, 1, 2] # y, x1, x2
    e2 = [18, 1, 2] # x, y1, y2
    logging.info("Checking the exits of pedestrians ...")
    for ped in peds:
        traj1 = traj[ traj[:,0] == ped ]
        x = traj1[:,2]
        y = traj1[:,3]
        if PassedLineX(traj1, e2):
            logging.critical("ped %d exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))
            failure = True

    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)


def PassedLineX(p, exit):
    """
    check if pedestrian (given by matrix p) passed the vertical line x, [y1, y2] y1<y2
    """
    x = exit[0]
    y1 = exit[1]
    y2 = exit[2]
    return any(p[:,2] <= x) & any(p[:,2] >= x) & any(p[:,3] >= y1) & any(p[:,3] <= y2)


if __name__ == "__main__":
    test = JPSRunTestDriver(7.1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest71)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
