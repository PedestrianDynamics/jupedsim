#!/usr/bin/env python3
"""
Test description
================
Distribute 23 pedestrians in 12 different rooms.
The building  has two exits.
The Pedestrians have exactly assigned exits and should evacuate through these.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

We assign pedestrian to two different groups.
In the simulation pedestrians disapear once they are outside therefore we check
if peds go through line  <exit> - <displacement>

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

displacement = 1

def run_rimea_test10(inifile, trajfile):
    # Read data
    fps, N, traj = parse_file(trajfile)
    failure = False
    # Ped IDs who use goal 0
    group_1 = [1, 2, 3, 4, 5, 6, 7, 12, 13, 14, 15, 16, 17, 18, 19]
    # Ped IDs who use goal 1
    group_2 = [8, 9, 10, 11, 20, 21, 22, 23]
    # Line goal 0
    e1 = [11-displacement, 6, 7.2]
    # Line goal 1
    e2 = [18-displacement, 5, 6]
    for ped in group_1:
        traj1 = traj[traj[:, 0] == ped]
        if not PassedLineY(traj1, e1):
            logging.critical("ped %d did not exit from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e1[1], e1[0], e1[2], e1[0]))
            failure = True
        else:
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e1[1], e1[0], e1[2], e1[0]))

    for ped in group_2:
        traj1 = traj[traj[:, 0] == ped]
        if not PassedLineX(traj1, e2):
            logging.critical("ped %d did not exit from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))
            failure = True
        else:
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))

    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(10, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test10)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)










