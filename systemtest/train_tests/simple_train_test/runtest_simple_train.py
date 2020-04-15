#!/usr/bin/env python3

"""
Simple test if the simulation time till the platform is empty does not change (reference value 44s with
master (c0931492a18a52aa925b84091d77f42731ca5894)).
"""

import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *

sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *


def runtest(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)

    maxFrame = np.max(traj[:, 1])
    time = maxFrame / fps

    if time > 44:
        logging.critical("%s exists with failure!" % argv[0])
        logging.critical("simulation took longer than 44s")
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
