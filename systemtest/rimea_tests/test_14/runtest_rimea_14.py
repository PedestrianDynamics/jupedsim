#!/usr/bin/env python3
"""
Test description
================
The pedestrians can choose between to options leaving the building. First there
is a short way out by using two sets of stairs. Then there is a longer way out
leading around two corners.

The test shows wether the pedestrians prefer the longer way out while the shorter
way is crowded.

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

states = ["short", "long", "mixed"]

def run_rimea_test14(inifile, trajfile):
    # Read data
    if os.path.isfile('flow_exit_id_1_traj.txt'):
        data_short = np.loadtxt('flow_exit_id_4_traj.txt')
        data_main = np.loadtxt('flow_exit_id_1_traj.txt')

    elif os.path.isfile('trajectories/traj.xml_flow_exit_id_1.dat'):
        data_short = np.loadtxt('trajectories/traj.xml_flow_exit_id_4.dat')
        data_main = np.loadtxt('trajectories/traj.xml_flow_exit_id_1.dat')

    else:
        print('Can not find Exit Files')

    # Cummulative number of peds using the right stair --> short way
    num_short = data_short[-1,-1]

    # Cummulative number of peds using the exit
    # Minus
    # Cummulative number of peds using the right stair --> long way
    num_long = data_main[-1,-1] - data_short[-1,-1]

    if num_long == 0 and num_short != 0:
        state = states[0]
    elif num_long != 0 and num_short == 0:
        state = states[1]
    else:
        state = states[2]

    logging.info("num_peds: %d, num_long: %d, num_short %d", data_main[-1,-1], num_long, num_short)
    logging.info("Return state ---> %s", state)

if __name__ == "__main__":
    start_time=time.time()
    test = JPSRunTestDriver(14, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test14)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)
