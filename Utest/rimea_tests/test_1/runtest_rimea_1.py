#!/usr/bin/env python3
"""
Test description
================
One pedestrian is moving along a corridor.
Test if pedestrian can maintain its speed constant

Remarks
=======

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdfs
"""

import os
import sys
#REMARK: Use new dedicated python console if you run this code with spyder
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test1(inifile, trajfile):
    must_min_time = 26
    must_max_time = 34
    # pedestrians in force-based models accelerate from 0 to v0 after some time (\tau)
    fps, n, traj = parse_file(trajfile)
    evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
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







