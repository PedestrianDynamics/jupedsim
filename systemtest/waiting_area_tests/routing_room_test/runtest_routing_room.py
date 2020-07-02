#!/usr/bin/env python3

"""
Test if ped walks through waiting area, even if the other way is faster
"""

import os
import sys
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *

sys.path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file

from JPSRunTest import JPSRunTestDriver
from utils import *
from wa_utils import *


def runtest(inifile, trajfile):
    success = True
    fps, N, traj = parse_file(trajfile)
    waitingAreas = parse_waiting_areas(inifile)

    trajPedDirect = traj[np.where(traj[:, 0] == 1)]
    trajPedWaitingArea = traj[np.where(traj[:, 0] == 2)]

    # ped on direct way should have walked through any waiting area
    pedDirectInWA = False
    for waitingArea in waitingAreas:
        pedDirectInWA = pedDirectInWA or PassedPolygon(trajPedDirect, waitingArea.boundingBox[0],
                                                       waitingArea.boundingBox[1], waitingArea.boundingBox[2],
                                                       waitingArea.boundingBox[3])
    if pedDirectInWA:
        logging.critical("Ped on direct way walked through waiting area")
        success = False

    # ped through waiting areas should have walked through all waiting areas
    pedWAInWA = True
    for waitingArea in waitingAreas:
        pedWAInWA = pedWAInWA and PassedPolygon(trajPedWaitingArea, waitingArea.boundingBox[0],
                                                waitingArea.boundingBox[1], waitingArea.boundingBox[2],
                                                waitingArea.boundingBox[3])
    if not pedWAInWA:
        logging.critical("Ped on way through waiting areas did miss at least one.")
        success = False

    if not success:
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
