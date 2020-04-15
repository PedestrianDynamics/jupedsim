#!/usr/bin/env python3

"""
Simple corridor with two exits, exit closer to pedestrian is closed. Check if pedestrian use further exit.
"""

import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *

sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *


def runtest1(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:, 0]).astype(int)
    e2 = [10, -5, 5]  # x, y1, y2
    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        if not PassedLineX(traj1, e2):
            logging.critical("ped %d did not pass exit" % ped)
            failure = True

    if failure:
        logging.critical("%s exists with failure!" % argv[0])
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest1)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
