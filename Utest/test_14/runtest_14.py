#! /usr/bin/env python
import numpy as np
import os, argparse, logging, time, sys
from os import path, system
from sys import argv ,exit
import subprocess, glob
import multiprocessing
import matplotlib.pyplot as plt
import re
import scipy
import scipy.stats
#=========================
testnr = 14
#========================

SUCCESS = 0
FAILURE = 1

#--------------------------------------------------------
logfile="log_test_%d.txt"%testnr
f=open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
DIR= os.path.dirname(os.path.realpath(argv[0]))
CWD = os.getcwd()
#--------------------------------------------------------
    
OK = 1
PX = []  #p-value for x
PY = []  #p-value for y
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
        maxtime = get_maxtime(inifile)
        fps, N, data = parse_file(trajfile)
        data = data[ data[:,1] == 0  ]

        x = data[:,2]
        y = data[:,3]

        nx = plt.hist(x, bins=10)[0]
        ny = plt.hist(y, bins=10)[0]

        px = scipy.stats.chisquare(nx)[1]
        py = scipy.stats.chisquare(ny)[1]

        PX.append(px)
        PY.append(py)


    plt.subplot(211)
    plt.plot(PX, label="PX")
    plt.ylabel("px")
    plt.subplot(212)
    plt.plot(PY, label="PY")
    plt.ylabel("py")
    plt.savefig("px_py.png")

        
    if np.mean(PX) < 0.1 or np.mean(PY)< 0.1:
        logging.info("%s exits with FAILURE PX = %f   PY = %f"%(argv[0], np.mean(px), np.mean(py)))
        OK = 0
        exit(FAILURE)
        
    else:
        logging.info("PX = %f  PY = %f"%( np.mean(px), np.mean(py)))
