#!/usr/bin/env python3
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'

def runtest12(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    fps, N, traj = parse_file(trajfile)
    y2 = traj[traj[:, 0] == 2][:, 3]
    dy2 = np.sum(np.abs(np.diff(y2)))
    # evac_time = ( max( traj[:,1] ) - min( traj[:,1] ) ) / float(fps)
    tolerance = 0.005
    if dy2 > tolerance:
        logging.info("%s exits with FAILURE dy2 = %f (tolerance=%f)", argv[0], dy2, tolerance)
        exit(FAILURE)
    else:
        logging.info("dy2 = %f (tolerance=%f)", dy2, tolerance)

if __name__ == "__main__":
    test = JPSRunTestDriver(12, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest12)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)

