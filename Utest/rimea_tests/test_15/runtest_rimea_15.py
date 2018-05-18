#!/usr/bin/env python
"""
Test description
================
In this test there are 3 rooms:
- Long corridor
- Short corridor
- Corner

The test shows the influence of the corner on the pedestrian movement. The
evacuation time of the corner should be in the middle of the long and short
corridor.

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

def run_rimea_test15(inifile, trajfile):
    
    # Read data
    data_long = np.loadtxt('trajectories/traj.xml_flow_exit_id_0.dat')
    data_short = np.loadtxt('trajectories/traj.xml_flow_exit_id_1.dat')
    data_corner = np.loadtxt('trajectories/traj.xml_flow_exit_id_2.dat')
    
    # Evac times
    evac_long = data_long[-1,0]
    evac_short = data_short[-1,0]
    evac_corner = data_corner[-1,0]
    
    if evac_long < evac_corner or evac_short > evac_corner:
        logging.info("%s exits with FAILURE" % (argv[0]))
        logging.info("Corner evac_time not in between long and short corridor evac_time")
        logging.info("Long corridor evac_time: %f", evac_long)
        logging.info("Short corridor evac_time: %f", evac_short)
        logging.info("Corner evac_time: %f", evac_corner)
        exit(FAILURE)
    
    logging.info("Long corridor evac_time: %f", evac_long)
    logging.info("Short corridor evac_time: %f", evac_short)
    logging.info("Corner evac_time: %f", evac_corner)

if __name__ == "__main__":
    test = JPSRunTestDriver(15, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test15)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
