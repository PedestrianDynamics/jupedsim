#!/usr/bin/env python3
"""
Test description
================
Distribute 10 pedestrians in a room of 8m lenght and 5m width with an exit in
the middle of one of the shorter walls. All pedestrians do have a different
reaction time between 10 and 100s:

Ped 1 has 10s reaction time
Ped 2 has 20s reaction time
...
Ped 10 has 100s reaction time

The test shows whether the pedestrians start moving exactly at their specified
reaction times.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""

import numpy as np
import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

def run_rimea_test5(inifile, trajfile):
    # Reaction times are: 10, 20, ..., 100
    delta_time = 10
    reaction_time = 0
    errors = []
    # Read data
    fps, n, traj = parse_file(trajfile)
    # Considered threshold
    threshold = 2.0/fps
    
    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, fps: %d, threshold = %.2f", n, fps, threshold)
    for ped in peds:
        # Reaction time of the ped i
        reaction_time += delta_time
        # Take only data of ped i
        ptraj = traj[traj[:, 0] == ped]
        
        # Differences of trajectory on x-axis saved in an array
        xdiff = np.diff(ptraj[:, 2]) # delta x
        # Differences of trajectory on y-axis saved in an array
        ydiff = np.diff(ptraj[:, 3]) # delta y
        
        # Frame when ped i starts moving along x-axis
        if (xdiff != 0).any():
            xfr = ptraj[xdiff != 0][0, 1]
        else:
            xfr = np.inf
        # Frame when ped i starts moving along y-axis
        if (ydiff != 0).any():
            yfr = ptraj[ydiff != 0][0, 1]
        else:
            yfr = np.inf
        # If ped i not moving at all, exit with failure
        if np.isinf(yfr) and np.isinf(xfr):
            logging.critical("xfr: %d, yfr: %d", xfr, yfr)
            logging.critical("%s exits with FAILURE", argv[0])
            exit(FAILURE)
        
        #Frame when ped i starts moving at all
        df = min(xfr, yfr)
        
        #Time when ped i starts moving at all
        dt = float(df)/fps
        error = np.abs(dt-reaction_time)
        errors.append(error)
        logging.info("ped: %2d, df: %6d, dt: %6.2f, must_time: %3d (error: %2.2f )",ped, df, dt, reaction_time, error)
    
    # If If pedetrian starts moving above threshold, exit with failure
    if not (np.array(errors) <= threshold).all():
        logging.critical("%s exits with FAILURE", argv[0])
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(5, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test5)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)