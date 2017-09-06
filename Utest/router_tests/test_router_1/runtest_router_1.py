#!/usr/bin/env python3

"""
- 6 peds distributed in a room with 2 exits:
- Test if all implemented router run. 
- This test does NOT check the behavior of the specific routers.

Actually the following routers are tested: 
- local_shortest
- global_shortest
- quickest
- ff_global_shortest
- ff_local_shortest
- ff_quickest
- AI

See also this issue: https://gitlab.version.fz-juelich.de/jupedsim/jpscore/issues/127
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest1(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:,0]).astype(int)
    group_2 = [4, 6, 5]
    e1 = [26, 1, 2] # y, x1, x2
    e2 = [18, 1, 2] # x, y1, y2
    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if not PassedLineY(traj1, e1) and not PassedLineX(traj1, e2):
            logging.critical("ped %d did not pass exit" % ped)
            failure = True

    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest1)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
