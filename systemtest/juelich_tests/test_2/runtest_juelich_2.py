#!/usr/bin/env python3
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest2(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    v = 1.0  # this is the desired speed. Check in the master inifile
    fps, n, traj = parse_file(trajfile)
    evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
    # print fps
    # print max(traj[:, 1])
    # print min(traj[:, 1])
    distance_x = (max(traj[:, 2]) - min(traj[:, 2]))
    distance_y = (max(traj[:, 3]) - min(traj[:, 3]))
    distance = np.sqrt(distance_x**2 + distance_y**2)
    must_time = distance/v
    tolerance = 0.1
    if abs(evac_time- must_time) > tolerance:
        logging.info("%s exits with FAILURE evac_time = %f (!= %f)"%(argv[0], evac_time, must_time))
        print("%s exits with FAILURE evac_time = %f (!= %f)"%(argv[0], evac_time, must_time))
        exit(FAILURE)
    else:
        logging.info("evac_time = %f (!= %f)"%(evac_time, must_time))



if __name__ == "__main__":
    logging.error("{}".format(argv))
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest2)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
