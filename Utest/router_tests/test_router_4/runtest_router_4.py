#!/usr/bin/env python3

"""
- Paper: Understanding human queuing behaviour at exits: an empirical study
         (http://rsos.royalsocietypublishing.org/content/4/1/160896)
- Name: Basigo 1
- Unit: Meter
- TODO: compare with the results form the paper
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

def runtest4(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:,0]).astype(int)
    e1 = [2.9, 2.8, 3.5] # y, x1, x2
    e2 = [2.9, 6.5, 7.6] # y, x1, x2
    count1 = 0
    count2 = 0
    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if PassedLineY(traj1, e1):
            count1 += 1
        elif PassedLineY(traj1, e2):
            count2 += 1
        else:
            logging.critical("ped %d did not pass one of the exists" % ped)
            failure = True

    logging.info("\t >> STATISTICS: Exit Left: %d. Exit Right: %d" % (argv[0], count1, count2))
    
    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest4)
    logging.info("%s exits with SUCCESS." % (argv[0]))
    exit(SUCCESS)
