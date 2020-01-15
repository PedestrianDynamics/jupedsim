#!/usr/bin/env python3
#################################################################################
# For every source, check if agents are generated in the given position defined by
# startX and startY
# Note:
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
    # assuming source_ids can be extracted from group ids
    source_ids = []
    for key in data.keys():
        source_ids.append(data[key][-1])

    source_file = "sources.xml" #get_source_file(inifile)
    for _id in source_ids:

        startX, startY = get_starting_position(_id, source_file)
        source = Source(ids=[_id],
                        startX=startX,
                        startY=startY)
        success = test_source(data, source, time_err, pos_err)
        if not success:
            exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
