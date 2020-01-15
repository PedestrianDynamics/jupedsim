#!/usr/bin/env python3
#################################################################################
# For every source, check  if agents are generated within the bounding box defined
# with xmin, x_max, y_min, y_max
#
# Note:
# In this test N_Create == agents_max
# To add new cases increment the group_id by 1.
# source_id == group_id
#################################################################################
import os
from sys import argv, path
import logging
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0])) # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver
from source_utils import *

def runtest(inifile, trajfile):
    pos_err = 0.15
    time_err, data = get_data(trajfile)
    if not data:
        logging.error("Simulation did not run properly")
        exit(FAILURE)

    source_file = "sources.xml" #get_source_file(inifile)
    # assuming source_ids can be extracted from group ids
    source_ids = set()
    for key in data.keys():
        source_ids.add(data[key][-1])

    for _id in source_ids:
        N_create = get_N_create(_id, source_file)
        xmin, xmax, ymin, ymax = get_bounding_box(_id, source_file)
        ids = [i + (_id-1)*N_create for i in range(1, N_create+1)]
        source = Source(ids=ids,
                        xmin=xmin,
                        xmax=xmax,
                        ymin=ymin,
                        ymax=ymax
                       )
        success = test_source(data, source, time_err, pos_err)
        if not success:
            exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
