#!/usr/bin/env python
import os
import sys
from sys import *
sys.path.append(os.path.abspath(os.path.dirname(sys.path[0])))
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest6(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    evac_time = ( max( traj[:,1] ) - min( traj[:,1] ) ) / float(fps)
    must_time = 10
    tolerance = 0.1
    if abs((evac_time - must_time)) > tolerance:
        logging.info("%s exits with FAILURE evac_time = %f (!= %f)"%(argv[0], evac_time, must_time))
        exit(FAILURE)
    else:
        logging.info("OK evac_time = %f (!= %f)"%( evac_time, must_time))


if __name__ == "__main__":
    test = JPSRunTestDriver(6, argv0=argv[0], testdir=sys.path[0])
    test.run_test(testfunction=runtest6)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)