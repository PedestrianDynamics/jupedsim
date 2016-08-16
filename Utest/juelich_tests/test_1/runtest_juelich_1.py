#!/usr/bin/env python3
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest1(inifile, trajfile):
    must_time = 10.0
    # pedestrians in force-based models accelerate from 0 to v0 after some time (\tau)
    fps, n, traj = parse_file(trajfile)
    evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
    tolerance = 0.5
    if abs(evac_time - must_time) > tolerance:
        logging.info("%s exits with FAILURE evac_time = %f (!= %f)", argv[0], evac_time, must_time)
        print("%s exits with FAILURE evac_time = %f (!= %f)"%( argv[0], evac_time, must_time))
        exit(FAILURE)
    else:
        logging.info("evac_time = %f (!= %f)", evac_time, must_time)



if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest1)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)



