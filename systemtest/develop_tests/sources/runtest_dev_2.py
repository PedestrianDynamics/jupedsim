#!/usr/bin/env python3
import os
import sys
import logging
import numpy as np
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import equals, contains, SUCCESS, FAILURE


def runtest(inifile, trajfile):
    failure = 0
    pos_err = 0.15
    d = {}

    f = open(trajfile)
    lines = f.readlines()
    f.close()
# read file and save content in dic as follows: d[ped_id] = [time, x, y]
    for line in lines:
        line = line.strip()
        #framerate: 8.00

        if line.startswith("#framerate"):
            fps = float(line.split(":")[1])
            time_err = 1/fps

        if not line or line.startswith("#"):
            continue

        line = line.split()
        pid = int(line[0])
        frame = int(line[1])
        x = float(line[2])
        y = float(line[3])
        time = frame/fps

        if pid not in d:
            d[pid] = [time, x, y]


# start testing sources
    source_ids = [1, 2, 3, 4]
    logging.info("testing source ids: %s", ", ".join(map(str, source_ids)))
    # time at 5, 10, 15, 20
    # position in (4, 4)
    for source_id in source_ids:
        if not equals(d[source_id][0], 5*source_id, err=time_err) or \
        not equals(d[source_id][1], 4, err=pos_err) or \
        not equals(d[source_id][2], 4, err=pos_err):

            err_msg = ("source "+str(source_id)+" "
                       "should start at time "+str(5*source_id)+" "
                       "in position (4, 4), but it starts at time "+
                       str(d[source_id][0])+" in position "+
                       str(d[source_id][1])+", "+ str(d[source_id][2]))

            logging.error(err_msg)
            failure = 1

    if failure == 0:
        logging.info("OK!")
    source_ids_2 = [5, 6, 7, 8]
    logging.info("testing source ids: %s", ", ".join(map(str, source_ids_2)))
    # time 5, 10, 15, 20
    # x in [0, 1]
    # y in [0, 1]
    for j, source_id in enumerate(source_ids_2):
        if not equals(d[source_id][0], 5*(j+1), err=time_err) or \
        not contains(d[source_id][1], 0, 1, pos_err) or \
        not contains(d[source_id][2], 0, 1, pos_err):

            err_msg = ("source "+str(source_id)+" "
                       "should start at time "+str(5*(j+1))+" "
                       "in position (4, 4), but it starts at time "+
                       str(d[source_id][0])+" in position "+
                       str(d[source_id][1])+", "+ str(d[source_id][2]))

            logging.error(err_msg)
            failure = 1

    if failure == 0:
        logging.info("OK!")

    source_ids_3 = [9, 10, 11]
    logging.info("testing source ids: %s", ", ".join(map(str, source_ids_3)))

    # time in [10, 15]
    # x in [3, 5]
    # y in [2, 4]
    for source_id in source_ids_3:
        if not contains(d[source_id][0], 10, 15, time_err) or \
        not contains(d[source_id][1], 3, 5, pos_err) or \
        not contains(d[source_id][2], 2, 4, pos_err):
            err_msg = ("source "+str(source_id)+" "
                       "should start at time in [10, 15] "
                       "and on a position in [3, 5]-[2, 4], "
                       "but it starts at "+str(d[source_id][0])+
                       " in "+str(d[1][1])+", "+ str(d[1][2]))

            logging.error(err_msg)
            failure = 1

    if failure == 0:
        logging.info("OK!")

    source_ids_4 = [12, 13, 14, 15, 16]
    logging.info("testing source ids: %s", ", ".join(map(str, source_ids_4)))
    # time in [15, 35]
    # x in [4, 6.2]
    # y in [3, 3.5]
    # frequency: 5

    times = []
    for source_id in source_ids_4:
        times.append(d[source_id][0])
        if not contains(d[source_id][0], 15, 35, time_err) or \
        not contains(d[source_id][1], 4, 6.2, pos_err) or \
        not contains(d[source_id][2], 3, 3.5, pos_err):
            err_msg = ("source "+str(source_id)+" "
                       "should start at time in [15, 35] "
                       "and on a position in [4, 6.2]-[3, 3.5], "
                       "but it starts at "+str(d[source_id][0])+
                       " in "+str(d[1][1])+", "+ str(d[1][2]))

            logging.error(err_msg)
            failure = 1
    # check frequency
    if not np.all(np.equal(np.diff(times), 5)):
        err_msg = ("source "+str(source_id)+ " "
                   "should create with a frequency of 5. "
                   "Got: "+", ".join(map(str, times)))

        logging.error(err_msg)
        failure = 1

    if failure == 0:
        logging.info("OK!")

    source_ids_5 = range(17, 29)
    logging.info("testing source ids: %s", ", ".join(map(str, source_ids_5)))
    # time in [10, 50]
    # x in [4, 6.2]
    # y in [5, 6.1]
    # frequency: 5
    for source_id in source_ids_5:
        if not contains(d[source_id][0], 10, 50, time_err) or \
        not contains(d[source_id][1], 4, 6.2, pos_err) or \
        not contains(d[source_id][2], 5, 6.1, pos_err):
            err_msg = ("source "+str(source_id)+" "
                       "should start at time in [10, 50] "
                       "and on a position in [4, 6.2]-[5, 6.1], "
                       "but it starts at "+str(d[source_id][0])+
                       " in "+str(d[1][1])+", "+ str(d[1][2]))

            logging.error(err_msg)
            failure = 1

    # check frequency=5
    if not np.all(np.equal(np.diff(times), 5)):
        err_msg = ("source "+str(source_id)+ " "
                   "should create with a frequency of 5. "
                   "Got: "+", ".join(map(str, times)))

        logging.error(err_msg)
        failure = 1

    if failure == 0:
        logging.info("OK!")

    if failure:
        exit(FAILURE)





if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
