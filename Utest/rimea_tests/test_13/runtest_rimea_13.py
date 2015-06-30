#!/usr/bin/env python
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

tolerance = 0.05

def run_rimea_test13(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    exit_basement = 0
    start_stair = 12
    J_corridor = flow(fps, N, traj, exit_basement)
    J_stair = flow(fps, N, traj, start_stair)

    if abs(J_corridor-J_stair) < tolerance:
        logging.critical("%s exists with FAILURE. Flows are almost equal:", argv[0])
        logging.critical("J_corridor = %.2f, J_stair = %.2f (tolerance = %.2f)",
                         J_corridor, J_stair, tolerance)
        exit(FAILURE)
    else:
        logging.info("J_corridor = %.2f, J_stair = %.2f (tolerance = %.2f)",
                     J_corridor, J_stair, tolerance)

if __name__ == "__main__":
    test = JPSRunTestDriver(13, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test13)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)









