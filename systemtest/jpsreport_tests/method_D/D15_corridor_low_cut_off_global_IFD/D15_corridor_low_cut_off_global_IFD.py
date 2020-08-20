#!/usr/bin/env python3
# ---------
# Test functionality of cut off option
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

# horizontal and vertical distance between pedestrians
ped_distance = 1.5
# ids of pedestrians to validate
ped_IDs = np.array([8.0, 9.0, 10.0, 11.0, 14.0, 15.0, 16.0, 17.0, 20.0, 21.0, 22.0, 23.0, 26.0, 27.0, 28.0, 29.0])
# cut off radius is set to 0.75 in inifile --> cut off function should be applied for inner vornoi cells
cut_off_has_effect = True


def runtest(inifile, trajfile):
    logging.info("===== Method D - Global IFD, Cut off   ===============")
    test_cut_off_all_frames(trajfile, ped_distance, ped_IDs, cut_off_has_effect)


if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="trajectory_grid_6x6.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
