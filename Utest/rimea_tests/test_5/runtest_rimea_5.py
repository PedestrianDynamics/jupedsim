#!/usr/bin/env python3
"""
Test description
================
Distribute 10 pedestrians with 10 different reaction times.

Check whether they start exactly at the specified times.

Remarks
=======
Use new dedicated python console if you run this code with spyder

An error of 1/fps is to be considered.

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdf
"""

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
    errors = []
    # pedestrians in force-based models accelerate from 0 to v0 after some time (\tau)
    fps, n, traj = parse_file(trajfile)

    threshold = 2./fps + 0.01  # consider reaction time

    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, fps: %d, threshold = %.2f", n, fps, threshold)
    for ped in peds:
        must_dt += reaction_time
        ptraj = traj[traj[:, 0] == ped]
        xdiff = np.diff(ptraj[:, 2]) # dx
        ydiff = np.diff(ptraj[:, 3]) # dy

        if (xdiff != 0).any():
            xfr = ptraj[xdiff != 0][1, 1] # second x element, not first
        else:
            xfr = np.inf

        if (ydiff != 0).any():
            yfr = ptraj[ydiff != 0][1, 1] # second x element, not first
        else:
            yfr = np.inf

        if np.isinf(yfr) and np.isinf(xfr): # ped did not move at all
            logging.critical("xfr: %d, yfr: %d", xfr, yfr)
            logging.critical("%s exits with FAILURE", argv[0])
            exit(FAILURE)


        df = min(xfr, yfr) - ptraj[0, 1]
        dt = 1.0*df/fps
        error = np.abs(dt-must_dt)
        errors.append(error)
        logging.info("ped: %2d, df: %6d, dt: %6.2f, must_time: %3d (error: %2.2f )",
                     ped, df, dt, must_dt, error)
    if not (np.array(errors) <= threshold).all():
        logging.critical("%s exits with FAILURE", argv[0])
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(5, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test5)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)



