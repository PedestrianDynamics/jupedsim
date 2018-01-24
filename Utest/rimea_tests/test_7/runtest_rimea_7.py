#!/usr/bin/env python3
"""
Test description
================
Distribute 200 pedestrian with velocities according to Rimea, page 15, picture 2

Four different groups are distributed:
1. 20 < age < 30
2. 30 < age < 50
3. 50 < age < 80
4. handicapped

Distribute 50 pedestrian for each group within a rectangle of 50m lenght and 200m width.

Check whether the speed values are within the specified fange like Rimea, page 15, picture 2

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

1. The velocities are assumed to be constant and are therefor averaged: v = DX/DT.
2. The ped-ped repulsive forces are intentionally set to zero, since in this
   test we are only interested in the desired velocies.
   ---> Overlapping-error to be considered
   
3. Script Gauss.py checks that the means and sigmas are in accordance with the min-max values.

This test produces two files

1. a png-file showing the distribution of the velocities
2. a csv-file with two comma separated columns: ids, velocities

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""

import numpy as np
import os
import pylab as P
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

# Group 1, 2, 3, 4
weidman_max_vel = np.array([1.61, 1.54, 1.41, 0.76])
weidman_min_vel = np.array([0.58, 1.41, 0.68, 0.46])

# Parameters means and sigmas are used in the master-inifile
means = 0.5*(weidman_max_vel+weidman_min_vel) # [1.095, 1.475, 1.045, 0.61]
sigmas = [0.2, 0.02, 0.13, 0.05] # Check gauss.py for a visual check
max_velocity = max(weidman_max_vel)
min_velocity = min(weidman_min_vel)
#====================================================
colors = ['g', 'r', 'b', 'm']
csv_file = 'ids_velocities.csv'
figname = 'velocity_distribution.png'

if os.path.isfile(figname):
    os.remove(figname)

def run_rimea_test7(inifile, trajfile):
    velocities = []
    # Read data
    fps, numpeds, traj = parse_file(trajfile)
    numPedsGr = numpeds/len(sigmas)
    peds = np.unique(traj[:, 0])
    logging.info("=== npeds: %d, numpeds_group = %d, fps: %d", numpeds, numPedsGr, fps)
    for ped in peds:
        # Take only data of ped i
        ptraj = traj[traj[:, 0] == ped]
        # Only when ped i moves
        ptraj = ptraj[np.diff(ptraj[:, 2]) != 0]
        # Distance
        dx = np.sqrt(np.sum((ptraj[0, 2:] - ptraj[-1, 2:])**2))
        # Time
        dt = (ptraj[-1, 1] - ptraj[0, 1])/fps
        # Velocity
        v = dx/dt
        velocities.append(v)

    id_velocity = np.vstack((peds, velocities))
    np.savetxt(csv_file, id_velocity.T, delimiter=',', fmt=["%d", "%f"])
    
    # Plotting
    i = 0
    j = 1 # Subplot index
    logging.info("Ploting distributions...")
    for (mu, sigma, c) in zip(means, sigmas, colors):
        P.subplot("41%d"%j)
        n, bins, patches = P.hist(velocities[i:i+numPedsGr-1],50, normed=1,histtype='bar',facecolor='%s'%c, alpha=0.7)
        y = P.normpdf(bins, mu, sigma)
        P.plot(bins, y, 'k--',linewidth=1.5,label=r"$\mathcal{N}(%.2f, %.2f)$"%(mu, sigma))
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