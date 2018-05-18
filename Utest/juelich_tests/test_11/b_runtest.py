#! /usr/bin/env python
import glob
import logging
import numpy as np
import os
import subprocess
import sys
from os import path
from sys import argv, exit
from xml.dom import minidom

#=========================
testnr = 11
#========================

must_time = 10  # 10 m corridor with 1m/s 
SUCCESS = 0
FAILURE = 1
#--------------------------------------------------------
logfile="log_test_11b.txt"
f=open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
CWD = os.getcwd()
DIR = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
#--------------------------------------------------------
    
def get_maxtime(filename):
    """
    get max sim time
    """
    logging.info("parsing <%s>"%filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file. exit')
        exit(FAILURE)        
    maxtime  = float(xmldoc.getElementsByTagName('max_sim_time')[0].firstChild.nodeValue)
    return maxtime



if __name__ == "__main__":
    juelich_dir = DIR  + os.sep + "juelich_tests" + os.sep + "test_11"
    geofile =  "%s/geometry/geometry_test%d_b.xml"%(juelich_dir,testnr)
    inifiles = glob.glob("inifiles_b/*.xml")
    if not path.exists(geofile):
        logging.critical("geofile <%s> does not exist"%geofile)
        exit(FAILURE)
       
    #-------- get directory of the code TRUNK
    os.chdir(DIR)
    os.chdir("..")
    TRUNK = os.getcwd()
    os.chdir(DIR)
    lib_path = os.path.abspath("%s/Utest"%TRUNK)
    sys.path.append(lib_path)
    from utils import *
    #---------------------------------------- 
    executable = "%s/bin/jpscore"%TRUNK
    if not path.exists(executable):
        logging.critical("executable <%s> does not exist yet."%executable)
        exit(FAILURE)
    evac_time = []
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
        fps, N, traj = parse_file(trajfile)
        etime =  ( max( traj[:,1] ) - min( traj[:,1] ) ) / float(fps)
        evac_time.append( etime )

        logging.info("%s -- evac time %.2f"%(argv[0], etime))

    logging.info("mean = %.2f ; std = %.2f "%(np.mean(evac_time), np.std(evac_time)))
    exit(np.mean(evac_time))
