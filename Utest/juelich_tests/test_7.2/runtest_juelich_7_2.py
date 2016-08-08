#!/usr/bin/env python3

"""
- 100 peds distributed in a room with 2 exits:
  - All  peds  with router "quickest"

- Some peds should leave from the exit e2
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest72(inifile, trajfile):
    e1 = [26, 1, 2] # y, x1, x2
    e2 = [18, 1, 2] # x, y1, y2
    fps, N, traj = parse_file(trajfile)
    peds = np.unique(traj[:,0]).astype(int)
    logging.info("Checking the exits of pedestrians ...")

    for ped in peds:
        traj1 = traj[ traj[:,0] == ped ]
        x = traj1[:,2]
        y = traj1[:,3]
        if PassedLineX(traj1, e2):
            logging.info("ped %d exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))
            logging.info("%s exists with success!"%argv[0])
            exit(SUCCESS)


def PassedLineX(p, exit):
    """
    check if pedestrian (given by matrix p) passed the vertical line x, [y1, y2] y1<y2
    """
    x = exit[0]
    y1 = exit[1]
    y2 = exit[2]
    return any(p[:,2] <= x) & any(p[:,2] >= x) & any(p[:,3] >= y1) & any(p[:,3] <= y2)


if __name__ == "__main__":
    test = JPSRunTestDriver(7.2, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest72)
    logging.info("%s exists with failure!" % argv[0])
    exit(FAILURE)
