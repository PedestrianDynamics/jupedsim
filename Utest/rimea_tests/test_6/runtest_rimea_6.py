#!/usr/bin/env python3
"""
Test description
================
20 pedestrians going around a corner.

Remarks
=======
Use new dedicated python console if you run this code with spyder

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



def run_rimea_test6(inifile, trajfile):
    # coordinates of the forbiden square
    x_min = 0
    x_max = 10
    y_min = 2
    y_max = 12
    fps, n, traj = parse_file(trajfile)
    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, fps: %d ===", n, fps)
    is_outside_geometry = ((traj[:, 2] >= x_min) & (traj[:, 2] <= x_max) &
                           (traj[:, 3] >= y_min) & (traj[:, 3] <= y_max))

    if is_outside_geometry.any():
        peds_outside = np.unique(traj[is_outside_geometry][:, 0])
        peds_outside_str = ", ".join(np.char.mod("%d", peds_outside))
        logging.critical("%s exits with FAILURE. %d pedestrians are outside %s ",
                         argv[0], peds_outside_str)
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(6, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test6)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)







