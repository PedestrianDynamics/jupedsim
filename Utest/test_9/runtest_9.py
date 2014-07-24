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
testnr = 9
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


#TRUNK = HOME + "/Workspace/peddynamics/JuPedSim/jpscore"
CWD = os.getcwd()
DIR = os.path.dirname(os.path.realpath(argv[0]))
 #TRUNK + "/Utest/test_%d"%testnr
#--------------------------------------------------------
    
def parse_file(filename):
    """
    parse trajectories in Travisto-format and output results
    in the following  format: id    frame    x    y
    (no sorting of the data is performed)
    returns
    N: number of pedestrians
    data: trajectories
    """
    logging.info("parsing <%s>"%filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file. exit')
        exit(FAILURE)
    N = int(xmldoc.getElementsByTagName('agents')[0].childNodes[0].data)
    fps= xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data #type unicode
    fps = float(fps)
    fps = int(fps)
    print "fps=", fps
    #fps = int(xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data)
    logging.info ("Npeds = %d, fps = %d"%(N, fps))
    frames = xmldoc.childNodes[0].getElementsByTagName('frame')
    data = []
    for frame in frames:
        frame_number = int(frame.attributes["ID"].value)
        for agent in frame.getElementsByTagName("agent"):
            agent_id = int(agent.attributes["ID"].value)
            x = float(agent.attributes["xPos"].value)
            y = float(agent.attributes["yPos"].value)
            data += [agent_id, frame_number, x, y]
    data = np.array(data).reshape((-1,4))
    return fps, N, data
           
def flow(fps, N, data, x0):
    """
    measure the flow at a vertical line given by <x0>
    trajectories are given by <data> in the following format: id    frame    x    y
    input: 
    - fps: frame per second
    - N: number of peds
    - data: trajectories
    - x0: x-coordinate of the vertical measurement line
    output:
    - flow
    """
    logging.info('measure flow')
    if not isinstance(data, np.ndarray):
        logging.critical("flow() accepts data of type <ndarray>. exit")
        exit(FAILURE)
    peds = np.unique(data[:,0]).astype(int)
    times = []
    for ped in peds:
        d = data[ data[:,0] == ped ]
        first = min( d[ d[:,2] >= x0 ][:,1] )
        times.append( first )
    if len(times) < 2:
        logging.warning("Number of pedestrians passing the line is small. return 0")
        return 0    
    flow = fps * float(N-1) / ( max(times) - min(times) )
    return flow


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
    #----------------------------------------
    logging.info("change directory back to %s"%DIR)
    time1 = time.time()
    i = 0
    flows = {}
    MAX_CPU = multiprocessing.cpu_count()
    
    geofile = "%s/geometry.xml"%DIR
    inifiles = glob.glob("inifiles/*.xml")
    logging.info("MAX CPU = %d"%MAX_CPU)
    if not path.exists(geofile):
        logging.critical("geofile <%s> does not exist"%geofile)
        exit(FAILURE)


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
        b = inifile.split("numCPU_")[1]
        ncpu  = int( re.split("[.|_]", b)[0] )
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('start simulating with exe=<%s>'%(cmd))
        logging.info('n CPU = <%d>'%(ncpu))
        #------------------------------------------------------
        t1_run = time.time()
        subprocess.call([executable, "--inifile=%s"%inifile])
        t2_run = time.time()
        if not timedic.has_key(ncpu):
            timedic[ncpu] = [t2_run - t1_run]
        else:
            timedic[ncpu].append(t2_run - t1_run)
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
        
        if not flows.has_key(ncpu):
            flows[ncpu] = [J]
        else:
            flows[ncpu].append(J)
        
    #------------------------------------------------------------------------------ 
    logging.debug("flows: (%s)"%', '.join(map(str, flows)))
    # ----------------------- PLOT RESULTS ----------------------
    flow_file = "result.txt"
    times_file = "times.txt"
    ff = open(flow_file, "w")
    tt = open(times_file, "w")
    logging.info('write flow values in \"%s\" and times in \"%s\"'%(flow_file, times_file))
    for key, value in flows.items():
        print >>ff, key, ":", value

    for key, value in timedic.items():
        print >>tt, key, ":", value

    time2 = time.time()
    M = np.array([np.mean(i) for i in flows.values()]) # std pro CPU
    MT = np.array([np.mean(i) for i in timedic.values()]) # std pro CPU

    S = np.array([np.std(i) for i in flows.values()])   # std pro CPU
    ST = np.array([np.std(i) for i in timedic.values()])   # std pro CPU
    std_all = np.std(M)
   
    print >>ff, "==========================="
    print >>ff, "==========================="
    print >>ff, "Means "
    print >>ff, M
    print >>ff, "==========================="
    print >>ff, "Std "
    print >>ff, S
    print >>ff, "==========================="
    print >>ff, "Std all "
    print >>ff, std_all
    print >>ff, "==========================="
    print >>ff, "==========================="
   
    ff.close()
    tt.close()
    #########################################################################
    ms = 8
    ax = plt.subplot(211)
    ax.plot(flows.keys(), M, "o-", lw=2, label='Mean', color='blue')
    ax.errorbar(flows.keys(), M, yerr=S, fmt='-o')
    #ax.fill_between(flows.keys(), M+S, M-S, facecolor='blue', alpha=0.5)
    #axes().set_aspect(1./axes().get_data_ratio())  
    #ax.legend(loc='best')
    ax.grid()
    ax.set_xlabel(r'# cores',fontsize=18)
    ax.set_ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$',fontsize=18)
    ax.set_xlim(0.5, MAX_CPU + 0.5)
    ax.set_xticks(flows.keys())
    plt.title("# Simulations %d"%len(flows[ncpu]))
    #------------------ plot times
    ax2 = plt.subplot(212)
    ax2.plot(timedic.keys(), MT, "o-", lw=2, label='Mean', color='blue')
    ax2.errorbar(timedic.keys(), MT, yerr=ST, fmt='-o')
    ax2.set_xlabel(r'# cores',fontsize=18)
    ax2.set_ylabel(r'$T\; [  s ]$',fontsize=18)
    ax2.set_xticks(timedic.keys())
    ax2.set_xlim(0.5, MAX_CPU + 0.5)
    ax2.set_ylim( min( MT ) - max(ST)-0.1 , max( MT ) + max(ST) +0.1)
    #ax.legend(loc='best')
    ax2.grid()
    plt.tight_layout()
    logging.info("save file in cpu.png")
    plt.savefig("cpu.png")
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
