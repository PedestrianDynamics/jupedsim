#!/usr/bin/env python3

"""
- Paper: Avoid or Follow? modelling Route Choice Based on Experimental Empirical Evidences 
        by Luca Crociani et al. PED16 in Hefei
- Name: Cruciani2016
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

def runtest7(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    ids = np.unique(traj[:,0]).astype(int)
    e1 = [8.2, 7.82, 8.82] # x, y1, y2
    e2 = [8.2, 5.82, 6.82] # x, y1, y2
    e3 = [8.2, 2.82, 3.82] # x, y1, y2
    count1 = 0
    count2 = 0
    count3 = 0
    N = len(ids)
    for ped in ids:
        traj1 = traj[traj[:, 0] == ped]
        x = traj1[:, 2]
        y = traj1[:, 3]
        if PassedLineX(traj1, e1) != 0:
            count1 += 1
            print("ped %d" % ped)
            print(min(x), max(x))
            print(min(y), max(y))
            np.savetxt('x.txt', x)
            np.savetxt('y.txt', y) 
            input()
        elif PassedLineX(traj1, e2) != 0:
            count2 += 1
        elif PassedLineX(traj1, e3) != 0:
            count3 += 1
        else:
            logging.critical("ped %d did not pass one of the exits" % ped)
            failure = True

    logging.info(">> STATISTICS: UP: %d. MIDDLE: %d, DOWN: %d, Total %d <<" % (count1, count2, count3, N))
    
    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)



if __name__ == "__main__":
    test = JPSRunTestDriver(7, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest7)
    logging.info("%s exits with SUCCESS." % (argv[0]))
    exit(SUCCESS)
