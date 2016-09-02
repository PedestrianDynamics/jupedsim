#!/usr/bin/env python3
"""
Test description
================
Pedestrian evacuation to an exit. They have two possible routes: 
1. short
2. and long

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

This test has no concrete condition to check for.
It should be documented whether pedestrians take a long detour or not
There are 4 stats that should be documented:
1. "kurz" (short)
2. "lang"  (long)
3. "gemischt" (mixed)
4. "konfigurierbar" (configurable)

Source
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

states = ["short", "long", "mixed", "configurable"]
stair_up_left = [5, 4, 6] # x, y1, y2 |--> exit to the stair left. 
# todo: read  stair_up_left from geometry

def run_rimea_test14(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    num_short = 0
    num_long = 0
    peds = np.unique(traj[:, 0])
    for ped in peds:
        traj1 = traj[traj[:, 0] == ped]
        if PassedLineX(traj1, stair_up_left):
            num_short += 1
        else:
            num_long += 1

    if num_long == 0 and num_short != 0:
        state = states[0]
    elif num_long != 0 and num_short == 0:
        state = states[1]
    else:
        state = states[2]

    logging.info("num_peds: %d, num_long: %d, num_short %d",
                 len(peds), num_long, num_short)
    logging.info("Return state ---> %s", state)

if __name__ == "__main__":
    test = JPSRunTestDriver(14, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test14)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)









