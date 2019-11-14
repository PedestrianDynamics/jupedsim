#! /usr/bin/env python
import glob
import logging
import os
import subprocess
import sys
from os import path
from sys import argv, exit

#=========================
testnr = 1
#========================

must_time = 10  # 10 m corridor with 1m/s
SUCCESS = 0
FAILURE = 1

#--------------------------------------------------------
logfile = "log_test_%d.txt"%testnr
f = open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile,
                    level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
DIR = os.path.dirname(os.path.realpath(argv[0]))
CWD = os.getcwd()
#--------------------------------------------------------


if __name__ == "__main__":
    if CWD != DIR:
        logging.info("working dir is %s. Change to %s", os.getcwd(), DIR)
        os.chdir(DIR)


    logging.info("change directory to ..")
    os.chdir("..")
    logging.info("call makeini.py with -f %s/master_ini.xml", DIR)
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
    logging.info("change directory back to %s", DIR)

    geofile = "%s/geometry.xml"%DIR
    inifiles = glob.glob("inifiles/*.xml")
    if not path.exists(geofile):
        logging.critical("geofile <%s> does not exist", geofile)
        exit(FAILURE)

    executable = "%s/bin/jpscore"%TRUNK
    if not path.exists(executable):
        logging.critical("executable <%s> does not exist yet.", executable)
        exit(FAILURE)

    for inifile in inifiles:
        if not path.exists(inifile):
            logging.critical("inifile <%s> does not exist", inifile)
            exit(FAILURE)
        #--------------------- SIMULATION ------------------------
        #os.chdir(TRUNK) #cd to the simulation directory
        cmd = "%s %s"%(executable, inifile)
        logging.info('start simulating with exe=<%s>', cmd)
        #------------------------------------------------------
        subprocess.call([executable, "%s"%inifile])
        #------------------------------------------------------
        logging.info('end simulation ...\n--------------\n')
        trajfile = "trajectories/traj" + inifile.split("ini")[2]
        logging.info('trajfile = <%s>', trajfile)
        #--------------------- PARSING & FLOW-MEASUREMENT --------
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exist", trajfile)
            exit(FAILURE)
        maxtime = get_maxtime(inifile)
        fps, N, traj = parse_file(trajfile)
        evac_time = (max(traj[:, 1]) - min(traj[:, 1])) / float(fps)
        tolerance = 0.01
        if (evac_time- must_time) > tolerance:
            logging.info("%s exits with FAILURE evac_time = %f (!= %f)",
                         argv[0], evac_time, must_time)
            exit(FAILURE)
        else:
            logging.info("evac_time = %f (!= %f)", evac_time, must_time)

    logging.info("%s exits with SUCCESS", argv[0])
    exit(SUCCESS)
