#!/usr/bin/env python
####################
# Test of Method D, I and J with cut_by_circle enabled and radius of 1
####################

import os
from sys import argv, path, stdout
import logging


utestdir = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(path[0])))))
path.append(utestdir)

path.append(os.path.dirname(os.path.dirname(path[0])))
from reference_test import check_diff_to_reference_data


from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

def runtest(inifile, trajfile):

    logging.info("===== Test of Method D: global_IFD enabled, cut_by_circle enabled with radius of 1 ===============")
    check_diff_to_reference_data()


if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile= "trajectories.xml", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
