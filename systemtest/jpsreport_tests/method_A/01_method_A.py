#!/usr/bin/env python3
#####################################################################
# For every source, check  if agents are generated within the bounding box defined
# with xmin, x_max, y_min, y_max
#
# Note:
# In this test N_Create == agents_max
# To add new cases increment the group_id by 1.
# source_id == group_id
#####################################################################
import os
from sys import argv, path
import logging
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
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
        logging.info('--> T_in: different dist -- p-value: {0:.4f}'.format(Tin_p_value))
        Tin_exit = False

    if cumPed_p_value > alpha:
        logging.info('--> cum_Ped: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(cumPed_p_value))
        cumPed_exit = True
    else:
        logging.info('--> cum_Ped: different dist (reject H0) -- p-value: {0:.4f}'.format(cumPed_p_value))
        cumPed_exit = False

    success = (Tin_exit == True) and (cumPed_exit == True)        
    if not success:
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=path[0], utestdir=utestdir)
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
