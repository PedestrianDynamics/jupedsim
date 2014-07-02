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
logfile="log_test_2.txt"
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
    inifile = "ini_test_2.xml"
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
    trajfile = "Traj_test_2.xml"
    logging.info('trajfile = <%s>'%trajfile)
    #--------------------- PARSING & FLOW-MEASUREMENT --------
    if not path.exists(trajfile):
        logging.critical("trajfile <%s> does not exist"%trajfile)
        exit(FAILURE)
    fps, N, traj = parse_file(trajfile)

    evac_time = ( max( traj[:,1] ) - min( traj[:,1] ) ) / float(fps)
    
    tolerance = 0.01 
    if (evac_time- must_time) > tolerance:
        logging.info("%s exits with FAILURE evac_time = %f (!= %f)"%(argv[0], evac_time, must_time))
        exit(FAILURE)
    else:
        logging.info("%s exits with SUCCESS evac_time = %f (= %f)"%(argv[0], evac_time, must_time))
        exit(SUCCESS)
