#!/usr/bin/env python3
# ---------
# Basic test setup:
# Pedestrians are moving in a 6x6 grid in x-direction with v=1 m/s
# They have a fixed horizontal and vertical distance (ped_distance)
# Due to the alignment the inner voronoi cells should be squares
# The inner voronoi cells should lay entirely in the specified square measurement area for frame 109 (after a travelled distance of 9m)
# For method I a 1x1 grid scenario was implemented to test the voronoi cell calculation with blind points
# ---------

import os
from sys import argv, path
import logging
import math

path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
import numpy as np


# ---------
# Test of classical voronoi output for Method D and J (use of blind points disabled)
# Real voronoi density can be calculated by 1 person / (ped_distance^2)
# Real velocity is 1.0 m/s for all frames (since cut off is false parts of the outer voronoi cells lay in the measurement area)
# ---------

def test_classical_voronoi(trajfile, ped_distance, file_extension=""):

    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'Classical_Voronoi',
                                         'rho_v_Voronoi_%s%s_id_1.dat' % (file_extension, trajfile)
                                         )

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # real density can be calculated based on the square/distance to each other
    real_density = 1 / (ped_distance ** 2)
    # real velocity must remain the same
    real_velocity = 1
    # accepted error
    acceptance_range = 0.001
    # frame when inner voronoi cells of 16 people are entirely in the measurement area
    frame = 109

    jpsreport_data = np.loadtxt(jpsreport_result_file)

    # check density for the specified frame
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


# ---------
# Test of IFD output for Method D and J (use of blind points disabled)
# IDs of pedestrians that are in the measurement area for frame 109 are checked
# Their individual density must be 1 person / (ped_distance^2)
# ---------
def test_IFD(method, trajfile, ped_distance):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'IndividualFD',
                                         'IFD_%s_%s_id_1.dat' % (method, trajfile)
                                         )

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # real density can be calculated based on the square/distance to each other
    real_density = 1 / (ped_distance ** 2)
    # accepted error
    acceptance_range = 0.001
    # frame when inner voronoi cells of 16 people are entirely in the measurement area
    frame = 109
    # ids of pedestrians whose voronoi cells are entireley in the measurement for the upper frame
    ped_IDs = np.array([8.0, 9.0, 10.0, 11.0, 14.0, 15.0, 16.0, 17.0, 20.0, 21.0, 22.0, 23.0, 26.0, 27.0, 28.0, 29.0])

    jpsreport_data = np.loadtxt(jpsreport_result_file, usecols=(0, 1, 2, 3, 4, 5))

    # get ids that are in the measurement area for the specified frame
    jpsreport_IDs = jpsreport_data[jpsreport_data[:, 0] == frame][:, 1]
    jpsreport_IDs_sorted = np.sort(jpsreport_IDs)
    real_ped_IDs_sorted = np.sort(ped_IDs)

    if np.array_equal(jpsreport_IDs_sorted, real_ped_IDs_sorted):
        logging.info("pedestrians in measurement area should be OK.")
    else:
        logging.critical(
            "IDs of pedestrians in measurement area did not match result. Got {}. Expected {}".format(
                jpsreport_IDs_sorted,
                real_ped_IDs_sorted))
        exit(FAILURE)

    # check density for for pedestrians in measurement area for specified frame
    jpsreport_density = jpsreport_data[jpsreport_data[:, 0] == frame][:, 5]
    real_density_array = np.ones(np.size(jpsreport_density)) * real_density

    if np.all(np.abs(jpsreport_density - real_density_array) < acceptance_range):
        logging.info("density calculation should be OK.")
    else:
        logging.critical(
            "density values did not match result. Got {}. Expected {}".format(jpsreport_density, real_density_array))
        exit(FAILURE)


# ---------
# Test of global IFD output for Method D (use of blind points enabled)
# Individual density for inner pedestrians must be 1 person / (ped_distance^2) for all frames
# ---------
def test_IFD_all_frames(trajfile, ped_distance):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'IndividualFD',
                                         'IFD_D_%s_id_-1.dat' % trajfile)

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # real density can be calculated based on the square/distance to each other
    real_density = 1 / (ped_distance ** 2)
    # accepted error
    acceptance_range = 0.001
    
    jpsreport_data = np.loadtxt(jpsreport_result_file, usecols=(0, 1, 2, 3, 4, 5))

    # ids of inner pedestrians
    ped_IDs = np.array([8.0, 9.0, 10.0, 11.0, 14.0, 15.0, 16.0, 17.0, 20.0, 21.0, 22.0, 23.0, 26.0, 27.0, 28.0, 29.0])

    # check density for pedestrians for all frames
    jpsreport_density = jpsreport_data[np.isin(jpsreport_data[:, 1], ped_IDs)][:, 5]
    real_density_array = np.ones(np.size(jpsreport_density)) * real_density

    if np.all(np.abs(jpsreport_density - real_density_array) < acceptance_range):
        logging.info("density calculation should be OK.")
    else:
        logging.critical(
            "density values did not match result. Got {}. Expected {} for all values".format(jpsreport_density,
                                                                                             real_density))
        exit(FAILURE)


# ---------
# Test of cut off option for Method D and J (use of blind points disabled)
# Densities of pedestrians that are in the measurement area for frame 109 are checked
# Cut off voronoi cells are approximated by circles
# If the area of the circle is smaller than the area of the square, the cut off option has an effect on the inner voronoi cells
# If the area of the circle is larger than the area of the square, the cut off option has no effect on the inner voronoi cells
# Both scenarios can be tested with this function by setting `cut_off_has_effect`
# ---------
def test_cut_off(method, trajfile, ped_distance, cut_off_has_effect=True):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'IndividualFD',
                                         'IFD_%s_%s_id_1.dat' % (method, trajfile)
                                         )

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # real density is based on the square for circumcircle or based on the circle for incircle
    if cut_off_has_effect:
        # cut off is applied: density is higher than for square voronoi cell.
        real_density = 1 / (math.pi * (ped_distance / 2) ** 2)
    else:
        # no cut off is applied: density remains the same
        real_density = 1 / (ped_distance ** 2)

    # accepted error
    acceptance_range = 0.001
    # frame when inner voronoi cells of 16 people are entirely in the measurement area
    frame = 109

    jpsreport_data = np.loadtxt(jpsreport_result_file, usecols=(0, 1, 2, 3, 4, 5))

    # check density for pedestrians in measurement area for specified frame
    jpsreport_density = jpsreport_data[jpsreport_data[:, 0] == frame][:, 5]
    real_density_array = np.ones(np.size(jpsreport_density)) * real_density  #

    if np.all(np.abs(jpsreport_density - real_density_array) < acceptance_range):
        logging.info("density calculation with cut off option should be OK.")
    else:
        logging.critical(
            "density values with cut off option did not match result. Got {}. Expected {}".format(jpsreport_density,
                                                                                                  real_density_array))
        exit(FAILURE)


# ---------
# Test velocity calculation with cut off option for Method J (use of blind points disabled)
# Voronoi cells are approximated by a circle with radius =0.75
# Velocity in MA should be 1 when vornoi cells are intersecting or 0 when no pedestrians are in MA
# Voronoi cells in the last column of the grid reach the MA at frame 19 (only one point on MA is not recognized as intersection)
# Voronoi cells in first column of the grid leave the MA at frame 199
# Note: Function is only applicable for one specified scenario since the shape of the voronoi cells needs to be known
# ---------
def test_cut_off_velocity(trajfile):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'Classical_Voronoi',
                                         'rho_v_Voronoi_J_%s_id_1.dat' %trajfile
                                         )

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # define frame range for which the velocity must be 1
    start_frame = 20
    end_frame = 198

    jpsreport_data = np.loadtxt(jpsreport_result_file)

    frames = np.arange(start_frame, end_frame+1)
    # check velocites for given frame range
    jpsreport_velocity = jpsreport_data[np.isin(jpsreport_data[:, 0], frames)][:, 2]

    if np.all(np.equal(jpsreport_velocity, np.ones(jpsreport_velocity.size))):
        logging.info("velocity calculation with cut off option when pedestirans are in the MA should be OK.")
    else:
        logging.critical(
            "velocity values with cut off option did not match result. Got {}. Expected {} for all values".format(jpsreport_velocity,
                                                                                                  1.0))
        exit(FAILURE)

    #check velocities outside given frame range
    jpsreport_velocity_before = jpsreport_data[jpsreport_data[:, 0]<start_frame][:, 2]
    jpsreport_velocity_after = jpsreport_data[jpsreport_data[:, 0]>end_frame][:, 2]
    jpsreport_velocity_outside = np.concatenate((jpsreport_velocity_before, jpsreport_velocity_after))

    if np.all(np.equal(jpsreport_velocity_outside, np.ones(jpsreport_velocity_outside.size)*0.0)):
        logging.info("velocity calculation with cut off option when no pedestrians are in the MA should be OK.")
    else:
        logging.critical(
            "velocity values with cut off option did not match result. Got {}. Expected {} for all values".format(jpsreport_velocity,
                                                                                                  0.0))
        exit(FAILURE)


# ---------
# Test of cut off option for Method D (global IFD, use of blind points enabled)
# Densities of inner pedestrians are checked for all frames
# Cut off voronoi cells are approximated by circles
# If the area of the circle is smaller than the area of the square, the cut off option has an effect on the inner voronoi cells
# If the area of the circle is larger than the area of the square, the cut off option has no effect on the inner voronoi cells
# Both scenarios can be tested with this function by setting `cut_off_has_effect`
# ped_IDs as parameter so that this test can be applied for the one person scenario as well and not for inner pedestrians only
# ---------
def test_cut_off_all_frames(trajfile, ped_distance, ped_IDs, cut_off_has_effect=True):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'IndividualFD',
                                         'IFD_D_%s_id_-1.dat' % trajfile)
    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # real density is based on the square for circumcircle or based on the circle for incircle
    if cut_off_has_effect:
        # cut off is applied: density is higher than for square voronoi cell.
        real_density = 1 / (math.pi * (ped_distance / 2) ** 2)
    else:
        # no cut off is applied: density remains the same
        real_density = 1 / (ped_distance ** 2)

    # accepted error
    acceptance_range = 0.001

    jpsreport_data = np.loadtxt(jpsreport_result_file, usecols=(0, 1, 2, 3, 4, 5))

    # check density for pedestrians in measurement area for all frames
    jpsreport_density = jpsreport_data[np.isin(jpsreport_data[:, 1], ped_IDs)][:, 5]
    real_density_array = np.ones(np.size(jpsreport_density)) * real_density

    if np.all(np.abs(jpsreport_density - real_density_array) < acceptance_range):
        logging.info("density calculation with cut off option should be OK.")
    else:
        logging.critical(
            "density values with cut off option did not match result. Got {}. Expected {} for all values".format(
                jpsreport_density,
                real_density))
        exit(FAILURE)


# ---------
# Test of IFD output when intersecting with geometry for Method D and J (use of blind points disabled)
# Corridor is choosen more narrow so that voronoi cells next to walls should intersect with the geometry
# Measurement area contains first and last row of these pedestrians
# Their individual density must be 1 person / (area of the rectangle)
# Note: Function is only applicable for one specified scenario since the distance to the geometry needs to be known
# ---------
def test_IFD_geometry_intersection(method, trajfile):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'IndividualFD',
                                         'IFD_%s_%s_id_1.dat' % (method, trajfile)
                                         )

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # accepted error
    acceptance_range = 0.001
    # frame when inner voronoi cells of 16 people are entirely in the measurement area
    frame = 109

    # density can be calculated based on the geometry and the corresponding rectangle
    real_density_first_row = 1 / (1.25 * 1.5)
    real_density_last_row = 1 / 1.5

    jpsreport_data = np.loadtxt(jpsreport_result_file, usecols=(0, 1, 2, 3, 4, 5))

    # get data for specified frame
    jpsreport_frame_data = jpsreport_data[jpsreport_data[:, 0] == frame][:, :]

    # get density data for first and last row
    jpsreport_density_first_row = jpsreport_frame_data[jpsreport_frame_data[:, 1] < 6][:,
                                  5]  # ped IDs of first row in measurement area: 2,3,4,5
    jpsreport_density_last_row = jpsreport_frame_data[jpsreport_frame_data[:, 1] > 31][:,
                                 5]  # ped IDs of last row in measurement area: 32,33,34,35

    real_density_array_first_row = np.ones(np.size(jpsreport_density_first_row)) * real_density_first_row
    real_density_array_last_row = np.ones(np.size(jpsreport_density_first_row)) * real_density_last_row

    if np.all(np.abs(jpsreport_density_first_row - real_density_array_first_row) < acceptance_range):
        logging.info("density calculation when intersecting geometry should be OK.")
    else:
        logging.critical(
            "density values for pedestrians in first row did not match result. Got {}. Expected {}".format(
                jpsreport_density_first_row, real_density_array_first_row))
        exit(FAILURE)

    if np.all(np.abs(jpsreport_density_last_row - real_density_array_last_row) < acceptance_range):
        logging.info("density calculation when intersecting geometry should be OK.")
    else:
        logging.critical(
            "density values for pedestrians in last row did not match result. Got {}. Expected {}".format(
                jpsreport_density_last_row, real_density_array_last_row))
        exit(FAILURE)


# ---------
# Test of global IFD output when intersecting with geometry for Method D (use of blind points enabled)
# Corridor is chosen more narrow so that voronoi cells next to walls should intersect with the geometry
# Density of pedestrians in first and last rows is checked for all frames
# Their individual density must be 1 person / (area of the rectangle)
# Note: Function is only applicable for one specified scenario since the distance to the geometry needs to be known
# ---------
def test_IFD_geometry_intersection_all_frames(trajfile):
    jpsreport_result_file = os.path.join('./Output',
                                         'Fundamental_Diagram',
                                         'IndividualFD',
                                         'IFD_D_%s_id_-1.dat' % trajfile)

    if not os.path.exists(jpsreport_result_file):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # set test configuration
    # accepted error
    acceptance_range = 0.001

    # density can be calculated based on the geometry and the corresponding rectangle
    real_density_first_row = 1 / (1.25 * 1.5)
    real_density_last_row = 1 / 1.5

    jpsreport_data = np.loadtxt(jpsreport_result_file, usecols=(0, 1, 2, 3, 4, 5))

    # ids of first and last row
    ped_IDs_first_row = np.array([2.0, 3.0, 4.0, 5.0])
    ped_IDs_last_row = np.array([32.0, 33.0, 34.0, 35.0])

    # get density data for first and last row for all frames
    jpsreport_density_first_row = jpsreport_data[np.isin(jpsreport_data[:, 1], ped_IDs_first_row)][:, 5]
    jpsreport_density_last_row = jpsreport_data[np.isin(jpsreport_data[:, 1], ped_IDs_last_row)][:, 5]

    real_density_array_first_row = np.ones(np.size(jpsreport_density_first_row)) * real_density_first_row
    real_density_array_last_row = np.ones(np.size(jpsreport_density_first_row)) * real_density_last_row

    if np.all(np.abs(jpsreport_density_first_row - real_density_array_first_row) < acceptance_range):
        logging.info("density calculation when intersecting geometry should be OK.")
    else:
        logging.critical(
            "density values for pedestrians in first row did not match result. Got {}. Expected {} for all values".format(
                jpsreport_density_first_row, real_density_first_row))
        exit(FAILURE)

    if np.all(np.abs(jpsreport_density_last_row - real_density_array_last_row) < acceptance_range):
        logging.info("density calculation when intersecting geometry should be OK.")
    else:
        logging.critical(
            "density values for pedestrians in last row did not match result. Got {}. Expected {} for all values".format(
                jpsreport_density_last_row, real_density_last_row))
        exit(FAILURE)
