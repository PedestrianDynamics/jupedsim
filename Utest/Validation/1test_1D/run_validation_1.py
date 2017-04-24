#!/usr/bin/env python3
"""
Test description
================
- Fundamental Diagram in 1D, test number 101
- Width = 0.6 m
- Length = 26.0 m
- Measurement area: X = [11, 13], Y = [-0.3, 0.3]

Remarks
=======
- JPSreport can not calculate Voronoi-diagrams for a "1D" scenario.
Therefore, we need to calculate the Voronoi diagram manually.

- TODO: Compare two "clouds" of points and return a number.

Remarks
=======

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
    plt.plot(dsim[:, 1], dsim[:, 2], ".b", alpha=0.5, label="simulation")
    plt.plot(dexp[:, 0], dexp[:, 1], "xr", label="experiment")
    plt.ylabel(r"$v\; [m/s]$", size=ms)
    plt.xlabel(r"$\rho \; [1/m]$", size=ms)
    plt.ylim([0, 1.5])
    plt.xlim([0, 3.0])
    plt.xticks(fontsize=mt)
    plt.yticks(fontsize=mt)
    plt.legend(loc="best", numpoints=1)
    fig.set_tight_layout(True)
    plt.savefig("fd1d.png", dpi=300)

def run_validation_1(inifile, trajfile):
    return 0

def eval_results(results):
    dexp = results[0]
    dsim = results[1]
    res = CDFDistance(dsim[:, 1], dsim[:, 2], dexp[:, 0], dexp[:, 1])
    return res

if __name__ == "__main__":
    test = JPSRunTestDriver(101, argv0=argv[0],
                            testdir=sys.path[0],
                            utestdir=utestdir)

    results = test.run_test(testfunction=run_validation_1, fd=1)
    res = eval_results(results) 
    if res < critical_value:
        plot_results(results)
        logging.info("%s exits with SUCCESS. res= %f" % (argv[0], res))
        exit(SUCCESS)
    else:
        logging.info("%s exits with FAILURE. res= %f" % (argv[0], res))
        exit(FAILURE)
