#!/usr/bin/env python3

"""
- Paper: Understanding human queuing behaviour at exits: an empirical study
         (http://rsos.royalsocietypublishing.org/content/4/1/160896)
- Name: Experiment B
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

def runtest5(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:,0]).astype(int)
    e1 = [0.1, 4.0, 4.8] # x, y1, y2 LEFT
    e2 = [8.7, 4.0, 4.8] # x, y1, y2 RIGHT

    e3 = [0.1, 4.0, 4.8] # y, x1, x2 DOWN
    e4 = [8.7, 4.0, 4.8] # y, x1, x2 UP 

    count1 = 0
    count2 = 0
    count3 = 0
    count4 = 0
    N = len(ids)
    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if PassedLineX(traj1, e1):
            count1 += 1
        elif PassedLineX(traj1, e2):
            count2 += 1
        elif PassedLineY(traj1, e3):
            count3 += 1
        elif PassedLineY(traj1, e4):
            count4 += 1            
        else:
            logging.critical("ped %d did not pass one of the exists" % ped)
            failure = True

    logging.info(">> STATISTICS: LEFT: %d. RIGHT: %d, DOWN: %d, UP: %d | Total %d <<" % (count1, count2, count3, count4, N))
    
    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(5, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest5)
    logging.info("%s exits with SUCCESS." % (argv[0]))
    exit(SUCCESS)
