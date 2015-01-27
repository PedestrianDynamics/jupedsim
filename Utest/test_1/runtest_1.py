#!/usr/bin/env python
import os
import sys
from sys import *
sys.path.append("../")
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest1(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    must_time = 10
    fps, n, traj = parse_file(trajfile)
    evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
    tolerance = 0.01
    if (abs(evac_time - must_time)) > tolerance:
        logging.info("%s exits with FAILURE evac_time = %f (!= %f)" % (argv[0], evac_time, must_time))
        exit(FAILURE)
    else:
        logging.info("evac_time = %f (!= %f)" % (evac_time, must_time))



if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=os.path.abspath("."))
    test.run_test(testfunction=runtest1)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)

