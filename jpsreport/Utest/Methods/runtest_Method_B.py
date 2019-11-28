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

logging.info("===== Method B =========================")

data_1_filename = 'reference_data_Method_B.dat'
data_1 = np.loadtxt(data_1_filename)

data_2_filename = './Output_Methods/Fundamental_Diagram/TinTout/FDTinTout_traj_Methods.txt_id_4.dat'
data_2 = np.loadtxt(data_2_filename)

# test
alpha = 0.01

PedID_ks_statistic, PedID_p_value = ks_2samp(data_2[:,0],data_1[:,0])

density_ks_statistic, density_p_value = ks_2samp(data_2[:,1],data_1[:,1])

velocity_ks_statistic, velocity_p_value = ks_2samp(data_2[:,2],data_1[:,2])

if PedID_p_value > alpha:
    logging.info('--> PedID: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(PedID_p_value))
    PedID_exit = True
else:
    logging.info('--> PedID: different dist (reject H0) -- p-value: {0:.4f}'.format(PedID_p_value))
    PedID_exit = False

if density_p_value > alpha:
    logging.info('--> density: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(density_p_value))
    density_exit = True
else:
    logging.info('--> density: different dist (reject H0) -- p-value: {0:.4f}'.format(density_p_value))
    density_exit = False

if velocity_p_value > alpha:
    logging.info('--> velocity: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(velocity_p_value))
    velocity_exit = True
else:
    logging.info('--> velocity: different dist (reject H0) -- p-value: {0:.4f}'.format(velocity_p_value))
    velocity_exit = False

if (PedID_exit == True) and (velocity_exit == True) and (density_exit == True):
    logging.info('Method B: Test passed successfully.')
    logging.shutdown()
    sys.exit(0)
else:
    logging.info('Method B: Test passed not successfully.')
    logging.shutdown()
    sys.exit(1)

