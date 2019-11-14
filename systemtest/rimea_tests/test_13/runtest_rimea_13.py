#!/usr/bin/env python3
"""
Test description
================
The pedestrians are entering a corridor from a bottleneck. At the end of the
corridor is a stair. The exist lays behind the stair.

The test shows that there is a jam in front of the stair.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

In case of jam, flow at the beginnning of the stair should be smaller than the
flow in the corridor.

The reduced speed on stairs (up) is according to  Tab 1 Burghardt2014:
|----------+----------------|
| Handbook | Speed Stair Up |
|----------+----------------|
| PM       | 0.63 m/s       |
| WM       | 0.61 m/s       |
| NM       | 0.8 m/s        |
| FM       | 0.55 m/s       |
|----------+----------------|

Therefore, we choose for v0_upstairs a Gauss-distribution with
mean = 0.675 and sigma = 0.04

See also Fig. DistributionSpeedStairUp.png

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""

import matplotlib.pyplot as plt
import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *
import time

tolerance = 0.05

def eval_results(results):
    results = np.array(results)
    mean_cor = np.mean(results[:, 0])
    mean_stair = np.mean(results[:, 1])
    std_cor = np.std(results[:, 0])
    std_stair = np.std(results[:, 1])
    logging.info("mean corridor: %.2f (+- %.2f), mean stair: %.2f (+- %.2f)",
                 mean_cor, std_cor, mean_stair, std_stair)
    plt.errorbar(list(range(len(results[:, 0]))), results[:, 0],
                 yerr=std_cor, fmt='-o', lw=2, label="Flow corridor")
    plt.errorbar(list(range(len(results[:, 1]))), results[:, 1],
                 yerr=std_stair, fmt='-D', lw=2, label="Flow stair")
    plt.ylabel("$J$", size=18)
    plt.xlim([-0.5, len(results[:, 0])+0.5])
    plt.xlabel("# runs")
    plt.title("mean_cor = %f (+-%f), mean_stair = %f (+-%f)"%
              (mean_cor, std_cor, mean_stair, std_stair))
    plt.legend(loc="best")
    plt.savefig("flow.png")

    if abs(mean_cor-mean_stair) < tolerance:
        logging.critical("%s exists with FAILURE. Flows are almost equal:", argv[0])
        exit(FAILURE)

def run_rimea_test13(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    exit_basement = 0
    start_stair = 12
    J_corridor = flow(fps, N, traj, exit_basement)
    J_stair = flow(fps, N, traj, start_stair)
    logging.info("J_corridor = %.2f, J_stair = %.2f (tolerance = %.2f)",
                 J_corridor, J_stair, tolerance)
    return (J_corridor, J_stair)

if __name__ == "__main__":
    start_time=time.time()
    test = JPSRunTestDriver(13, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    results = test.run_test(testfunction=run_rimea_test13)
    eval_results(results)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)













