#! /usr/bin/env python
import numpy as np
from xml.dom import minidom
import os, argparse, logging, time
from os import path, system
from sys import argv ,exit
import subprocess

#from matplotlib.pyplot import *

SUCCESS = 0
FAILURE = 1

#--------------------------------------------------------
logfile="log_testflow.txt"
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')


#-------------------- DIRS ------------------------------
#HOME = path.expanduser("~")
TRUNK = argv[1] #HOME + "/workspace/peddynamics/JuPedSim/JPScore/trunk/"
GEODIR = TRUNK + "/inputfiles/Bottleneck/"
TRAJDIR = GEODIR
CWD = os.getcwd()
#--------------------------------------------------------

# def sh(script):
#     # run bash command
#     system("bash -c '%s'" % script)
    
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
    time1 = time.clock()
    widths = [0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.5]
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
        subprocess.call([executable, "--inifile=%s"%inifile])
        logging.info('end simulation ...')
        #os.chdir(CWD) # cd back to the working directory
            
        #--------------------- PARSING & FLOW-MEASUREMENT --------
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exit"%trajfile)
            exit(FAILURE)
        fps, N, traj = parse_file(trajfile)
        J = flow(fps, N, traj, 6100)
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
