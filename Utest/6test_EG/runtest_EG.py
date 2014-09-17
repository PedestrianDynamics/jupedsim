#! /usr/bin/env python
import numpy as np
from xml.dom import minidom
import os, argparse, logging, time
from os import path, system
from sys import argv ,exit
import subprocess, sys

from matplotlib.pyplot import *

SUCCESS = 0
FAILURE = 1

#--------------------------------------------------------
logfile="log_test_EG.txt"
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')


#-------------------- DIRS ------------------------------
#HOME = path.expanduser("~")
TRUNK = "/home/chraibi/Workspace/peddynamics/JuPedSim/jpscore"
GEODIR = TRUNK + "/inputfiles/Bottleneck/"
TRAJDIR = GEODIR
CWD = os.getcwd()
#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
CWD = os.getcwd()
DIR = os.path.dirname(os.path.realpath(argv[0]))
#--------------------------------------------------------


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

    time1 = time.clock()
    widths = [0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.5]
    widths = [ 1.0, 1.4, 2.5]
    flows = []

    for w in widths:
    #------------------------------------------------------------------------------ 
        geofile = GEODIR +  str(w) + "_" + "bottleneck.xml"
        trajfile = GEODIR +  str(w) + "_" + "TrajBottleneck.xml"
        inifile =  GEODIR +  str(w) + "_ini-Bottleneck.xml"
        if not path.exists(geofile):
            logging.critical("geofile <%s> does not exist"%geofile)
            exit(FAILURE)
        if not path.exists(inifile):
            logging.critical("inifile <%s> does not exist"%inifile)
            exit(FAILURE)
        #--------------------- SIMULATION ------------------------  
        #os.chdir(TRUNK) #cd to the simulation directory
        executable = "%s/bin/jpscore"%TRUNK
        if not path.exists(executable):
            logging.critical("executable <%s> does not exit yet."%executable)
            exit(FAILURE)
        cmd = "%s --inifile=%s"%(executable, inifile)

        logging.info('start simulating wirh exe=<%s>'%cmd)
        #sh(cmd) #make simulation
        #subprocess.call([executable, "--inifile=%s"%inifile])
        logging.info('end simulation ...')
        #os.chdir(CWD) # cd back to the working directory
            
        #--------------------- PARSING & FLOW-MEASUREMENT --------
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exit"%trajfile)
            exit(FAILURE)
        fps, N, traj = parse_file(trajfile)
        J = flow(fps, N, traj, 61)
        flows.append(J)
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
    lid_f = np.array([ 1.70998632,  2.02483801,  2.19426565,  2.53207292,  2.91149068,  3.11461794, 3.90625,     3.91032325, 4.52352232,  5.54733728])
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
    xticks([0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3, 2.5])
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
