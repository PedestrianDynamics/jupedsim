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

logging.info("===== Method D =========================")

data_1_filename = 'reference_data_Method_2_D.dat'
data_1 = np.loadtxt(data_1_filename)

data_2_filename = './Output_Methods_2/Fundamental_Diagram/Classical_Voronoi/rho_v_Voronoi_traj_Methods.txt_id_5.dat'
data_2 = np.loadtxt(data_2_filename)

# Compute the Kolmogorov-Smirnov statistic on 2 samples.
alpha = 0.95
density_ks_statistic, density_p_value = ks_2samp(data_2[:,1],data_1[:,1])
velocity_ks_statistic, velocity_p_value = ks_2samp(data_2[:,2],data_1[:,2])

# numpy - True if two arrays have the same shape and elements, False otherwise.
data_equal = np.array_equal(data_1, data_2)

# check
if data_equal == True:
    logging.info('--> The data are equal.'.format(density_p_value))
    data_exit = True
else:
    data_exit = False

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

# result
if data_exit == True:
    logging.info('Method D: Test passed successfully.')
    logging.shutdown()
    sys.exit(0)
else:
    if (velocity_exit == True) and (density_exit == True):
        logging.info('Method D: Test passed successfully.')
        logging.shutdown()
        sys.exit(0)
    else:
        logging.info('Method D: Test passed not successfully.')
        logging.shutdown()
        sys.exit(1)
