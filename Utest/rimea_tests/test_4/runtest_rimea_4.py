#!/usr/bin/env python
"""
Test description
================
Distribute 0.5 - 6 ped / m^2 within a corridor

Track the flow of each distribution and return the fundamental diagram for
three measuring points (2 m x 1 m). One measuring point is considered to be
the main measure point and two are considered to be control measure points.

M -  Main measuring    located at   x: 12 - 14 / y: 0.5 - 1.5
C1 - Control measuring located at   x: 12 - 14 / y:   0 -   1
C2 - Control measuring located at   x:  0 -  2 / y: 0.5 - 1.5

 ____________ 26m ___________
|                            |
|C2            M             | 2m
|              C1            |
******************************

The tests simulates 70 s, but starts measuring after 10 s. (Check Rimea)

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

It is to be considered, that the peds wont be evacuated, because of the
periodic boundary conditions.

The geometry is way smaller than stated in Rimea, because we use
peridic boudary conditions:

Rimea:    1000x10

We use:   26x2

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""


import matplotlib
import numpy as np

matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
import glob
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *
import time

def run_rimea_test4(inifile, trajfile):
    # Catch all Traji-files
    files = glob.glob("trajectories/*")

    files.sort()

    density =  [2.0,3.0,4.0,0.5,5.0,6.0,1.0]

    # Main measuring point
    flow_Main = []
    v_mean_Main = []

    # Control measruing point 1
    flow_Control1 = []
    v_mean_Control1 = []

    # Control measruing point 2
    flow_Control2 = []
    v_mean_Control2 = []

    if len(files) == len(density): # Avoiding bug
        for f in files:
            print('Parsing file: %s'%(f))
            v_buffer_Main = []
            v_buffer_Control1 = []
            v_buffer_Control2 = []

            # Read data
            fps, N, traj = parse_file(f)

            peds = np.unique(traj[:,0])

################## Main measuring point #######################################

            for ped in peds:
                # Take only data of ped i
                ptraj = traj[traj[:, 0] == ped]
                #Take only data of ped i, when fps >= 80 (10 s simulation)
                ptraj = ptraj[ptraj[:,1]>=160]
                # Take only data of ped i, when he is inside measuring point: 12 <= x <= 14 (geometry from 0 - 26)
                ptraj = ptraj[ptraj[:,2]>=12]
                ptraj = ptraj[ptraj[:,2]<=14]
                # Take only data of ped i, when he is inside measuring point: 0.5 <= y <= 1.5 (geometry from 0 - 2)
                ptraj = ptraj[ptraj[:,3]>=0.5]
                ptraj = ptraj[ptraj[:,3]<=1.5]

                if len(ptraj) == 0:
                    continue # If ped will never be on measuring point --> next ped

                # Velocity of ped i
                v = np.zeros_like(ptraj[1:,2])
                v[:] = (ptraj[1:,2] - ptraj[:-1,2])/(ptraj[1:,1] - ptraj[:-1,1])*fps

                for i in v:
                    if i >= 0 and i < 1.5: # v needs to be inbetween 0 and 1.5 m/s
                        v_buffer_Main.append(i)

            # Average velocity for every density ---> Calculate flow from this one
            v_mean_Main.append(np.mean(v_buffer_Main))

################## Control1 measuring point ###################################

            for ped in peds:
                # Take only data of ped i
                ptraj = traj[traj[:, 0] == ped]
                #Take only data of ped i, when fps >= 80 (10 s simulation)
                ptraj = ptraj[ptraj[:,1]>=160]
                # Take only data of ped i, when he is inside measuring point: 12 <= x <= 14 (geometry from 0 - 26)
                ptraj = ptraj[ptraj[:,2]>=12]
                ptraj = ptraj[ptraj[:,2]<=14]
                # Take only data of ped i, when he is inside measuring point: 0 <= y <= 1 (geometry from 0 - 2)
                ptraj = ptraj[ptraj[:,3]>=0]
                ptraj = ptraj[ptraj[:,3]<=1]

                if len(ptraj) == 0:
                    continue # If ped will never be on measuring point --> next ped

                # Velocity of ped i
                v = np.zeros_like(ptraj[1:,2])
                v[:] = (ptraj[1:,2] - ptraj[:-1,2])/(ptraj[1:,1] - ptraj[:-1,1])*fps

                for i in v:
                    if i >= 0 and i < 1.5: # v needs to be inbetween 0 and 1.5 m/s
                        v_buffer_Control1.append(i)

            # Average velocity for every density ---> Calculate flow from this one
            v_mean_Control1.append(np.mean(v_buffer_Control1))


################## Control2 measuring point ###################################

            for ped in peds:
                # Take only data of ped i
                ptraj = traj[traj[:, 0] == ped]
                #Take only data of ped i, when fps >= 80 (10 s simulation)
                ptraj = ptraj[ptraj[:,1]>=160]
                # Take only data of ped i, when he is inside measuring point: 0 <= x <= 2 (geometry from 0 - 26)
                ptraj = ptraj[ptraj[:,2]>=0]
                ptraj = ptraj[ptraj[:,2]<=2]
                # Take only data of ped i, when he is inside measuring point: 0.5 <= y <= 1.5 (geometry from 0 - 2)
                ptraj = ptraj[ptraj[:,3]>=0.5]
                ptraj = ptraj[ptraj[:,3]<=1.5]

                if len(ptraj) == 0:
                    continue # If ped will never be on measuring point --> next ped

                # Velocity of ped i
                v = np.zeros_like(ptraj[1:,2])
                v[:] = (ptraj[1:,2] - ptraj[:-1,2])/(ptraj[1:,1] - ptraj[:-1,1])*fps

                for i in v:
                    if i >= 0 and i < 1.5: # v needs to be inbetween 0 and 1.5 m/s
                        v_buffer_Control2.append(i)

            # Average velocity for every density ---> Calculate flow from this one
            v_mean_Control2.append(np.mean(v_buffer_Control2))

        #Getting the flows:
        #flow_Main:
        for i in range(len(v_mean_Main)):
            flow_Main.append(v_mean_Main[i]*density[i])

        #flow_Control1:
        for i in range(len(v_mean_Control1)):
            flow_Control1.append(v_mean_Control1[i]*density[i])

        #flow_Control2:
        for i in range(len(v_mean_Control2)):
            flow_Control2.append(v_mean_Control2[i]*density[i])

        #Plot fundamental diagramm:
        plt.subplot(311)
        plt.plot(density, flow_Main, 'bo', label='Main')
        plt.grid()
        plt.xlim(0,7)
        plt.ylim(0,np.max(flow_Main)+1)
        plt.ylabel('flow')
        plt.legend(loc=0)

        plt.subplot(312)
        plt.plot(density, flow_Control1, 'bo', label='Control1')
        plt.grid()
        plt.xlim(0,7)
        plt.ylim(0,np.max(flow_Main)+1)
        plt.ylabel('flow')
        plt.legend(loc=0)

        plt.subplot(313)
        plt.plot(density, flow_Control2, 'bo', label='Control2')
        plt.grid()
        plt.xlim(0,7)
        plt.ylim(0,np.max(flow_Main)+1)
        plt.xlabel('density')
        plt.ylabel('flow')
        plt.legend(loc=0)
        plt.savefig('fundamental_diag.png')

        #Write data in txt:
        txt_file = open('fundamental_diag.txt','w')
        txt_file.write('#density    flow_Main    flow_Control1    flow_Control2\n')
        for i in range(len(density)):
            txt_file.write(str(density[i]) + '    ' + str(flow_Main[i]) + '    ' + str(flow_Control1[i]) + '    ' + str(flow_Control2[i]) + '\n')
        txt_file.close()

if __name__ == "__main__":
    start_time=time.time()
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test4)
    logging.info("%s exits with SUCCESS\nExecution time %.3f seconds." % (argv[0],time.time()-start_time))
    exit(SUCCESS)
