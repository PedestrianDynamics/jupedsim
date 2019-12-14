#!/usr/bin/env python3
"""
Test description
================
150 pedestrians are distributed in a room which is connected to another room
via a thin corridor. In the second room there is an exit.

The test shows that there is a jam at the entry of the corridor.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

condition of this test is not clear enough...
In the last exit there should be no jam. Means:
J_botl >= J_last
However, this condition is not enough to quantify a jam.

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
import time

tolerance = 0.05
displacement = 1 # in the simulation pedestrians disapear once they are outside
# therefore we measure the last flow at a <last_exit> - <displacement>
def run_rimea_test12(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    exit_botl = [19.13, -0.55, 0.45]
    last_exit = [29.13 - displacement, -0.55, 0.45]
    J_botl = flow(fps, N, traj, exit_botl[0])
    J_last = flow(fps, N, traj, last_exit[0])

    if J_botl-J_last < -tolerance:
        logging.critical("%s exists with FAILURE. There is some jam in the last exit:", argv[0])
        logging.critical("J_botl = %.2f, J_last = %.2f (tolerance = %.2f)",
                         J_botl, J_last, tolerance)
        exit(FAILURE)
    else:
        logging.info("J_botl = %.2f, J_last = %.2f (tolerance = %.2f)", J_botl, J_last, tolerance)



if __name__ == "__main__":
    start_time=time.time()
    test = JPSRunTestDriver(12, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test12)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)











