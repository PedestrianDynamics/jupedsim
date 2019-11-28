#!/usr/bin/python
#----------------------------------------------
# input txt-file in the following format
# format id  frame  x  y
# output: xml-file for JuPedSim/jpsvis
#=====
# Tip: follow log by running in another pane: tail -f log.dat
#----------------------------------------------

import numpy as np
import time
import pandas as pd
import os, glob
from sys import exit
import logging, argparse

#================================
logfile='log.dat'
logging.basicConfig(filename=logfile, level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def getParserArgs():
    parser = argparse.ArgumentParser(description='Convert the experimental data to JuPedSim-format')
    parser.add_argument("-m", "--m", action='store_const', const=0, default=1, help='trajectories are in m (default cm)')
    parser.add_argument("-f", "--fps", type=int , default=10, help='frames per seconds (default 10)')
    parser.add_argument("-d", "--df", type=int , default=10, help='Delta (frames) to calculate velocity  (default 10)')
    parser.add_argument("-l", "--log" , type=argparse.FileType('w'), default='log.dat', help="log file (default log.dat)")
    parser.add_argument("-g", "--geometry" , type=str, default='', help="geometry file (default '')")
    parser.add_argument("-p", "--path", default=".", help='give the directory of the trajectory files')
    args = parser.parse_args()
    return args

def write_header(out, Nagents, fps, geometry=""):
    out.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    out.write("<trajectories>\n\n")
    # write header
    out.write("<header version = \"0.5.1\">\n")
    out.write("\t<agents>%d</agents>\n"%Nagents)
    out.write("\t<seed>12542</seed>\n")
    out.write("\t<frameRate>%d</frameRate>\n"%fps)
    out.write("</header>\n")
    # write geometry if any
    if len(geometry):
        out.write("<geometry>\n")
        out.write("\t<file location= \"%s\"/>\n"%geometry)
        out.write("</geometry>\n")

    # write description
    out.write("<AttributeDescription>\n")
    out.write("\t<property tag=\"x\" description=\"xPosition\"/>\n")
    out.write("\t<property tag=\"y\" description=\"yPosition\"/>\n")
    out.write("\t<property tag=\"z\" description=\"zPosition\"/>\n")
    out.write("\t<property tag=\"rA\" description=\"radiusA\"/>\n")
    out.write("\t<property tag=\"rB\" description=\"radiusB\"/>\n")
    out.write("\t<property tag=\"eC\" description=\"ellipseColor\"/>\n")
    out.write("\t<property tag=\"eO\" description=\"ellipseOrientation\"/>\n")
    out.write("</AttributeDescription>\n\n")



def write_frames(out, frames, data, mTocm ):
    extract_id = np.unique(data[:,0])
    if len(extract_id) != (max(data[:,0])-min(data[:,0])+1):
        data1 = np.array([[0, 0, 0, 0]])        
        id = 1
        for i in extract_id:
            data_id = data[ data[:,0] == i ]
            data_id[:,0] = id
            data1 = np.append(data1, data_id, axis=0)
            id = id + 1
        data = np.delete(data1, (0), axis=0)
    for frame in frames:
        if frame%1000 == 0:
            logging.info("++ frame:\t %d / %d"%(frame, len(frames)))
        d = data [ data[:,1] == frame ] # get data framewise
#================== begin write frame ==============
        out.write("<frame ID=\"%d\">\n"%frame)
        
        for (agent, x, y, z) in zip(d[:,0].astype(int), d[:,2], d[:,3], d[:, 4]):        

            text = "\t<agent ID=\"%d\" \tx=\"%.2f\"\ty=\"%.2f\"\tz=\"%.2f\"\trA=\"%.2f\"\trB=\"%.2f\"\t eO=\"%.2f\"\teC=\"%d\"/>\n"%(agent, x*mTocm*0.01, y*mTocm*0.01, z*mTocm*0.01, 0.20, 0.20, 0, 100)
            out.write(text)
            
        out.write("</frame>\n")
#================== end write frame ================


if __name__ == '__main__':

    args = getParserArgs()
    fps = args.fps  #10  # 16? default frame per second
    df = args.df  #10  # 16? default frame per second
    isTrajCm = args.m # 1 means trajectories are in cm. Otherwise they are in m
    logfile = args.log.name  #'log.txt'
    geometryFile = args.geometry #os.path.abspath(args.geometry)
    dir = args.path
    v0 = 1.3
    logging.info('fps = %d'%fps)
    logging.info('fd = %d'%df)
    logging.info('isTrajCm = %d'%isTrajCm)
    logging.info('logfile = \"%s\"'%logfile)
    logging.info('geometry = \"%s\"'%geometryFile)
    t1 = time.time()
    #========================================================================================================
    WDir = os.getcwd() #working directory
    PrevDir = os.path.split(WDir)[0]

    if isTrajCm == 0:  # data are in meter
        mTocm = 100 # to convert trajectories to cm
        cm = 1
    else:
        mTocm = 1
        cm = 0.01  # to convert speed to m/s


    #for dir in dirs:
    logging.info("dir = %s"%dir)
    logging.info("dir: %s" %dir)
    files = glob.glob("%s/*.txt"%(dir))
    logging.info("Found %d txt-files"%len(files))
    if not files:
        exit(logging.critical("found no files *.txt. exit.."))
    for inputfile in files:
        outputfile  = os.path.split(inputfile)[0] + "/" + os.path.basename(inputfile).split(".")[0] + ".xml"
        #data = np.loadtxt(inputfile)
        data = np.array( pd.read_csv(inputfile, sep="\s+", header=None, comment="#") )
        if len(data[0,:]) < 4: #experiment data have exactly 5 columns
            logging.warning("File has only %d columns. at least 4 columns are expected"%len(data[0, :]))
            continue
        elif len(data[0,:]) == 4: # no z column, add z with default 1.71 m
            z_col = 1.71 *mTocm*0.01 * np.ones(len(data[:, 0]))
            data = np.concatenate((data, z_col.reshape(len(data[:, 0]), 1)), axis=1)
        logging.info("|----> inputfile=%s"%inputfile)
        Ntemp = len(data[:,0])
        logging.info("Max ids: %d" % max(data[:,0]))
        #Nagents = max(data[:,0]) - min(data[:,0]) + 1 
        Nagents = len(np.unique(data[:,0]))
        logging.info("N: %d" %Nagents)
        out = open(outputfile, "w")
        logging.info("Write header to <%s>" % outputfile)
        write_header(out, Nagents, fps, geometryFile)
        #write pedestrian data
        frames = np.unique(data[:,1]).astype(int)
        pids = np.unique(data[:,0]).astype(int)


        if np.min(pids) == 0:  #id start with 1
                pids += 1
                data[:,0] += 1


        logging.info("Write frames to <%s>" % outputfile)
        write_frames(out, frames, data, mTocm)
        out.write("</trajectories>")


        out.close()
        logging.info("<----| outputfile=%s"%outputfile)
    t2 = time.time()
    logging.info('time: %.2f s'%(t2 - t1))

