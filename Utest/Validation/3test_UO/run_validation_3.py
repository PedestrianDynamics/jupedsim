#!/usr/bin/env python3
"""
Test description
================
- UO
- Fundamental Diagram in 2D, test number 103
- Width = 2.0 m
- Length = 8.0 m
- Measurement area: X = [9, 11],  Y = [1.3, 3.7]

Remarks
=======
TODO: Compare two "clouds" of points and return a number.

Source
======

"""

import os
import sys
import matplotlib.pyplot as plt
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

tolerance = 0.05
ms = 20 # size of labels
mt = 18 # size of ticks

def plot_results(results):
    dexp = results[0]
    dsim = results[1]
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    p1 = plt.plot(dsim[:, 2], dsim[:, 3], ".b", ms=0.1 , alpha=0.5, label="simulation")
    p2 = plt.plot(dexp[:, 0], dexp[:, 1], "xr", label="experiment")
    plt.ylabel(r"$v\; [m/s]$", size=ms)
    plt.xlabel(r"$\rho \; [1/m^2]$", size=ms)
    plt.xticks(fontsize=mt)
    plt.yticks(fontsize=mt)
    leg = plt.legend(loc="best", numpoints=1)

    fig.set_tight_layout(True)
    plt.savefig("fd2d_uo.png", dpi=300)

def run_validation_3(inifile, trajfile):
    return 0

def eval_results(results):
    dexp = results[0]
    dsim = results[1]
    res = CDFDistance(dsim[:, 1], dsim[:, 2], dexp[:, 0], dexp[:, 1])
    return res

if __name__ == "__main__":
    test = JPSRunTestDriver(103, argv0=argv[0],
                            testdir=sys.path[0],
                            utestdir=utestdir)

    results = test.run_test(testfunction=run_validation_3, fd=1)
    res = eval_results(results) 
    if res < critical_value:
        plot_results(results)
        logging.info("%s exits with SUCCESS. res= %f" % (argv[0], res))
        exit(SUCCESS)
    else:
        logging.info("%s exits with FAILURE. res= %f" % (argv[0], res))
        exit(FAILURE)
