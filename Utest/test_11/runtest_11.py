#! /usr/bin/env python
import numpy as np
from xml.dom import minidom
import os, argparse, logging, time
from os import path, system
from sys import argv ,exit
import subprocess, glob
import multiprocessing
import matplotlib.pyplot as plt
import re

#=========================
testnr = 11
#========================

SUCCESS = 0
FAILURE = 1
#--------------------------------------------------------

logfile="log_test_%d.txt"%testnr
f=open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')


# https://stackoverflow.com/questions/1994488/copy-file-or-directory-in-python
import shutil, errno
def copyanything(src, dst):
    if os.path.exists(dst):
        shutil.rmtree(dst)
    try:
        shutil.copytree(src, dst)
    except OSError as exc: # python >2.5
        if exc.errno == errno.ENOTDIR:
            print dst
            shutil.copy(src, dst)
        else: raise

                                    
#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
CWD = os.getcwd()
DIR= os.path.dirname(os.path.realpath(argv[0]))
#--------------------------------------------------------
    
if __name__ == "__main__":
    if CWD != DIR:
        logging.info("working dir is %s. Change to %s"%(os.getcwd(), DIR))
        os.chdir(DIR)
    results = []
    for e in ["a", "b"]:
        os.chdir("..")
        logging.info("Change directory to %s"%os.getcwd())
        Masterfile = "%s/master_ini_%c.xml"%(DIR,e)
        logging.info('makeini files with = <%s>'%Masterfile)
        #subprocess.call(["python", "makeini.py", "-f %s"%Masterfile])
        subprocess.call(["python", "makeini.py", "-f", "%s"%Masterfile])
        os.chdir(DIR)
        logging.info("Change directory to %s"%DIR)
        logging.info('copy inifiles to  = inifiles_%c'%e)
        if not path.exists("inifiles"):
            logging.critical("inifiles was not created")
            exit(FAILURE)
        copyanything("inifiles", "inifiles_%c"%e)
        
        logging.info('run %c_runtest.py'%e)
        result  = subprocess.call(["python", "%c_runtest.py"%e])
        results.append(result)
        logging.info('copy trajectories to trajectories_%c'%e)
        copyanything("trajectories", "trajectories_%c"%e)
    logging.info('results [%.2f --- %.2f]'%(results[0], results[1]))
    if np.fabs(results[0]-results[1] ) >0.01:
        logging.critical('%s returns with FAILURE'%(argv[0]))
        exit(FAILURE)
    else:
        logging.info('%s returns with SUCCESS'%(argv[0]))
        exit(SUCCESS)
