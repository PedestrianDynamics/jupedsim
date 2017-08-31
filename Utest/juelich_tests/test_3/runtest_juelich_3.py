#!/usr/bin/env python3
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest3(inifile, trajfile):
    #       x     y1   y2
    #       |     |    |
    #       v     v    v
    door= [8.5, 2.0, 3.0] # value from geometry file. Door is at 9.0 m shifted by 0.5 m
    maxtime = get_maxtime(inifile)
    fps, N, traj = parse_file(trajfile)
    evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
    if evac_time > maxtime*0.5 or not PassedLineX(traj, door):
        logging.info("%s exits with FAILURE. evac_time = %f (maxtime =  %f) or not passed exit" % (argv[0], evac_time, maxtime))
        exit(FAILURE)
    else:
        logging.info("evac_time = %f (maxtime =  %f)" % (evac_time, maxtime))


if __name__ == "__main__":
    test = JPSRunTestDriver(3, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest3)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
