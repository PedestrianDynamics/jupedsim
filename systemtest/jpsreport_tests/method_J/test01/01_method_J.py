#!/usr/bin/env python3
# Test method J using a reference file
import os
from sys import argv, path
import logging
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(path[0]))))
path.append(utestdir)
path.append(os.path.dirname(path[0])) # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver
from scipy.stats import ks_2samp
import numpy as np

alpha = 0.01

def runtest(inifile, trajfile):
    logging.info("===== Method J =========================")

    data_1_filename = 'reference_data_Method_J.dat'
    data_1 = np.loadtxt(data_1_filename)

    data_2_filename = os.path.join('./Output',
                                   'Fundamental_Diagram',
                                   'Classical_Voronoi',
                                   'rho_v_Voronoi_J_%s_id_5.dat'%trajfile
                                   )
    if not os.path.exists(data_2_filename):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    data_2 = np.loadtxt(data_2_filename)

    density_ks_statistic, density_p_value = ks_2samp(data_2[:, 1], data_1[:, 1])

    velocity_ks_statistic, velocity_p_value = ks_2samp(data_2[:, 2], data_1[:, 2])

    if density_p_value > alpha:
        logging.info('--> density: same dist -- p-value: {0:.4f}'.format(density_p_value))
        density_exit = True
    else:
        logging.critical('--> density: different dist -- p-value: {0:.4f}'.format(density_p_value))
        density_exit = False

    if velocity_p_value > alpha:
        logging.info('--> velocity: same dist -- p-value: {0:.4f}'.format(velocity_p_value))
        velocity_exit = True
    else:
        logging.critical('--> velocity: different dist -- p-value: {0:.4f}'.format(velocity_p_value))
        velocity_exit = False

    success = velocity_exit and density_exit
    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
