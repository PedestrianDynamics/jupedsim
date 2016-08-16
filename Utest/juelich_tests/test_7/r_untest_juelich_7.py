#!/usr/bin/env python3

"""
- 6 peds distributed in a room with 2 exits:
  - Group 1, peds: 1, 2, 3 with router "global_shortest"
  - Group 2, peds: 4, 5, 6 with router "quickest"

- Group 1 should leave from the exit e1
- Group 2 should leave from exit e2
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest7(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    group_1 = [1, 2, 3]
    group_2 = [4, 6, 5]
    e1 = [26, 1, 2] # y, x1, x2
    e2 = [18, 1, 2] # x, y1, y2
    for ped in group_1:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if not PassedLineY(traj1, e1):
            logging.critical("ped %d did not exit from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%
                             (ped, e1[1], e1[0], e1[2], e1[0]))
            failure = True
        else:
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%
                         (ped, e1[1], e1[0], e1[2], e1[0]))

    for ped in group_2:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if not PassedLineX(traj1, e2):
            logging.critical("ped %d did not exit from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%
                             (ped, e2[0], e2[1], e2[0], e2[2]))
            failure = True
        else:
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%
                         (ped, e2[0], e2[1], e2[0], e2[2]))

    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)




if __name__ == "__main__":
    test = JPSRunTestDriver(7, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest7)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
