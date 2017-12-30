#!/usr/bin/env python

# todo: spaghetti code. directory mess

import glob
import logging
import matplotlib.pyplot as plt
import numpy as np
import os
import subprocess
import sys
import time
from os import path
from sys import argv, exit

#=========================
testnr = 10
#========================

SUCCESS = 0
FAILURE = 1
#--------------------------------------------------------
logfile = "log_test_%d.txt"%testnr
f = open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG,
                    format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
CWD = os.getcwd()
DIR = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
#--------------------------------------------------------


if __name__ == "__main__":
    [open(f, 'w') for f in glob.glob("log_*")] # empty logs
    if CWD != DIR:
        logging.info("working dir is %s. Change to %s"%(os.getcwd(), DIR))
        os.chdir(DIR)
    #-------- get directory of the code TRUNK
    os.chdir("../")
    TRUNK = os.getcwd()
    os.chdir(DIR)
    lib_path = os.path.abspath("%s/Utest"%TRUNK)
    sys.path.append(lib_path)
    from utils import *
    #----------------------------------------
    #logging.info("change directory to ..")
    #os.chdir("..")
    juelich_dir = lib_path + os.sep + "juelich_tests" + os.sep + "test_10"
    logging.info("call makeini.py with -f %s/master_ini.xml"%juelich_dir)
    subprocess.call(["python", "makeini.py", "-f", "%s/master_ini.xml"%juelich_dir])
    os.chdir(juelich_dir)
    logging.info("change directory back to %s"%juelich_dir)
    time1 = time.time()
    i = 0
    flows = {}
    geofile = "%s/geometry.xml"%juelich_dir
    inifiles = glob.glob("inifiles/*.xml")
    if not path.exists(geofile):
        logging.critical("geofile <%s> does not exist"%geofile)
        exit(FAILURE)
    if path.exists("cell.png"):
        subprocess.call(["rm", "cell.png"])

    timedic = {}
    for inifile in inifiles:
        if not path.exists(inifile):
            logging.critical("inifile <%s> does not exist"%inifile)
            exit(FAILURE)
        #--------------------- SIMULATION ------------------------
        #os.chdir(TRUNK) #cd to the simulation directory
        executable = "%s/bin/jpscore"%TRUNK
        if not path.exists(executable):
            logging.critical("executable <%s> does not exist yet."%executable)
            exit(FAILURE)
        cell_size = inifile.split("cell_size_")[1]
        if len(cell_size.split("_")) > 1: # cell_size in the middle
            cell_size = float(cell_size.split("_")[0])
        else: # cell_size at the end of filename
            cell_size = float(cell_size.split(".xml")[0])
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('start simulating with exe=<%s>'%(cmd))
        logging.info('cell_size = <%.2f>'%cell_size)
        #------------------------------------------------------
        t1_run = time.time()
        subprocess.call([executable, "--inifile=%s"%inifile])
        t2_run = time.time()
        if cell_size not in timedic:
            timedic[cell_size] = [t2_run - t1_run]
        else:
            timedic[cell_size].append(t2_run - t1_run)

        #------------------------------------------------------
        logging.info('end simulation ...\n--------------\n')
        trajfile = "trajectories/traj" + inifile.split("ini")[2]
        logging.info('trajfile = <%s>'%trajfile)
        #--------------------- PARSING & FLOW-MEASUREMENT --------
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exist"%trajfile)
            exit(FAILURE)
        fps, N, traj = parse_file(trajfile)
        J = flow(fps, N, traj, 61)

        if cell_size not in flows:
            flows[cell_size] = [J]
        else:
            flows[cell_size].append(J)

    #------------------------------------------------------------------------------
    logging.debug("flows: (%s)"%', '.join(map(str, flows)))
    # ----------------------- PLOT RESULTS ----------------------
    flow_file = "result.txt"
    times_file = "times.txt"
    ff = open(flow_file, "w")
    tt = open(times_file, "w")
    logging.info('write flow values in \"%s\"'%flow_file)
    # for key, value in list(flows.items()):
    #      print(key, ":", value, file=ff)

    # for key, value in list(timedic.items()):
    #     print(key, ":", value, file=tt)

    time2 = time.time()
    M = np.array([np.mean(i) for i in list(flows.values())]) # std pro CPU
    S = np.array([np.std(i) for i in list(flows.values())])   # std pro CPU
    MT = np.array([np.mean(i) for i in list(timedic.values())]) # std pro CPU
    ST = np.array([np.std(i) for i in list(timedic.values())])   # std pro CPU

    std_all = np.std(M)

    # print("===========================", file=ff)
    # print("===========================", file=ff)
    # print("Means ", file=ff)
    # print(M, file=ff)
    # print("===========================", file=ff)
    # print("Std ", file=ff)
    # print(S, file=ff)
    # print("===========================", file=ff)
    # print("Std all ", file=ff)
    # print(std_all, file=ff)
    # print("===========================", file=ff)
    # print("===========================", file=ff)

    ff.close()
    #########################################################################
    ms = 8
    ax = plt.subplot(211)
    indexsort = np.argsort(list(flows.keys()))
    F = np.array(list(flows.keys()))[indexsort]
    ax.plot(F, np.array(M)[indexsort], "o-", lw=2, label='Mean', color='blue')
    ax.errorbar(F, np.array(M)[indexsort], yerr=np.array(S)[indexsort], fmt='-o')
    #ax.errorbar(flows.keys(), M, yerr=S, fmt='-o')
    #ax.fill_between(flows.keys(), M+S, M-S, facecolor='blue', alpha=0.5)
    #axes().set_aspect(1./axes().get_data_ratio())
    #ax.legend(loc='best')
    ax.grid()
    ax.set_xlabel(r'$cell size\; [ m ]$', fontsize=18)
    ax.set_ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$', fontsize=18)
    ax.set_xlim(min(flows.keys()) - 0.5, max(flows.keys()) + 0.5)
    ax.set_ylim(min(M) - max(S) - 0.1, max(M) + max(S) + 0.1)
    ax.set_xticks(list(flows.keys()))
    plt.title("# Simulations %d"%len(flows[cell_size]))
#------------------ plot times
    ax2 = plt.subplot(212)

    indexsort = np.argsort(list(timedic.keys()))
    T = np.array(list(timedic.keys()))[indexsort]
    ax2.plot(T, np.array(MT)[indexsort], "o-", lw=2, label='Mean', color='blue')
    ax2.errorbar(T, np.array(MT)[indexsort], yerr=np.array(ST)[indexsort], fmt='-o')
    ax2.set_xlabel(r'$cell size\; [ m ]$', fontsize=18)
    ax2.set_ylabel(r'$T\; [  s ]$', fontsize=18)
    ax2.set_xticks(list(timedic.keys()))
    ax2.set_xlim(min(flows.keys()) - 0.5, max(flows.keys()) + 0.5)
    ax2.set_ylim(min(MT) - max(ST) - 0.1, max(MT) + max(ST) + 0.1)
    ax2.set_xticks(list(flows.keys()))
    #ax.legend(loc='best')
    ax2.grid()
    plt.tight_layout()

    logging.info("save file in cell.png")
    plt.savefig("cell.png")
    #plt.show()
    #########################################################################

    tolerance = 0.5# todo: this is to large 0.5
    logging.info("time elapsed %.2f [s]."%(time2-time1))
    logging.info("std_all = %.2f, tol=%.2f"%(std_all, tolerance))
    if std_all > tolerance:
        logging.success("%s exits with FAILURE std_all = %f > %f"%(argv[0], std_all, tolerance))
        exit(FAILURE)
    else:
        logging.info("%s exits with SUCCESS std_all = %f < %f"%(argv[0], std_all, tolerance))
        exit(SUCCESS)


