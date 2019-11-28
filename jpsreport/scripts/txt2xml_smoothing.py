#!/usr/bin/python
# format id  frame  x  y


import numpy as np
import time
#import pandas as pd
import os, sys, glob, math
from matplotlib.pyplot import *
from scipy import interpolate
import types
from sys import stderr, exit
from scipy.misc import comb
import logging, time
from txt2xml_lib import *



logfile='log.dat'
logging.basicConfig(filename=logfile, level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

args = getParserArgs()

fps = args.fps  #10  # 16? default frame per second
df = args.df  #10  # 16? default frame per second
isTrajCm = args.m # 1 means trajectories are in cm. Otherwise they are in m
isSmoothAngle = args.smoth # smooth angle
logfile = args.log.name  #'log.txt'
dir = args.path
v0 = 1.3
debug = 0
logging.info('fps = %d'%fps)
logging.info('fd = %d'%df)
logging.info('isTrajCm = %d'%isTrajCm)
logging.info('isSmoothAngle = %d'%isSmoothAngle)
logging.info('logfile = \"%s\"'%logfile)

t1 = time.time()
#========================================================================================================
WDir = os.getcwd() #working directory
#dirs=["UO/180", "UO/240", "UO/300", "UG", "KO/240", "KO/300", "EO/240", "EO/300"]
PrevDir = os.path.split(WDir)[0]
phi = 0 # dummy initialisation
phi2 = 0 # dummy initialisation

if isTrajCm == 0:  # data are in meter
    mTocm = 100 # to convert trajectories to cm
    cm = 1
else:
    mTocm = 1  
    cm = 0.01  # to convert speed to m/s


#for dir in dirs:
logging.info("dir = %s"%dir)
print(dir)
files = glob.glob("%s/*.txt"%(dir))
logging.info("Found %d txt-files"%len(files))
if not files:
	exit(logging.critical("found no files. exit.."))
for inputfile in files:
	outputfile  = os.path.split(inputfile)[0] + "/" + os.path.basename(inputfile).split(".")[0] + ".xml"
	data = np.loadtxt(inputfile)
	#data = np.array( pd.read_csv(inputfile, sep="\s+", header=None) )
	if len(data[0,:]) < 4: #experiment data have exactly 5 columns
		continue
	logging.info("|----> inputfile=%s"%inputfile)
	logging.info("<----| outputfile=%s"%outputfile)
	Ntemp = len(data[:,0])
	print(max(data[:,0]))
	#Nagents = max(data[:,0]) - min(data[:,0]) + 1 
	Nagents = len(np.unique(data[:,0]))
	print(Nagents)
	# if(Ntemp!=Nagents):
		# Nagents=Ntemp
	out = open(outputfile, "w") 
	write_header(out, Nagents, fps)
	# Todo: write geometry data------------------------------------------------------
	# write geometry
	#walls = [ [-500.0,-300.0], [-500,0], [-300, 0], [-300, 430], [0, 430], [0, 0], [400, 0], [400, -300],[-500.0,-300.0]]
	#write_geometry(out, walls)
	#--------------------------------------------------------------------------------------
	#write pedestrian data
	frames = np.unique(data[:,1]).astype(int)
	pids = np.unique(data[:,0]).astype(int)


	if np.min(pids) == 0:  #id start with 1
		pids += 1
		data[:,0] += 1

	#trajectories of peds. to be smoothed

	if isSmoothAngle == 1:
		paths = [data[data[:,0] == pid,1:4] for pid in pids]
		nullpoints = [getNullpoints(path) for path in paths]
		smoothedPaths = [interpolate_polyline(npt[:,1:], 100) for npt in nullpoints]

 
	write_frames(out, frames, data, mTocm)
	out.write("</trajectories>")
        


out.close()        
t2 = time.time()
logging.info('time: %.2f s'%(t2 - t1))

