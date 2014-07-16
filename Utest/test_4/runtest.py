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

must_time = 10  # 10 m corridor with 1m/s 
SUCCESS = 0
FAILURE = 1
#--------------------------------------------------------
logfile="log_test_4.txt"
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
TRUNK = HOME + "/Workspace/peddynamics/JuPedSim/jpscore"
CWD = os.getcwd()
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
           

if __name__ == "__main__":
    
    geofile = "geometry.xml"
    inifile = "ini_test_4.xml"
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
        logging.critical("executable <%s> does not exist yet."%executable)
        exit(FAILURE)
    cmd = "%s --inifile=%s"%(executable, inifile)
    logging.info('start simulating with exe=<%s>'%(cmd))
    #------------------------------------------------------
    subprocess.call([executable, "--inifile=%s"%inifile])
    #------------------------------------------------------
    logging.info('end simulation ...\n--------------\n')
    trajfile = "Traj_test_4.xml"
    logging.info('trajfile = <%s>'%trajfile)
    #--------------------- PARSING & FLOW-MEASUREMENT --------
    if not path.exists(trajfile):
        logging.critical("trajfile <%s> does not exist"%trajfile)
        exit(FAILURE)
    fps, N, traj = parse_file(trajfile)

    traj_1 = traj[ traj[:,0] == 1 ]
    x_1 = traj_1[:,2]
    y_1 = traj_1[:,3]

    x_2 = traj[ traj[:,0] == 2 ][:,2]
    y_2 = traj[ traj[:,0] == 2 ][:,3]

    eps = 0.3 # 10 cm
    x_min = x_2[0] - eps
    x_max = x_2[0] + eps
    y_min = y_2[0] - eps
    y_max = y_2[0] + eps
    
    lx = np.logical_and( x_1 > x_min, x_1 < x_max )
    ly = np.logical_and( y_1 > y_min, y_1 < y_max )

    overlap = (lx*ly).any()
    

    if overlap:
        logging.info("%s exits with FAILURE"%(argv[0]))
        exit(FAILURE)
    else:
        logging.info("%s exits with SUCCESS"%(argv[0]))
        exit(SUCCESS)
