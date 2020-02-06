#!/usr/bin/env python3

"""
- Paper: Understanding human queuing behaviour at exits: an empirical study
         (http://rsos.royalsocietypublishing.org/content/4/1/160896)
- Name: Experiment C, Part 1 (start from Holding area 1)
- Npeds = 67
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

def runtest6(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:,0]).astype(int)
    e1 = [0.1, 1.0, 1.8] # x, y1, y2
    e2 = [0.1, 4.0, 4.8] # x, y1, y2
    count1 = 0
    count2 = 0
    N = len(ids)
    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if PassedLineX(traj1, e1):
            count1 += 1
        elif PassedLineX(traj1, e2):
            count2 += 1
        else:
            logging.critical("ped %d did not pass one of the exists" % ped)
            failure = True

    logging.info(">> STATISTICS: Exit DOWN: %d. Exit UP: %d, Total %d <<" % (count1, count2, N))
    
    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(6, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest6)
    logging.info("%s exits with SUCCESS." % (argv[0]))
    exit(SUCCESS)
