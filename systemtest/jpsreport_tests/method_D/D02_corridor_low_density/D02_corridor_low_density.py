#!/usr/bin/env python3
# ---------
# Test of classical voronoi output
# Pedestrians are moving in a 6x6 grid in x-direction with v=1 m/s
# They have a fixed horizontal and vertical distance of 1.5m
# Due to the alignment the inner voronoi cells should be squares
# The inner voronoi cells should lie entirely in the specified square measurement area for frame 109
# Real voronoi density can be calculated by 1 person / (1.5m*1.5m)
# Real velocity is 1.0 m/s for all frames (since cut off is false parts of the outer voronoi cells lie in the measurement area)
# ---------
import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(path[0]))))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
import numpy as np
from JPSRunTest import JPSRunTestDriver

# real density can be calculated based on the square
real_density = 1 / (1.5 ** 2)
# real velocity must remain the same
real_velocity = 1
# accepted error
acceptance_range = 0.001
# frame when inner vornoi cells of 16 people are entirely in the measurement area
frame = 109


def runtest(inifile, trajfile):
    logging.info("===== Method D - Classical Voronoi ===============")
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'Classical_Voronoi',
                                         'rho_v_Voronoi_%s_id_1.dat' % trajfile
                                         )

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    jpsreport_data = np.loadtxt(jpsreport_result_file)

    # check density for one frame
    jpsreport_density = jpsreport_data[jpsreport_data[:, 0] == frame][0, 1]

    if abs(jpsreport_density - real_density) < acceptance_range:
        logging.info("density in measurement area should be OK.")
    else:
        logging.critical(
            "density value in measurement area did not match result. Got {}. Expected {}".format(jpsreport_density,
                                                                                                 real_density))
        exit(FAILURE)

    # check velocity for all frames
    jpsreport_velocity = jpsreport_data[:, 2]
    real_velocity_array = np.ones(np.size(jpsreport_velocity)) * real_velocity

    if np.all(np.abs(jpsreport_velocity - real_velocity_array) < acceptance_range):
        logging.info("velocity calculation should be OK.")
    else:
        logging.critical(
            "velocity values did not match result. Got {}. Expected {}".format(jpsreport_velocity, real_velocity_array))
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="trajectory_grid_6x6.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
