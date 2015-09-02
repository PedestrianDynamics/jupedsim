#!/usr/bin/env python
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
import scipy
import scipy.stats
import matplotlib.pyplot as plt
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest14(inifile, trajfile, PX, PY):
    maxtime = get_maxtime(inifile)
    fps, N, data = parse_file(trajfile)
    data = data[ data[:,1] == 0  ]

    x = data[:,2]
    y = data[:,3]

    nx = plt.hist(x, bins=10)[0]
    ny = plt.hist(y, bins=10)[0]

    px = scipy.stats.chisquare(nx)[1]
    py = scipy.stats.chisquare(ny)[1]

    PX.append(px)
    PY.append(py)



if __name__ == "__main__":
    OK = 1
    PX = []  #p-value for x
    PY = []  #p-value for y
    test = JPSRunTestDriver(14, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(runtest14, PX, PY)
    plt.subplot(211)
    plt.plot(PX, label="PX")
    plt.ylabel("px")
    plt.subplot(212)
    plt.plot(PY, label="PY")
    plt.ylabel("py")
    plt.savefig("px_py.png")


    if np.mean(PX) < 0.1 or np.mean(PY)< 0.1:
        logging.info("%s exits with FAILURE PX = %f   PY = %f"%(argv[0], np.mean(PX), np.mean(PY)))
        OK = 0
        exit(FAILURE)

    else:
        logging.info("PX = %f  PY = %f"%( np.mean(PX), np.mean(PY)))
    exit(SUCCESS)
