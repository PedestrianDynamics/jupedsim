#!/usr/bin/env python3
#---------
# one pedestrian created at frame 0
# pedestrian starts in position (0, 1) and moves forward
# in a straight corridor with a speed of 1 m/s
#
# arrival times should be:
# - 7s at line in x=7
# - 9s at line in x=9
#---------
import os
from sys import argv, path
import logging
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(path[0]))))
path.append(utestdir)
path.append(os.path.dirname(path[0])) # source helper file
from utils import SUCCESS, FAILURE
import numpy as np
import subprocess

should_be_7 = 7.0
should_be_9 = 9.0


def runtest(trajfile):
    logging.info("===== Method A - Flow-NT ===============")
    data_9_filename = os.path.join('./Output',
                                   'Fundamental_Diagram',
                                   'FlowVelocity',
                                   'Flow_NT_%s_id_9.dat'%trajfile
                                   )
    
    if not os.path.exists(data_9_filename):
        logging.error("jpsreport did not output results correctly.")
        exit(FAILURE)
    data_9 = np.loadtxt(data_9_filename)
    print("*****")
    print(data_9[np.nonzero(np.diff(data_9[:,1]) >0)][0][0])
    time_change_9 = data_9[np.nonzero(np.diff(data_9[:,1]) >0)][0][0]
    if np.abs(time_change_9 - should_be_9) < 0.5:
        print("OK")
    else:
        print("Got", time_change_9, ", expected", should_be_9)

    data_7_filename = os.path.join('./Output',
                                   'Fundamental_Diagram',
                                   'FlowVelocity',
                                   'Flow_NT_%s_id_7.dat'%trajfile
                                   )
    if not os.path.exists(data_7_filename):
        logging.error("jpsreport did not output results correctly.")
        exit(FAILURE)

    data_7 = np.loadtxt(data_7_filename)
    time_change_7 = data_7[np.nonzero(np.diff(data_7[:, 1]) >0)][0][0]
    print(data_7[np.nonzero(np.diff(data_7[:,1]) >0)][0][0])
    if np.abs(time_change_7 - should_be_7) < 0.5:
        print("OK")
    else:
        print("Got OK", time_change_7, ", expected", should_be_7)



if __name__ == "__main__":
    if len(argv) < 4:
        exit("usage: %s jpsreport_path inifile trajfile"%argv[0])
    jpsreport = argv[1]
    inifile = argv[2]
    trajfile = argv[3]
    subprocess.call([jpsreport, "%s" % inifile])
    runtest(trajfile)
