#!/usr/bin/env python
import os
import sys
import numpy as np
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test5(inifile, trajfile):
    reaction_time = 10 # reaction times are [10, 20, 30, ..., 100]
    must_dt = 0
    threshold = 0.5 # second
    errors = []
    # pedestrians in force-based models accelerate from 0 to v0 after some time (\tau)
    fps, n, traj = parse_file(trajfile)
    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, fps: %d", n, fps)
    for ped in peds:
        must_dt += reaction_time
        ptraj = traj[traj[:, 0] == ped]
        df = ptraj[np.diff(ptraj[:, 2]) != 0][1, 1] - ptraj[0, 1] + 1
        dt = 1.0*df/fps
        error = np.abs(dt-must_dt)
        errors.append(error)
        logging.info("ped: %2d, df: %4d, dt: %5.2f, must_time: %3d (error: %2.2f )",
                     ped, df, dt, must_dt, error)

    if not (np.array(errors) < threshold).all():
        logging.critical("%s exits with FAILURE. threshold = %.2f", argv[0], threshold)
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(5, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test5)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)





