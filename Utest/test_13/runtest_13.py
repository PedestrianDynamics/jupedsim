#! /usr/bin/env python
import numpy as np
import os, argparse, logging, time, sys
from os import path, system
from sys import argv ,exit
import subprocess, glob
import multiprocessing
from matplotlib.pyplot import *
import re

widths = [ 1.0, 1.2, 1.4, 1.6, 2.0, 2.2, 2.5 ]
#lid_w = np.array([ 0.9,  1.,   1.1,  1.2,  1.4, 1.6,  1.8,  2.,   2.2,  2.5])
#lid_f = np.array([ 1.70998632,  2.02483801,  2.19426565,  2.53207292,  2.91149068,  3.11461794, 3.90625,     3.91032325, 4.52352232,  5.54733728])
lid_w = widths
lid_f = np.array([ 2.02483801,  2.19426565,  2.53207292, 3.11461794, 3.91032325, 4.52352232, 5.54733728])
#=========================
testnr = 13
#========================

# SUCCESS = 0
# FAILURE = 1

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
    
    flows = {}
    tolerance = 0.5# todo: maybe too large
    time1 = time.clock()
    for e in ["png", "txt"]:
        if os.path.isfile("flow.%s"%e):
            os.remove("flow.%s"%e)
        
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
        print inifile
        width_size = float(inifile.split("geometry_")[1].split("_")[0])
        cmd = "%s --inifile=%s"%(executable, inifile)
        #--------------------- SIMULATION ------------------------  
        #os.chdir(TRUNK) #cd to the simulation directory      
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('\n--------------\n start simulating with exe=<%s>'%(cmd))
        logging.info('width_size = <%.2f>'%width_size)
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
        if not flows.has_key(width_size):
            flows[width_size] = [J]
        else:
            flows[width_size].append(J)
       
        logging.info("W = %f;  Flow = %f"%(width_size, J))
                     
    #logging.debug("flows: (%s)"%', '.join(map(str, flows)))
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

#    flows = np.array(flows)

    flow_file = "flow.txt"
    ff = open(flow_file, "w")
    logging.info('write flow values in \"%s\"'%flow_file)
    for key, value in flows.items():
        print >>ff, key, ":", value

    ff.close
    M = np.array([np.mean(i) for i in flows.values()]) # std pro CPU
    S = np.array([np.std(i) for i in flows.values()])   # std pro CPU
    print >>ff, "==========================="
    print >>ff, "==========================="
    print >>ff, "Means "
    print >>ff, M
    print >>ff, "==========================="
    print >>ff, "Std "
    print >>ff, S
    print >>ff, "==========================="
                 #########################################################################
    ms = 8
    #plot(widths, flows, "o-b", lw = 2, ms = ms, label = "simulation")
    indexsort = np.argsort( flows.keys() )
    F = np.array( flows.keys() )[indexsort]
    plot(F,  np.array(M)[indexsort], "o-", lw=2, label='Mean', color='blue')
    errorbar(F , np.array(M)[indexsort] , yerr=np.array(S)[indexsort], fmt='-o')

    plot(lid_w , lid_f, "D-k", lw=2, ms = ms, label = "experiment")
    axes().set_aspect(1./axes().get_data_ratio())
    columns = np.vstack((F, np.array(M)[indexsort]))
    gg = open("flow_col.txt", "w")
    for i in range(len(F)):
        print >>gg, columns[0][i], columns[1][i]
    gg.close()
    legend(loc='best', numpoints=1)
    grid()
    xlabel(r'$w\; [\, \rm{m}\, ]$',fontsize=18)
    ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$',fontsize=18)
    #xticks([0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3, 2.5])
    xticks(widths)
    xlim([0.7, 2.6])
    ylim([1, 6])
    err = np.sqrt( sum((M-lid_f)**2) )
    
    title(r"$\sqrt{{\sum_w {(\mu(w)-E(w)})^2 }}=%.2f\; (tol=%.2f)$"%(err, tolerance), y=1.02)
    
    savefig("flow.png")
    #show()
    #########################################################################
    
    time2 = time.clock()
    logging.info("time elapsed %.2f [s]."%(time2-time1))
    logging.info("err = %.2f, tol=%.2f"%(err, tolerance))
    
    if err > tolerance:
        exit(FAILURE)
    else:
        exit(SUCCESS)
