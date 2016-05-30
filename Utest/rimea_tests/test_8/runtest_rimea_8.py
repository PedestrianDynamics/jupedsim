#!/usr/bin/env python3
"""
Test description
================
A 3D building is simulated and the influence of parameter e.g. speed
is investigated. It should be shown how the evacuation time behaves with respect
to the investigated parameter.

Remarks
=======
Use new dedicated python console if you run this code with spyder

There is no fail criterion in this test. Just documentation.

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/r2.2.1.pdf
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
import glob
import warnings
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *



def run_rimea_test8(inifile, trajfile):
    print(inifile)
    files = glob.glob("trajectories/*_exit*")
    if len(files) == 0:
        logging.critical("%s exists with failure! Found no exit-files.", argv[0])
        exit(FAILURE)

    for f in files:
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            d = np.loadtxt(f)

        if len(d) == 0:
            logging.critical("File %s is empty", f)
            logging.critical("%s exists with failure!", argv[0])
            exit(FAILURE)

        num_evacuated = max(d[:, 1]) # >0 ?
        evac_time = max(d[:, 0])
        logging.info("%d peds evacuated from exit <%s>. Evac_time: %f",
                     num_evacuated, f.split(".dat")[0].split("_id_")[-1], evac_time)



if __name__ == "__main__":
    test = JPSRunTestDriver(8, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test8)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)











