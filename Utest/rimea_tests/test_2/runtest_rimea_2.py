#!/usr/bin/env python
"""
Test Description
================
1 ped moving on a 10m long stair.
It should be shown that the ped can maintain its speed constant.

Remarks:
========
In JuPedSim pedestrians adapt their velocity
from a corridor to a stair *smoothly* (no step function).
In this test we set v0 == v0Upstairs. Hence, we can avoid any delay related to the transition
from v0 to v0Upstairs.

Source:
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdfs
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test2(inifile, trajfile):
    tolerance = 1     # ped accelerates from 0 to v0 after some time (\tau)
    v0_upstairs = 0.5    # velocity on stair from master-ini.xml
    end_stair = 10.0     # values from geometry.xml
    start_stair = 0.0    # values from geometry.xml
    must_time = (end_stair - start_stair)/v0_upstairs
    fps, n, traj = parse_file(trajfile)
    # filter trajecetries. Consider only the stair
    # traj_stair = traj[0 <= traj[:, 2]]
    # traj_stair = traj_stair[traj_stair[:, 2] <= 10]
    in_stair = (traj[:, 2] >= start_stair) & (traj[:, 2] <= end_stair)
    traj_stair = traj[in_stair]
    evac_time = (max(traj_stair[:, 1]) - min(traj_stair[:, 1])) / float(fps)
    if must_time -tolerance <= evac_time <= must_time + tolerance:
        logging.info("evac_time: %f <= %f <= %f",
                     must_time-tolerance, evac_time, must_time+tolerance)
    else:
        logging.critical("%s exits with FAILURE. evac_time: %f <= %f <= %f ?",
                         argv[0], must_time-tolerance, evac_time, must_time+tolerance)
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test2)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)







