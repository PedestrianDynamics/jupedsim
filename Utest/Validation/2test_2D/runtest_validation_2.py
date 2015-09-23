#!/usr/bin/env python
"""
Test description
================
- Fundamental Diagram in 2D
- Width = 1.8 m
- Length = 26.0 m
- Measurement area: X = [10, 16],  Y = [-0.9, 0.9]

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

def eval_results(results):
    fd_exp = results[0]
    fd_sim = results[1]
    dexp = results[0]
    dsim = results[1]
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.plot(dsim[:, 2], dsim[:, 3], ".b", alpha=0.5, label="simulation")
    plt.plot(dexp[:, 0], dexp[:, 1], "xr", label="experiment")
    plt.ylabel(r"$v\; [m/s]$", size=20)
    plt.xlabel(r"$\rho \; [1/m^2]$", size=20)
    plt.legend(loc="best", numpoints=1)
    fig.set_tight_layout(True)
    plt.savefig("fd2d.png", dpi=300)

def run_validation_2(inifile, trajfile):
    return 0

if __name__ == "__main__":
    jpsreportdir = "/Users/chraibi/Workspace/jpsreport"
    test = JPSRunTestDriver(101, argv0=argv[0],
                            testdir=sys.path[0],
                            utestdir=utestdir,
                            jpsreportdir=jpsreportdir)

    results = test.run_test(testfunction=run_validation_2, fd=1)
    eval_results(results)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
