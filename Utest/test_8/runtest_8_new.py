import os
import sys
from sys import *
sys.path.append("../")
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest8(inifile, trajfile):
    maxtime = get_maxtime(inifile)
    fps, N, traj = parse_file(trajfile)
    if not N:
        logging.critical(" N = %d"%N)
        exit(FAILURE)
    evac_time = ( max( traj[:,1] ) - min( traj[:,1] ) ) / float(fps)

    if evac_time > maxtime*0.5:
        logging.info("%s exits with FAILURE evac_time = %f (maxtime =  %f)"%(argv[0], evac_time, maxtime))
        exit(FAILURE)
    else:
        logging.info("evac_time = %f (maxtime =  %f)"%(evac_time, maxtime))


if __name__ == "__main__":
    test = JPSRunTestDriver(8, argv0=argv[0], testdir=os.path.abspath("."))
    test.run_test(testfunction=runtest8)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)