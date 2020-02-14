#!/usr/bin/env python3
#---------
# 6 pedestrians created at frame >= 9
# ids are discontinuous
# creation times in sources are 1, 5, 10, 15, 25, 30
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
# adding 9 frames (fps=8). The distance for the first agent takes exactly 7 seconds.
should_be_7 = np.array([7 + 1.125, 11 + 1.125, 16 + 1.125, 21 + 1.125, 31 + 1.125, 36 + 1.125])
should_be_9 = should_be_7 + 2

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
    time_change_9 = data_9[np.nonzero(np.diff(data_9[:, 1]) >0)][:, 0]
    if np.all(np.abs(time_change_9 - should_be_9) < 0.5):
        print("Got", time_change_9, ", expected", should_be_9)
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
    time_change_7 = data_7[np.nonzero(np.diff(data_7[:, 1]) >0)][:, 0]

    if np.all(np.abs(time_change_7 - should_be_7) < 0.5):
        print("Got", time_change_7, ", expected", should_be_7)
    else:
        print("Error. Got", time_change_7, ", expected", should_be_7)
        exit(FAILURE)


if __name__ == "__main__":
    if len(argv) < 4:
        exit("usage: %s jpsreport_path inifile trajfile"%argv[0])
    jpsreport = argv[1]
    inifile = argv[2]
    trajfile = argv[3]
    subprocess.call([jpsreport, "%s" % inifile])
    runtest(trajfile)
    exit(SUCCESS)
