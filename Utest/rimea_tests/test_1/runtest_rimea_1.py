#!/usr/bin/env python3
"""
Test description
================
One pedestrian is moving along a 40m long corridor. The geometry is built as a
42m long corridor with the x-axis starting at -2 and ending at 40. The corridor
has a width of 2m.
The pedestrian is starting at x=-1 and y = 1, because the used force-based model
needs space to accelerate to its constant speed v_0.

The test shows wether the pedestrian can maintain its speed constant or not.

Remarks
=======
Use new dedicated python console if you run this code with spyder

Source
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



def run_rimea_test1(inifile, trajfile):
    # Geometry data
    start_way = 0.0
    end_way = 40.0
    # Min. and max. traveltime of the pedestrian (Rimea)
    must_min_time = 26.0
    must_max_time = 34.0
    # Pedestrians in force-based models accelerate from 0 to v0 after some time (\tau)
    fps, n, traj = parse_file(trajfile)
    # Pedestrian starting at x = -1, only get traveltime between x = 0 and x = 40
    in_way = (traj[:, 2] >= start_way) & (traj[:, 2] <= end_way)
    traj_way = traj[in_way]
    evac_time = (max(traj_way[:, 1]) - min(traj_way[:, 1])) / float(fps)
    # Check wether the simulation fits into the wanted range of time 
    if must_min_time <= evac_time <= must_max_time:
        logging.info("evac_time: %f <= %f <= %f", must_min_time, evac_time, must_max_time)
    else:
        logging.info("%s exits with FAILURE. evac_time: %f <= %f <= %f ?",
                     argv[0], must_min_time, evac_time, must_max_time)
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test1)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)







