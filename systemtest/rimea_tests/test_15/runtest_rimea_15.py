#!/usr/bin/env python
"""
Test description
================
In this test there are 3 rooms:
- Long corridor
- Short corridor
- Corner

The test shows the influence of the corner on the pedestrian movement. The
evacuation time of the corner should be in the middle of the long and short
corridor.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""


import numpy as np
import os
import sys

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *
import time
import matplotlib.pyplot as plt
import os

def run_rimea_test15(inifile, trajfile):
    #read data
    data_long = np.loadtxt('flow_exit_id_0_traj.txt')
    data_short = np.loadtxt('flow_exit_id_1_traj.txt')
    data_corner = np.loadtxt('flow_exit_id_2_traj.txt')

    # Evac times
    evac_long = data_long[-1,0]
    evac_short = data_short[-1,0]
    evac_corner = data_corner[-1,0]

    if evac_long < evac_corner or evac_short > evac_corner:
        logging.info("%s exits with FAILURE" % (argv[0]))
        logging.info("Corner evac_time not in between long and short corridor evac_time")
        logging.info("Long corridor evac_time: %f", evac_long)
        logging.info("Short corridor evac_time: %f", evac_short)
        logging.info("Corner evac_time: %f", evac_corner)
        exit(FAILURE)

    logging.info("Long corridor evac_time: %f", evac_long)
    logging.info("Short corridor evac_time: %f", evac_short)
    logging.info("Corner evac_time: %f", evac_corner)

def get_data (number):
    time=[]
    textfile=open("flow_exit_id_"+str(number)+".txt")
    for i in range(3):
        textfile.readline()
    for line in textfile:
        line=line.split(' ')
        if len(line) == 3:
            time.append(float(line[0]))
    return time

def get_dia_data (number):
    time=[]
    textfile=open("flow_exit_id_"+str(number)+"_traj.txt")
    for i in range(3):
        textfile.readline()
    for line in textfile:
        line=line.split(' ')
        if len(line) == 3:
            time.append(float(line[0]))
    return time

def create_dia (peds):

    x=range(1,peds+1,1)

#    0 = exit long
#   1 = exit short
#   2 = exit corner

    plt.plot(x,get_dia_data(2), 'b-', label='corner')
    plt.plot(x,get_dia_data(1), 'r-', label='short')
    plt.plot(x,get_dia_data(0), 'g-', label='long')
    plt.xlabel('peds')
    plt.ylabel('time[sec]')
    plt.grid()
    plt.xlim(0,peds)
    plt.legend(loc=0)
    plt.savefig('dia_test_15.png', dpi=100)
    print ("diagram created")
    return ('none')

if __name__ == "__main__":
    start_time=time.time()
    test = JPSRunTestDriver(15, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test15)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)

create_dia(500)
