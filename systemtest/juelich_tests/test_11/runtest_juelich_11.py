#! /usr/bin/env python3
import logging
import os
import subprocess
import sys
from os import path
from shutil import copyfile
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *
#=========================
testnr = 11
#========================
logging.basicConfig(stream=sys.stdout, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
PATH = os.path.dirname(os.path.abspath(__file__))

def get_evak_time(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    etime =  (max( traj[:, 1] ) - min(traj[:, 1])) / float(fps)
    return etime

def call_test(num, suffix):
    logging.info("copy master_ini_%s.xml"%suffix)
    logging.info("copy geometry/geometry_test11_%s.xml"%suffix,)
    copyfile("%s/master_ini_%s.xml"%(PATH, suffix), "%s/master_ini.xml"%PATH)
    copyfile("%s/geometry/geometry_test11_%s.xml"%(PATH, suffix), "%s/geometry.xml"%PATH)
    test = JPSRunTestDriver(num, argv0=sys.argv[0], testdir=sys.path[0], utestdir=utestdir, jpscore=sys.argv[1])
    return test.run_test(testfunction=get_evak_time)

if __name__ == "__main__":
    threashold = 0.001
    result_a = call_test(111, "a")
    result_b = call_test(112, "b")
    diff = [a - b for a, b in zip(result_a, result_b)]
    success = all(v <= threashold for v in diff)
    if success:
        logging.info("%s exits with SUCCESS" % (sys.argv[0]))
        sys.exit(SUCCESS)
    else:
        logging.critical("%s exits with FAILURE" % (sys.argv[0]))
        logging.debug("result_a {}".format(", ".join(map(str, result_a))))
        logging.debug("result_b {}".format(", ".join(map(str, result_a))))
        logging.debug("diff {} (threashold = {})".format(", ".join(map(str, diff)), threashold))
        sys.exit(FAILURE)
