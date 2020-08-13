#!/usr/bin/env python3
# Test results method I using a reference file
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
    logging.info("===== Method D - Global IFD =========================")

    data_1_filename = 'reference_data_Method_D_global_IFD.dat'
    data_1 = np.loadtxt(data_1_filename, usecols=(0, 1, 2, 3, 4, 5, 6))

    data_2_filename = os.path.join('./Output',
                                   'Fundamental_Diagram',
                                   'IndividualFD',
                                   'IFD_D_%s_id_-1.dat' % trajfile
                                   )
    if not os.path.exists(data_2_filename):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    data_2 = np.loadtxt(data_2_filename, usecols = (0,1,2,3,4,5,6))

    Frame_ks_statistic, Frame_p_value = ks_2samp(data_2[:, 0],
                                                 data_1[:, 0])
    PersID_ks_statistic, PersID_p_value = ks_2samp(data_2[:, 1],
                                                   data_1[:, 1])
    x_ks_statistic, x_p_value = ks_2samp(data_2[:, 2],
                                         data_1[:, 2])
    y_ks_statistic, y_p_value = ks_2samp(data_2[:, 3],
                                         data_1[:, 3])
    z_ks_statistic, z_p_value = ks_2samp(data_2[:, 4],
                                         data_1[:, 4])
    density_ks_statistic, density_p_value = ks_2samp(data_2[:, 5],
                                                     data_1[:, 5])
    velocity_ks_statistic, velocity_p_value = ks_2samp(data_2[:, 6],
                                                       data_1[:, 6])

    if Frame_p_value > alpha:
        logging.info('--> Frame: same dist -- p-value: {0:.4f}'.
                     format(Frame_p_value))
        Frame_exit = True
    else:
        logging.critical('--> Frame: different dist -- p-value: {0:.4f}'.
                     format(Frame_p_value))
        Frame_exit = False

    if PersID_p_value > alpha:
        logging.info('--> PersID: same dist -- p-value: {0:.4f}'.
                     format(PersID_p_value))
        PersID_exit = True
    else:
        logging.critical('--> PersID: different dist -- p-value: {0:.4f}'.
                     format(PersID_p_value))
        PersID_exit = False
        
    if x_p_value > alpha:
        logging.info('--> x-coordinate: same dist -- p-value: {0:.4f}'.
                     format(x_p_value))
        x_exit = True
    else:
        logging.critical('--> x-coordinate: different dist -- p-value: {0:.4f}'.
                     format(x_p_value))
        x_exit = False
        
    if y_p_value > alpha:
        logging.info('--> y-coordinate: same dist -- p-value: {0:.4f}'.
                     format(y_p_value))
        y_exit = True
    else:
        logging.critical('--> y-coordinate: different dist -- p-value: {0:.4f}'.
                     format(y_p_value))
        y_exit = False
        
    if z_p_value > alpha:
        logging.info('--> z-coordinate: same dist -- p-value: {0:.4f}'.
                     format(z_p_value))
        z_exit = True
    else:
        logging.critical('--> z-coordinate: different dist -- p-value: {0:.4f}'.
                     format(z_p_value))
        z_exit = False
        
    if density_p_value > alpha:
        logging.info('--> density: same dist -- p-value: {0:.4f}'.
                     format(density_p_value))
        density_exit = True
    else:
        logging.critical('--> density: different dist -- p-value: {0:.4f}'.
                     format(density_p_value))
        density_exit = False

    if velocity_p_value > alpha:
        logging.info('--> velocity: same dist -- p-value: {0:.4f}'.
                     format(velocity_p_value))
        velocity_exit = True
    else:
        logging.critical('--> velocity: different dist -- p-value: {0:.4f}'.
                     format(velocity_p_value))
        velocity_exit = False


    success = Frame_exit and \
      PersID_exit and \
      x_exit and \
      y_exit and \
      z_exit and \
      velocity_exit and \
      density_exit
    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
