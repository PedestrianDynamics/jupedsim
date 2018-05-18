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

The exits of the left room have the ids 0 to 3.
The exits of the right room have the ids 6 and 7.

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

    # Read data:
    left_room_0 = np.loadtxt('trajectories/traj.xml_flow_exit_id_0.dat')
    left_room_1 = np.loadtxt('trajectories/traj.xml_flow_exit_id_1.dat')
    left_room_2 = np.loadtxt('trajectories/traj.xml_flow_exit_id_2.dat')
    left_room_3 = np.loadtxt('trajectories/traj.xml_flow_exit_id_3.dat')
    
    right_room_0 = np.loadtxt('trajectories/traj.xml_flow_exit_id_6.dat')
    right_room_1 = np.loadtxt('trajectories/traj.xml_flow_exit_id_7.dat')
    
    # Evac times for each scenario:
    evac_time_left = np.max([left_room_0[-1,0], left_room_1[-1,0], left_room_2[-1,0], left_room_3[-1,0]])
    evac_time_right = np.max([right_room_0[-1,0], right_room_1[-1,0]])
    
    logging.info("Evac_time_4_door: %f, Evac_time_2_door: %f", evac_time_left, evac_time_right)
    
    factor = evac_time_right/evac_time_left
    
    if 1.6 <= factor <= 2.4:
        logging.info("Evac_time_2door divided by Evac_time_4door is in between:")
        logging.info("1.6 <= %f <= 2.4", factor)
    else:
        logging.info("Evac_time_2door divided by Evac_time_4door is not in between:")
        logging.info("1.6 <= %f <= 2.4", factor)
        logging.critical("%s exists with FAILURE.", argv[0])
        exit(FAILURE)

if __name__ == "__main__": 
    test = JPSRunTestDriver(9, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test9)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)










