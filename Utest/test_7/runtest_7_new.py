from sys import argv
from JPSRunTest import JPSRunTestDriver
from utils import *

__author__ = 'Oliver Schmidts'


def runtest7(inifile, trajfile):
    fps, N, traj = parse_file(trajfile)
    failure = False
    group_1 = [1, 2, 3]
    group_2 = [4, 6, 5]
    e1 = [26, 1, 2] # y, x1, x2
    e2 = [18, 1, 2] # x, y1, y2
    for ped in group_1:
        traj1 = traj[ traj[:,0] == ped ]
        x = traj1[:,2]
        y = traj1[:,3]
        if not PassedLineY(traj1, e1):
            logging.critical("ped %d did not exit from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e1[1], e1[0], e1[2], e1[0]))
            failure = True
        else:
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e1[1], e1[0], e1[2], e1[0]))

    for ped in group_2:
        traj1 = traj[ traj[:,0] == ped ]
        x = traj1[:,2]
        y = traj1[:,3]
        if not PassedLineX(traj1, e2):
            logging.critical("ped %d did not exit from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))
            failure = True
        else:
            logging.info("ped %d  exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))

    if failure:
        logging.critical("%s exists with failure!"%argv[0])
        exit(FAILURE)


def PassedLineX(p, exit):
    """
    check if pedestrian (given by matrix p) passed the vertical line x, [y1, y2] y1<y2
    """
    x = exit[0]
    y1 = exit[1]
    y2 = exit[2]
    return any(p[:,2] <= x) & any(p[:,2] >= x) & any(p[:,3] >= y1) & any(p[:,3] <= y2)


def PassedLineY(p, exit):
    """
    check if pedestrian (given by matrix p) passed the horizontal line y, [x1, x2] x1<x2
    """
    y = exit[0]
    x1 = exit[1]
    x2 = exit[2]
    return any(p[:,3] <= y) & any(p[:,3] >= y) & any(p[:,2] >= x1) & any(p[:,2] <= x2)


if __name__ == "__main__":
    test = JPSRunTestDriver(7, argv0=argv[0])
    test.run_test(testfunction=runtest7)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)