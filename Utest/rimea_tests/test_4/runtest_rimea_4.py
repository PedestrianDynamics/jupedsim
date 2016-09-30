#!/usr/bin/env python
"""
Test description
================
Distribute [0.5, 1, 2, 3, 4, 5, 6] ped / m^2 within a corridor

Track the flow of each distribution and return the fundamental diagram for
three measuring points (2 m x 2 m). The measuring points lay in the middle of
the corridor. One is considered to be the main measure point and two are
considered to be control measure points.

The tests simulates 70 s, but starts measuring after 10 s.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

It is to be considered, that the peds wont be evacuated, because of the
periodic boundary conditions.

The geometry is way smaller than Rimea wants it to be, because we use
peridic boudary conditions.

---> At the moment there is no measuring at control measure points.

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""


import numpy as np
import matplotlib.pyplot as plt
import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
import glob
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *

def run_rimea_test4(inifile, trajfile):
    # Catch all Traji-files
    files = glob.glob("trajectories/*")
    
    if len(files) == 0:
        logging.critical("%s exists with failure! Found no exit-files.", argv[0])
        exit(FAILURE)
    
    density = [1.0,2.0,3.0,4.0,5.0,6.0,0.5] # DO NOT CHANGE THE ORDER
    
    Flow = []
    v_mean = []
    
    if len(files) == len(density): # Avoiding SegFault
        for f in files:
            v_list = []
            
            # Read data
            fps, N, traj = np.loadtxt(f)
            logging.info("=== npeds: %d, fps: %d ===", N, fps)
            
            peds = np.unique(traj[:,0])
            
            for ped in peds:
                # Take only data of ped i
                ptraj = traj[traj[:, 0] == ped]
                #Take only data of ped i, when fps >= 80 (10 s simulation)
                ptraj = ptraj[ptraj[:,1]>=80]
                # Take only data of ped i, when he is inside measuring point: 12 <= x <= 14
                ptraj = ptraj[ptraj[:,2]>=12]
                ptraj = ptraj[ptraj[:,2]<=14]
                # Take only data of ped i, when he is inside measuring point: 1 <= y <= 3
                ptraj = ptraj[ptraj[:,3]>=1]
                ptraj = ptraj[ptraj[:,3]<=3]
                
                if len(ptraj) == 0:
                    continue
                
                v = np.zeros_like(ptraj[1:,2])
                v[:] = (ptraj[1:,2] - ptraj[:-1,2])/(ptraj[1:,1] - ptraj[:-1,1])*fps
                
                for i in v:
                    if i >= 0 and i < 1.5:
                        v_list.append(i)
                
            v_mean.append(np.mean(v_list))
        
        for i in range(len(v_mean)):
            Flow.append(v_mean[i]*density[i])
        
        plt.plot(density,Flow,'bo')
        plt.grid()
        plt.xlim(0,7)
        plt.ylim(0,6)
        plt.xlabel('density')
        plt.ylabel('flow')
        plt.savefig('fundamental_diag.png')
        plt.show()

if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test4)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)