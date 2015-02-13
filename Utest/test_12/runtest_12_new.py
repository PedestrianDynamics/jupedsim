#!/usr/bin/env python
import os
import sys
from sys import *
sys.path.append(os.path.abspath(os.path.dirname(sys.path[0])))
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'

def runtest12(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    fps, N, traj = parse_file(trajfile)
    y2 = traj[ traj[:,0] == 2 ][:,3]
    y4 = traj[ traj[:,0] == 4 ][:,3]
    dy2 = np.sum( np.abs( np.diff(y2) ) )
    dy4 = np.sum( np.abs( np.diff(y4) ) ) 

    # evac_time = ( max( traj[:,1] ) - min( traj[:,1] ) ) / float(fps)
    tolerance = 0.0001
    if dy2 > tolerance or dy4 >tolerance:
        logging.info("%s exits with FAILURE dy2 = %f,  dy4 = %f"%(argv[0], dy2, dy4))
        exit(FAILURE)
    else:
        logging.info("%s exits with SUCCESS dy2 = %f  dy4 = %f"%(argv[0], dy2, dy4))

if __name__ == "__main__":
    test = JPSRunTestDriver(12, argv0=argv[0], testdir=sys.path[0])
    test.run_test(testfunction=runtest12)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
