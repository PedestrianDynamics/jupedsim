#!/usr/bin/env python3
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

def plotit(PX, PY):
    ms = 20
    mt = 18
    fig = plt.figure()
    ax = fig.add_subplot(2, 1, 1)
    plt.plot(PX, label="PX")
    plt.ylabel("px", size=ms)
    plt.xticks(fontsize=mt)
    plt.yticks(fontsize=mt)
    ax = fig.add_subplot(2, 1, 2)
    plt.plot(PY, label="PY")
    plt.ylabel("py", size=ms)
    plt.xticks(fontsize=mt)
    plt.yticks(fontsize=mt)
    fig.set_tight_layout(True)
    plt.savefig("px_py.png")

def runtest14(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    fps, N, data = parse_file(trajfile)
    data = data[ data[:,1] == 0  ]

    x = data[:,2]
    y = data[:,3]

    nx = plt.hist(x, bins=10)[0]
    ny = plt.hist(y, bins=10)[0]

    px = scipy.stats.chisquare(nx)[1]
    py = scipy.stats.chisquare(ny)[1]

    return (px, py)
    # PX.append(px)
    # PY.append(py)



if __name__ == "__main__":
    PX = []  #p-value for x
    PY = []  #p-value for y
    test = JPSRunTestDriver(14, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    results = test.run_test(testfunction=runtest14)
    results = np.array(results)
    PX = results[:, 0]
    PY = results[:, 1]
    
    plotit(PX, PY)

    if np.mean(PX) < 0.1 or np.mean(PY)< 0.1:
        logging.info("%s exits with FAILURE PX = %f   PY = %f"%(argv[0], np.mean(PX), np.mean(PY)))
        exit(FAILURE)

    else:
        logging.info("PX = %f  PY = %f"%(np.mean(PX), np.mean(PY)))
        logging.info("%s exits with SUCCESS"%(argv[0]))
    exit(SUCCESS)
