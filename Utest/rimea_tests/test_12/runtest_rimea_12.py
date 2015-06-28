#!/usr/bin/env python
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test12(inifile, trajfile):
    """
    condition of this test is not clear enough...
    """
    pass


if __name__ == "__main__":
    test = JPSRunTestDriver(12, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test12)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)







