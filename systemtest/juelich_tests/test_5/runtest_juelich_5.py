#!/usr/bin/env python3
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest5(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    traj_1 = traj[traj[:, 0] == 1] # behind
    x_1 = traj_1[:, 2]
    y_1 = traj_1[:, 3]

    traj_2 = traj[traj[:, 0] == 2] # vorne
    x_2 = traj_2[:, 2]
    y_2 = traj_2[:, 3]

    eps = 0.15 # see master_ini.xml
    x_min = x_2[0] - eps
    x_max = x_2[0] + eps
    y_min = y_2[0] - eps
    y_max = y_2[0] + eps

    n = x_2.shape[0]

    print(n)

    lx = np.logical_and(x_1[:n] > x_2 - eps, x_1[:n] < x_2 + eps)
    ly = np.logical_and(y_1[:n] > y_2 - eps, y_1[:n] < y_2 + eps)

    overlap = (lx*ly).any()
    if overlap:
        logging.info("%s exits with FAILURE " % argv[0])
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(5, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest5)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
