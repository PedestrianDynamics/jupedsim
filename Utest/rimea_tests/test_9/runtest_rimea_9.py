#!/usr/bin/env python3
"""
Test description
================
1000 pedestrians are distributed in a room of 30m lenght and 20m width. Each of the longer walls of the room
has two exits of 1m width. There are two scenarios calculated:

Scenario 1: All four exits are open
Scenario 2: The exits of one of the longer walls are closed. The remaining two exits remain open.

The test shows, that the evacuation time of scenario 1 is about the half the evacuation time in scenario 2.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

For simplicity we simulate two identical rooms:
Room left with 4 exits. All of them are open.
Room right with 4 exits. Two of them are closed.

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""

import os
import sys
from sys import *
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

def run_rimea_test9(inifile, trajfile):
    fps, numpeds, traj = parse_file(trajfile)
    
    logging.info("Npeds_4door: %d, Npeds_2door: %d fps: %d", numpeds/2, numpeds/2 , fps)
    peds = np.unique(traj[:, 0])
    
    peds_4door = peds[peds <= numpeds/2] # Pedestrians in room with 4 doors open
    peds_2door = peds[peds > numpeds/2] # Pedestrians in room with 2 doors open
    
    evac_times4door = []
    evac_times2door = []
    
    for ped in peds_4door:
        # Data of each ped
        ptraj = traj[traj[:, 0] == ped]
        # Get evac time of each ped
        evac_times4door.append(ptraj[-1,1]/8)

    for ped in peds_2door:
        # Data of each ped
        ptraj = traj[traj[:, 0] == ped]
        # Get evac time of each ped
        evac_times2door.append(ptraj[-1,1]/8)
    
    evac_time_4door = np.max(evac_times4door)
    evac_time_2door = np.max(evac_times2door)
    
    logging.info("Evac_time_4door: %f, Evac_time_2door: %f", evac_time_4door, evac_time_2door)
    
    factor = evac_time_2door/evac_time_4door
    
    if 1.6 <= factor <= 2.4:
        logging.info("Evac_time_2door diveded by Evac_time_4door is in between:")
        logging.info("2 - 0.2 <= %f <= 2 + 0.2", factor)
    else:
        logging.info("Evac_time_2door diveded by Evac_time_4door is not in between:")
        logging.info("2 - 0.2 <= %f <= 2 + 0.2", factor)
        logging.critical("%s exists with FAILURE.", argv[0])
        exit(FAILURE)

if __name__ == "__main__": 
    test = JPSRunTestDriver(9, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test9)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)










