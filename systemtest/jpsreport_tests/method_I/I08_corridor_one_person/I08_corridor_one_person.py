#!/usr/bin/env python3
# ---------
# Test of IFD output for one person moving along the corridor
# Special function of Method I: voronoi cell calculation should work since blind points are used for construction
# Cut off is used to approximate the voronoi cell by a circle
# ---------
import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(path[0]))))
path.append(utestdir)
path.append(os.path.dirname(os.path.dirname(path[0])))  # source helper file
from tests_method_DIJ import test_cut_off_all_frames

from utils import SUCCESS, FAILURE
import numpy as np
from JPSRunTest import JPSRunTestDriver

# cut_off_all_frames needs distance between pedestrians to calc incircle (needed for radius) even if there is one person only
ped_distance = 1.5
# ids of pedestrians to validate
ped_IDs = np.array([1.0])
# cut off radius is set to 0.75 in inifile --> cut off function should be applied for inner vornoi cells
cut_off_has_effect = True


def runtest(inifile, trajfile):
    logging.info("===== Method I - IFD with cut off for one person only  ===============")
    test_cut_off_all_frames(trajfile, ped_distance, ped_IDs, cut_off_has_effect)


if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="trajectory_grid_1x1.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
