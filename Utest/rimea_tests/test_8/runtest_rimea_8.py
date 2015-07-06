#!/usr/bin/env python
"""
Test description
================
A 3D building is simulated and the influence of parameter e.g. speed
is investigated. It should be shown how the evacuation time behaves with respect
to the investigated parameter.

Remarks
=======
Test not running in JuPedSim.

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdf
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test8(inifile, trajfile):
    fps, numpeds, traj = parse_file(trajfile)



if __name__ == "__main__":
    test = JPSRunTestDriver(8, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test8)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)







