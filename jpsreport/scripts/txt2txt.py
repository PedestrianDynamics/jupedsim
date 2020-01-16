#!/usr/bin/python
# format id  frame  x  y    z
from numpy import *
import os, sys, glob, math
import argparse


def getParserArgs():
	parser = argparse.ArgumentParser(description='Tranfer the unit from cm to m and add header for .txt file')
	parser.add_argument("-p", "--path", default="./", help='give the path of source file')
	parser.add_argument("-f", "--fps", default="16", type=float, help='give the frame rate of data')
	args = parser.parse_args()
	return args

if __name__ == '__main__':
    args = getParserArgs()
    path = args.path
    print(path)
    sys.path.append(path)
    fps = args.fps
    files = glob.glob("%s/*.txt"%(path))
    for file in files:
        description="experiment"
        geometry="geometry.xml"
        print(os.path.splitext(file)[0])
        fname = os.path.splitext(file)[0]+".txt"
        header="#description: %s\n#framerate: %d\n#geometry: %s\n#ID: the agent ID\n#FR: the current frame\n#X,Y,Z: the agents coordinates (in metres)\n\n#ID\tFR\tX\tY\tZ"%(description,fps,geometry)
        data=loadtxt(file,usecols = (0,1,2,3,4))
        data[:,2]/=100.
        data[:,3]/=100.
        data[:,4]/=100.
        #np.savetxt("./uo-100-300-300_space.txt",data, fmt= "%d %d %.4f %.4f %.4f", delimiter ='\t', header=header, comments='',newline='\n')
        savetxt(fname,data, fmt= "%d\t%d\t%.4f\t%.4f\t%.4f", delimiter ='\t', header=header, comments='', newline='\r\n')

