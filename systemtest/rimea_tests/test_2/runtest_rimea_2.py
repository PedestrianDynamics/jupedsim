#!/usr/bin/env python3
"""
Test Description
================
One pedestrian is moving along a 10m long stairway upstairs (x-lenght 8m and
z-height 6m).

The test shows wether the pedestrian can maintain its speed constant or not.

Remarks:
========
Use this code with python 2
Use new dedicated python console if you run this code with spyder

In JuPedSim pedestrians adapt their velocity from a corridor to a stair
smoothly (no step function).
In this test we set v0 = v0_upstairs. Hence, we can avoid any delay related to
the transition from v0 to v0_upstairs.

See Fig. stairs.png

Source:
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *
import time

def run_rimea_test2(inifile, trajfile):
    # Geometry data
    start_stair = 0.0
    end_stair = 8.0
    # Min. and max. traveltime of the pedestrian (Calculated)
    v0_upstairs = 0.5 # See master_ini
    must_min_time = 0.95*(end_stair - start_stair)/v0_upstairs # mean_time - 5% * mean_time
    must_max_time = 1.05*(end_stair - start_stair)/v0_upstairs # mean_time + 5% * mean_time
    # Read data
    fps, n, traj = parse_file(trajfile)
    # Pedestrian starting at x = -1, only get traveltime between x = 0 and x = 8
    in_stair = (traj[:, 2] >= start_stair) & (traj[:, 2] <= end_stair)
    traj_stair = traj[in_stair]
    evac_time = (max(traj_stair[:, 1]) - min(traj_stair[:, 1])) / float(fps)
    # Check wether the simulation fits into the wanted range of time 
    if must_min_time <= evac_time <= must_max_time:
        logging.info("evac_time: %f <= %f <= %f", must_min_time, evac_time, must_max_time)
    else:
        logging.critical("%s exits with FAILURE. evac_time: %f <= %f <= %f ?",
                         argv[0], must_min_time, evac_time, must_max_time)
        exit(FAILURE)

if __name__ == "__main__":
    start_time = time.time()
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=run_rimea_test2)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)