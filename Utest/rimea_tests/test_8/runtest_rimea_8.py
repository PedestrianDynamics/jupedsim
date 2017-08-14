#!/usr/bin/env python3
"""
Test description
================
A 3D building is simulated and the influence of the parameters speed and tau is investigated.
It should be shown how the evacuation time behaves with respect to the investigated parameter.

Remarks
=======
Use this code with python 2
Use new dedicated python console if you run this code with spyder

There is no fail criterion in this test. Just documentation.

Source
======
http://www.rimea.de/fileadmin/files/dok/richtlinien/RiMEA_Richtlinie_3.0.0_-_D-E.pdf
"""

import os
import sys
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
import glob
import matplotlib.pyplot as plt
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import *


def run_rimea_test8(inifile, trajfile):
    v0_mean = [1.0,1.25,0.75] # Used in master_ini
    tau_mean = [0.3,0.5,0.7] # Used in master_ini
    
    # Catch all Traj-files
    files = glob.glob("trajectories/*_exit_id_0*")
    
    v0_list = []
    tau_list = []
    evac_time_list = []
    
    if len(files) == len(v0_mean)*len(tau_mean): # Avoiding bug
        for f in files:
            data = np.loadtxt(f)
        
            if len(data) == 0:
                logging.critical("File %s is empty", f)
                logging.critical("%s exists with failure!", argv[0])
                exit(FAILURE)
        
            evac_num = data[-1,1]
            
            evac_time = data[-1,0]
            evac_time_list.append(evac_time)
            
            v0 = float(f.split("_v0-mu_")[1].split("_tau-mu_")[0])
            v0_list.append(v0)
            
            tau = float(f.split("_tau-mu_")[1].split(".xml_")[0])
            tau_list.append(tau)
            
            logging.info("%d peds evacuated from exit <%s>. Evac_time: %f, v0: %s tau: %s", evac_num, f.split(".dat")[0].split("_id_")[-1], evac_time, v0, tau)
        
        # Plotting
        logging.info("Ploting graph...")
        figname = 'parameter.png'
        if os.path.isfile(figname):
            os.remove(figname)
        i = 1
        f = plt.figure()
        for v0 in np.unique(v0_list):
            plt.subplot('%d1%d'%(len(np.unique(v0_list)),i))
            plt.plot(np.array(tau_list)[v0_list==v0],np.array(evac_time_list)[v0_list==v0],'bo')
            plt.title('v0 = %f'%v0)
            plt.ylabel('Evac time [s]')
            plt.xlim([0,1])
            plt.ylim([0,np.max(evac_time_list)+100])
            plt.grid()
            i = i + 1
        plt.xlabel('tau')
        plt.tight_layout()
        plt.savefig(figname)

if __name__ == "__main__":
    test = JPSRunTestDriver(8, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=run_rimea_test8)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
