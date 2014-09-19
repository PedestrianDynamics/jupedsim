#! /usr/bin/env python
import numpy as np
import os, argparse, logging, time, sys
from os import path, system
from sys import argv ,exit
import subprocess, glob
import multiprocessing
import matplotlib.pyplot as plt
import re

#=========================
testnr = 13
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
    
    widths = [ 1.0, 1.6, 2.5]    
    flows = []
    time1 = time.clock()

    # geofile = "%s/geometry.xml"%DIR
    inifiles = glob.glob("inifiles/*.xml")
    # if not path.exists(geofile):
    #     logging.critical("geofile <%s> does not exist"%geofile)
    #     exit(FAILURE)
        
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
        fps, N, traj = parse_file(trajfile)
        J = flow(fps, N, traj, 61)
        flows.append(J)
        tolerance = 0.01
        #------------------------------------------------------------------------------ 
    logging.debug("flows: (%s)"%', '.join(map(str, flows)))
    
    # ----------------------- PLOT RESULTS ----------------------
    #sey = np.loadtxt("bck-b-scaling/seyfried-j-b.dat")
    #kretz = np.loadtxt("bck-b-scaling/kretz-j-b.dat")
    #muel32 = np.loadtxt("bck-b-scaling/mueller-bg-32-No.dat")
    #muel26 = np.loadtxt("bck-b-scaling/mueller-bg-26-No.dat")
    # lid = np.loadtxt("bck-b-scaling/Flow_vs_b_v2.dat")
    # lid_w = lid[:,0]/100.0
    # lid_f = 150.0/(lid[:,4]-lid[:,1])
    lid_w = np.array([ 0.9,  1.,   1.1,  1.2,  1.4, 1.6,  1.8,  2.,   2.2,  2.5])
    #lid_f = np.array([ 1.70998632,  2.02483801,  2.19426565,  2.53207292,  2.91149068,  3.11461794, 3.90625,     3.91032325, 4.52352232,  5.54733728])
    lid_f = np.array([ 2.02483801,  3.11461794,  5.54733728])
    flows = np.array(flows)

    flow_file = "flow.txt"
    ff = open(flow_file, "w")
    logging.info('write flow values in \"%s\"'%flow_file)
    print >>ff, flows
    ff.close
    #########################################################################
    ms = 8
    plot(widths, flows, "o-b", lw = 2, ms = ms, label = "simulation")
    plot(lid_w , lid_f, "D-k", ms = ms, label = "experiment")
    axes().set_aspect(1./axes().get_data_ratio())  
    legend(loc='best')
    grid()
    xlabel(r'$w\; [\, \rm{m}\, ]$',fontsize=18)
    ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$',fontsize=18)
    #xticks([0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3, 2.5])
    xticks([1.0, 1.6, 2.5])
    xlim([0.7, 2.6])
    ylim([1, 6])
    savefig("flow.png")
    show()
    #########################################################################
    
    time2 = time.clock()
    err = np.sqrt( sum((flows-lid_f)**2) )
    tolerance = 0.5# todo: this is to large 0.5
    logging.info("time elapsed %.2f [s]."%(time2-time1))
    logging.info("err = %.2f, tol=%.2f"%(err, tolerance))
    if err > tolerance:
        exit(FAILURE)
    else:
        exit(SUCCESS)
