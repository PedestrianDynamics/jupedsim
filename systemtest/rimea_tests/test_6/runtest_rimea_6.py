#!/usr/bin/env python3
"""
Test description
================
Distribute 20 pedestrians in a room with an exit behind a corner.

The test shows whether the pedestrians are going around the corner or walking
through walls.

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
import time

def run_rimea_test6(inifile, trajfile):
    # Coordinates of the square outside the room
    x_min = 0
    x_max = 10
    y_min = 2
    y_max = 12
    # Read data
    fps, n, traj = parse_file(trajfile)
    logging.info("=== npeds: %d, fps: %d ===", n, fps)
    # Forbidden geometry
    is_outside_geometry = ((traj[:, 2] >= x_min) & (traj[:, 2] <= x_max) &
                           (traj[:, 3] >= y_min) & (traj[:, 3] <= y_max))

    if is_outside_geometry.any():
        peds_outside = np.unique(traj[is_outside_geometry][:, 0])
        peds_outside_str = ", ".join(np.char.mod("%d", peds_outside))
        logging.critical("%s exits with FAILURE. %d pedestrians are outside %s ",
                         argv[0], peds_outside_str)
        exit(FAILURE)

if __name__ == "__main__":
    start_time = time.time()
    test = JPSRunTestDriver(6, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=run_rimea_test6)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)