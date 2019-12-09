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

logging.info("===== Method D - IFD ===================")

data_1_filename = 'reference_data_Method_D_IFD.dat'
data_1 = np.loadtxt(data_1_filename, usecols = (0,1,2,3,4,5,6))

data_2_filename = './Output_Methods/Fundamental_Diagram/IndividualFD/IFD_D_traj_Methods.txt_id_5.dat'
data_2 = np.loadtxt(data_2_filename, usecols = (0,1,2,3,4,5,6))

# test
alpha = 0.01

Frame_ks_statistic, Frame_p_value = ks_2samp(data_2[:,0],data_1[:,0])
PersID_ks_statistic, PersID_p_value = ks_2samp(data_2[:,1],data_1[:,1])
x_ks_statistic, x_p_value = ks_2samp(data_2[:,2],data_1[:,2])
y_ks_statistic, y_p_value = ks_2samp(data_2[:,3],data_1[:,3])
z_ks_statistic, z_p_value = ks_2samp(data_2[:,4],data_1[:,4])
density_ks_statistic, density_p_value = ks_2samp(data_2[:,5],data_1[:,5])
velocity_ks_statistic, velocity_p_value = ks_2samp(data_2[:,6],data_1[:,6])

if Frame_p_value > alpha:
    logging.info('--> Frame: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(Frame_p_value))
    Frame_exit = True
else:
    logging.info('--> Frame: different dist (reject H0) -- p-value: {0:.4f}'.format(Frame_p_value))
    Frame_exit = False

if PersID_p_value > alpha:
    logging.info('--> PersID: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(PersID_p_value))
    PersID_exit = True
else:
    logging.info('--> PersID: different dist (reject H0) -- p-value: {0:.4f}'.format(PersID_p_value))
    PersID_exit = False

if x_p_value > alpha:
    logging.info('--> x-coordinate: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(x_p_value))
    x_exit = True
else:
    logging.info('--> x-coordinate: different dist (reject H0) -- p-value: {0:.4f}'.format(x_p_value))
    x_exit = False

if y_p_value > alpha:
    logging.info('--> y-coordinate: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(y_p_value))
    y_exit = True
else:
    logging.info('--> y-coordinate: different dist (reject H0) -- p-value: {0:.4f}'.format(y_p_value))
    y_exit = False

if z_p_value > alpha:
    logging.info('--> z-coordinate: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(z_p_value))
    z_exit = True
else:
    logging.info('--> z-coordinate: different dist (reject H0) -- p-value: {0:.4f}'.format(z_p_value))
    z_exit = False

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


if (Frame_exit == True) and (PersID_exit == True) and (x_exit == True) and (y_exit == True) and (z_exit == True) and (velocity_exit == True) and (density_exit == True):
    logging.info('Method D: IFD-Test passed successfully.')
    logging.shutdown()
    sys.exit(0)
else:
    logging.info('Method D: IFD-Test passed not successfully.')
    logging.shutdown()
    sys.exit(1)

