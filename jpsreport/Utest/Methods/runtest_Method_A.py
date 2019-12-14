#!/usr/bin/env python

import os
import sys
import numpy as np
from scipy.stats import ks_2samp

import logging

logging.basicConfig(
    filename = "Utest_Methods.log",
    level    = logging.DEBUG,
    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S')

logging.info("===== Method A - Flow-NT ===============")

data_1_filename = 'reference_data_Method_A_Flow_NT.dat'
data_1 = np.loadtxt(data_1_filename)

data_2_filename = './Output_Methods/Fundamental_Diagram/FlowVelocity/Flow_NT_traj_Methods.txt_id_2.dat'
data_2 = np.loadtxt(data_2_filename)

# test
alpha = 0.01

Tin_ks_statistic, Tin_p_value = ks_2samp(data_2[:,0],data_1[:,0])

cumPed_ks_statistic, cumPed_p_value = ks_2samp(data_2[:,1],data_1[:,1])

if Tin_p_value > alpha:
    logging.info('--> T_in: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(Tin_p_value))
    Tin_exit = True
else:
    logging.info('--> T_in: different dist (reject H0) -- p-value: {0:.4f}'.format(Tin_p_value))
    Tin_exit = False

if cumPed_p_value > alpha:
    logging.info('--> cum_Ped: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(cumPed_p_value))
    cumPed_exit = True
else:
    logging.info('--> cum_Ped: different dist (reject H0) -- p-value: {0:.4f}'.format(cumPed_p_value))
    cumPed_exit = False

if (Tin_exit == True) and (cumPed_exit == True):
    logging.info('Method A: Test passed successfully.')
    logging.shutdown()
    sys.exit(0)
else:
    logging.info('Method A: Test passed not successfully.')
    logging.shutdown()
    sys.exit(1)

