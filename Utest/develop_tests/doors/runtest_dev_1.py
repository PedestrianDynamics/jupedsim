#!/usr/bin/env python3
import os
import sys
#import matplotlib.pyplot as plt
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def runtest(inifile, trajfile):
    tolerance = 0.2
    failure = 0
    ids, outflow = get_outflow(inifile)
    for (i, o) in zip(ids, outflow):
        filename = "flow_exit_id_%d_rate_%.2f_%s.txt"%(i, o, os.path.basename(trajfile).split(".")[0])
        if not os.path.exists(filename):
            logging.info("ERROR: can not find statistics file %s"%filename)
            exit(FAILURE)

        data = np.loadtxt(filename)
        N = data[:, 1]
        T = data[:, 0]
        J = N[-1]/T[-1]
 #       plt.plot(T, N)
 #       plt.plot(T, N, lw=2)
 #       plt.plot(T, o*T, "-k", lw=2)
        logging.info("outflow (file %s) |  inifile: %.2f | state-file: %.2f (+-%.2f)"%(filename, o, J, abs(J-o)))
        if abs(J-o) > tolerance:
            failure = 1

 #   logging.info("Plot to flow.png")
    # plt.savefig("flow.png")
    if failure:
        logging.info("flow from statistics files does not much expected flow from inifile")
        exit(FAILURE)





if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
