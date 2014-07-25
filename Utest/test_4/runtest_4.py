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
testnr = 4
#========================

must_time = 10  # 10 m corridor with 1m/s 
SUCCESS = 0
FAILURE = 1
#--------------------------------------------------------
logfile="log_test_%d.txt"%testnr
f=open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
CWD = os.getcwd()
DIR = os.path.dirname(os.path.realpath(argv[0]))
#--------------------------------------------------------
    


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
    inifiles = glob.glob("inifiles/*.xml")
    if not path.exists(geofile):
        logging.critical("geofile <%s> does not exist"%geofile)
        exit(FAILURE)
        
    executable = "%s/bin/jpscore"%TRUNK
    if not path.exists(executable):
        logging.critical("executable <%s> does not exist yet."%executable)
        exit(FAILURE)
        
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
        #--------------------- PARSING & FLOW-MEASUREMENT --------
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exist"%trajfile)
            exit(FAILURE)
  
        fps, N, traj = parse_file(trajfile)
        traj_1 = traj[ traj[:,0] == 1 ]
        x_1 = traj_1[:,2]
        y_1 = traj_1[:,3]

        x_2 = traj[ traj[:,0] == 2 ][:,2]
        y_2 = traj[ traj[:,0] == 2 ][:,3]

        eps = 0.3 # 10 cm
        x_min = x_2[0] - eps
        x_max = x_2[0] + eps
        y_min = y_2[0] - eps
        y_max = y_2[0] + eps

        lx = np.logical_and( x_1 > x_min, x_1 < x_max )
        ly = np.logical_and( y_1 > y_min, y_1 < y_max )

        overlap = (lx*ly).any()
        
        
        if overlap:
            logging.info("%s exits with FAILURE "%argv[0])
            exit(FAILURE)
         
    logging.info("%s exits with SUCCESS"%(argv[0]))
    exit(SUCCESS)
