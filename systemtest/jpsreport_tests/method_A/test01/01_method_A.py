#!/usr/bin/env python3
#####################################################################
# Check if the result of jpsreport matches a reference file.
#####################################################################
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

def runtest(inifile, trajfile):
    logging.info("===== Method A - Flow-NT ===============")
    data_1_filename = 'reference_data_Method_A_Flow_NT.dat'
    data_1 = np.loadtxt(data_1_filename)

    data_2_filename = os.path.join('./Output',
                                   'Fundamental_Diagram',
                                   'FlowVelocity',
                                   'Flow_NT_%s_id_2.dat'%trajfile
                                   )
    if not os.path.exists(data_2_filename):
        logging.error("jpsreport did not output results correctly.")
        exit(FAILURE)
    data_2 = np.loadtxt(data_2_filename)

    # test
    alpha = 0.01
    Tin_ks_statistic, Tin_p_value = ks_2samp(data_2[:, 0],
                                             data_1[:, 0])
    cumPed_ks_statistic, cumPed_p_value = ks_2samp(data_2[:, 1],
                                                   data_1[:, 1])

    if Tin_p_value > alpha:
        logging.info('--> T_in: same dist  -- p-value: {0:.4f}'.format(Tin_p_value))
        Tin_exit = True
    else:
        logging.critical('--> T_in: different dist -- p-value: {0:.4f}'.format(Tin_p_value))
        Tin_exit = False

    if cumPed_p_value > alpha:
        logging.info('--> cum_Ped: same dist -- p-value: {0:.4f}'.format(cumPed_p_value))
        cumPed_exit = True
    else:
        logging.critical('--> cum_Ped: different dist -- p-value: {0:.4f}'.format(cumPed_p_value))
        cumPed_exit = False

    success = Tin_exit and cumPed_exit
    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
