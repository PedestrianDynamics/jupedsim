#!/usr/bin/env python3
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest4(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    traj_1 = traj[traj[:, 0] == 1]
    x_1 = traj_1[:, 2]
    y_1 = traj_1[:, 3]

    x_2 = traj[traj[:, 0] == 2][:, 2]
    y_2 = traj[traj[:, 0] == 2][:, 3]

    eps = 0.3 # 10 cm
    x_min = x_2[0] - eps
    x_max = x_2[0] + eps
    y_min = y_2[0] - eps
    y_max = y_2[0] + eps

    lx = np.logical_and(x_1 > x_min, x_1 < x_max)
    ly = np.logical_and(y_1 > y_min, y_1 < y_max)

    overlap = (lx*ly).any()
    if overlap:
        logging.info("%s exits with FAILURE " % argv[0])
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest4)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
