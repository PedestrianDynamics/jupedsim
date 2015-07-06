#!/usr/bin/env python
"""
Test description
================
300 pedestrians are distributed in a room with two exits.
The pedestrians should prefer the nearest exit,
but some should (spontaneously) choose the second exit

Remarks
=======

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdf
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test11(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    exit_right = [20, 27, 28] # y, x1, x2
    peds = np.unique(traj[:, 0])
    total_peds = len(peds)
    passed_peds = 0
    for ped in peds:
        traj1 = traj[traj[:, 0] == ped]
        if  PassedLineY(traj1, exit_right):
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%
                         (ped, exit_right[1], exit_right[0], exit_right[2], exit_right[0]))
            passed_peds += 1


    if passed_peds == 0:
        logging.critical("%s exists with failure! 0 peds passed through right exit"%argv[0])
        exit(FAILURE)
    else:
        logging.info("[%d/%d] pedestrians passed through right exit!", passed_peds, total_peds)


if __name__ == "__main__":
    test = JPSRunTestDriver(11, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test11)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)








