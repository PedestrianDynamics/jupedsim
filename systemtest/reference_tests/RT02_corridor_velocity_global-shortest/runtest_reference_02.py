#!/usr/bin/env python3

# Checks whether the results of the current version and of the expected output are identical,
# tests for reproducibility.

import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *

sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

from filecmp import cmp


def runtest(inifile, trajfile):
    expected_output_file = 'output_expected/traj.txt'

    success = cmp(trajfile, expected_output_file, False)
    if not success:
        logging.critical('The contents of {} and {} are not identical!'.format(trajfile, expected_output_file))
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=argv[1])

    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS." % (argv[0]))

    exit(SUCCESS)
