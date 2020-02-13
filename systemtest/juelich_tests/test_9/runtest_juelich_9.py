#!/usr/bin/env python3
"""
Test description
================
- Fundamental Diagram in 2D, test number 102
- Width = 1.8 m
- Length = 26.0 m
- Measurement area: X = [10, 16],  Y = [-0.9, 0.9]

Remarks
=======
TODO: Compare two "clouds" of points and return a number.

Source
======

"""

import matplotlib.pyplot as plt
import numpy as np
import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

tolerance = 0.05
ms = 20 # size of labels
mt = 18 # size of ticks

def eval_results(results):
    results = np.sort(results, axis=0)
    num_threads = results[:, 0]
    evac_times = results[:, 1]
    std = np.std(evac_times)
    mean = np.mean(evac_times)
    logging.info("INFO: mean = %f (+-%f)", mean, std)
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.plot(num_threads, evac_times, "o-b", alpha=0.5)

    plt.ylabel(r"Evacuation Time [s]", size=ms)
    plt.xlabel(r"#Threads", size=ms)
    plt.xlim([min(num_threads)-0.5, max(num_threads)+0.5])
    plt.xticks(fontsize=mt)
    plt.xticks(num_threads, fontsize=mt)
    plt.title("mean = %f (+-%f) [s]" % (mean, std))
    plt.grid(alpha=0.7)
    plt.savefig("evactimes.png", dpi=300)

def run_test_9(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    num_threads = get_num_threads(inifile)
    fps, N, traj = parse_file(trajfile)
    if not N:
        logging.critical(" N = %d"%N)
        exit(FAILURE)
    evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
    return (num_threads, evac_time)

if __name__ == "__main__":
    test = JPSRunTestDriver(9, argv0=argv[0],
                            testdir=sys.path[0],
                            utestdir=utestdir, jpscore=argv[1])

    results = test.run_test(testfunction=run_test_9)
    eval_results(results)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
