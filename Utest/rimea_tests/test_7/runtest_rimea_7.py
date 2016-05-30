#!/usr/bin/env python3
"""
Test description
================
Distribute pedestrian's speed according to Tab. P. 6
4 different groups are distributed
1. v<30
2. 30<v<50
3. v>50
4. handicapted

Check whether the speed values are within the specified fange.

Remarks
=======
Use new dedicated python console if you run this code with spyder

1. The velocities are assumed to be constant and are therefor averaged: v = DX/DT.
2. The ped-ped repulsive forces are intentionally set to zero, since in diesem
   Test we are interested only in the desired velocies.
   Some error messages could result (overlapping)
3. Script Gauss.py checks that the sigmas, means are in accordance with the min-max values in Tab. 1

This test produces two files

1. a png-file showing the distribution of the velocities
2. a csv-file with two comma separated columns: ids, velocities

@todo: write csv-file to excel as in the rimea reports

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdf
"""

import os
import sys
import numpy as np
import pylab as P
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *


# Group 1, 2, 3, 4
weidman_max_vel = np.array([1.61, 1.54, 1.41, 0.76])
weidman_min_vel = np.array([0.58, 1.41, 0.68, 0.46])
# --->Parameters means and sigmas are used in the master-inifile<---
means = 0.5*(weidman_max_vel+weidman_min_vel)   #[1.095, 1.475, 1.045, 0.61]
sigmas = [0.2, 0.02, 0.13, 0.05] # check gauss.py for a visual check
max_velocity = max(weidman_max_vel)
min_velocity = min(weidman_min_vel)
#====================================================
colors = ["g", "r", "b", "m"]
csv_file = "ids_velocities.csv"
figname = "velocity_distribution.png"

if os.path.isfile(figname):
    os.remove(figname)

def run_rimea_test7(inifile, trajfile):
    velocities = []
    fps, numpeds, traj = parse_file(trajfile)
    numPedsGr = numpeds/len(sigmas)
    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, numpeds_group = %d, fps: %d", numpeds, numPedsGr, fps)
    for ped in peds:
        ptraj = traj[traj[:, 0] == ped]
        ptraj = ptraj[np.diff(ptraj[:, 2]) != 0] # only when movement of <ped> starts
        dx = np.sqrt(np.sum((ptraj[0, 2:] - ptraj[-1, 2:])**2)) # distance
        dt = (ptraj[-1, 1] - ptraj[0, 1])/fps
        v = dx/dt
        velocities.append(v)

    id_velocity = np.vstack((peds, velocities))
    np.savetxt(csv_file, id_velocity.T, delimiter=',', fmt=["%d", "%f"])
    i = 0
    j = 1 # subplot index
    logging.info("Ploting distributions...")
    for (mu, sigma, c) in zip(means, sigmas, colors):
        P.subplot("41%d"%j)
        n, bins, patches = P.hist(velocities[i:i+numPedsGr-1], 50, normed=1, histtype='bar',
                                  facecolor='%s'%c, alpha=0.7)
        y = P.normpdf(bins, mu, sigma)
        P.plot(bins, y, 'k--', linewidth=1.5,
               label=r"$\mathcal{N}(%.2f, %.2f)$"%(mu, sigma))
        P.legend()
        i += numPedsGr
        j += 1

    P.tight_layout()
    P.savefig(figname)
    if min(velocities) < min_velocity or max(velocities) > max_velocity:
        logging.critical("%s exits with FAILURE. min_velocity = %.3f (>%.3f?), max_velocity = %.3f (<%.3f?)",
                         argv[0], min(velocities), min_velocity, max(velocities), max_velocity)
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(7, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test7)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)







