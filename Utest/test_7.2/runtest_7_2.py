#! /usr/bin/env python
import numpy as np
from xml.dom import minidom
import os, argparse, logging, time
from os import path, system
from sys import argv ,exit
import subprocess, glob
import multiprocessing
import matplotlib.pyplot as plt
import re, sys

#=========================
testnr = 7.2
#========================

SUCCESS = 0
FAILURE = 1
#--------------------------------------------------------
logfile="log_test_%1.1f.txt"%testnr
f=open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
CWD = os.getcwd()
DIR = os.path.dirname(os.path.realpath(argv[0]))
#--------------------------------------------------------
def PassedLineX(p, exit):
    """
    check if pedestrian (given by matrix p) passed the vertical line x, [y1, y2] y1<y2
    """
    x = exit[0]
    y1 = exit[1]
    y2 = exit[2]
    return any(p[:,2] <= x) & any(p[:,2] >= x) & any(p[:,3] >= y1) & any(p[:,3] <= y2)
def PassedLineY(p, exit):
    """
    check if pedestrian (given by matrix p) passed the horizontal line y, [x1, x2] x1<x2
    """
    y = exit[0]
    x1 = exit[1]
    x2 = exit[2]
    return any(p[:,3] <= y) & any(p[:,3] >= y) & any(p[:,2] >= x1) & any(p[:,2] <= x2)
    
    
if __name__ == "__main__":
    if CWD != DIR:
        logging.info("working dir is %s. Change to %s"%(os.getcwd(), DIR))
        os.chdir(DIR)

    logging.info("change directory to ..")
    os.chdir("..")
    logging.info("call makeini.py with -f %s/master_ini.xml"%DIR)
    subprocess.call(["python", "makeini.py", "-f", "%s/master_ini.xml"%DIR])
    os.chdir(DIR)
    #-------- get directory of the code TRUNK
    os.chdir("../..")
    TRUNK = os.getcwd()
    os.chdir(DIR)
    lib_path = os.path.abspath("%s/Utest"%TRUNK)
    sys.path.append(lib_path)
    from utils import *
    #----------------------------------------
    logging.info("change directory back to %s"%DIR)
    geofile = "%s/geometry.xml"%DIR
    logging.info("geofile <%s>"%geofile)
    inifiles = glob.glob("inifiles/*.xml")
    if not path.exists(geofile):
        logging.critical("geofile <%s> does not exist"%geofile)
        exit(FAILURE)
        
    executable = "%s/bin/jpscore"%TRUNK
    if not path.exists(executable):
        logging.critical("executable <%s> does not exist yet."%executable)
        exit(FAILURE)

    # Exits
    e1 = [26, 1, 2] # y, x1, x2
    e2 = [18, 1, 2] # x, y1, y2
        
    for inifile in inifiles:
        if not path.exists(inifile):
            logging.critical("inifile <%s> does not exist"%inifile)
            exit(FAILURE)
        #--------------------- SIMULATION ------------------------  
        #os.chdir(TRUNK) #cd to the simulation directory      
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('start simulating with exe=<%s>'%(cmd))
        #------------------------------------------------------
        subprocess.call([executable, "--inifile=%s"%inifile])
        #------------------------------------------------------
        logging.info('end simulation ...\n--------------\n')
        trajfile = "trajectories/traj" + inifile.split("ini")[2]
        logging.info('trajfile = <%s>'%trajfile)
        #--------------------- PARSING & TEST LOGIC --------
        # this tests quickest router. Check if someone exists from e2
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exist"%trajfile)
            exit(FAILURE)
  
        fps, N, traj = parse_file(trajfile)
        peds = np.unique(traj[:,0]).astype(int)
        logging.info("Checking the exits of pedestrians ...")
        
        for ped in peds:
            traj1 = traj[ traj[:,0] == ped ]
            x = traj1[:,2]
            y = traj1[:,3]
            if PassedLineX(traj1, e2):
                logging.info("ped %d exits from Exit (%1.2f, %1.2f) | (%1.2f, %1.2f)"%(ped, e2[0], e2[1], e2[0], e2[2]))
                logging.info("%s exists with success!"%argv[0]) 
                exit(SUCCESS)
            
    #----- for inifiles    
    logging.info("%s exists with failure!"%argv[0]) 
    exit(FAILURE)

