#SCRIPT IN EARLY DEVELOPMENT

#!/usr/bin/env python3
import numpy as np
import numpy.linalg as npl
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import pylab
import argparse
import sys, glob,os
import math
from lxml import etree


def getParserArgs():
	parser = argparse.ArgumentParser(description='Correct raw trajectories to keep points inside geometry')
	parser.add_argument("-p", "--pathfile", default="./", help='give the path of source file')
	parser.add_argument("-n", "--nametraj", help='give the name of the trajectory file (txt)')
	parser.add_argument("-g", "--namegeo", help='give the name of the geometry file (xml)')
	parser.add_argument("-f", "--fps", default="16", type=int, help='give the frame rate of data')
	args = parser.parse_args()
	return args
 
#convert the txt file of trajectories into
#list of [ID,frame,x,y,z]
def readTrajData(pathfile,nametraj,fps):
    
    print("reading from:%s/%s"%(pathfile,nametraj))
    f_traj = open("%s/%s"%(pathfile,nametraj),"r")
    lines = f_traj.readlines()
    lines = lines[8:]
    data = []
    for line in lines:
        line = line[:-1]
        numbers = line.split('\t')
        data.append(numbers)
    return data

#convert the xml file of geography into
#list of [p1x, p1y, p2x, p2y]
def readGeoData(pathfile,namegeo,fps):
    f_geo = etree.parse("%s/%s"%(pathfile,namegeo))
    vertex_list = f_geo.findall("rooms/room/subroom/polygon/vertex")
    wall_list = []
    i = 0
    while i < len(vertex_list)-1 :
        wall = [float(vertex_list[i].attrib["px"])
                ,float(vertex_list[i].attrib["py"])
                ,float(vertex_list[i+1].attrib["px"])
                ,float(vertex_list[i+1].attrib["py"])]
        wall_list.append(wall)        
        i=i+1
    print(vertex_list)
    print(vertex_list[0])
    print(wall_list)
    return wall_list

#compute the direction "around" a geometry
#clock-wise = -1, counter-clokwise = 1
def computeDirection(wall_list):
    if (wall_list[0][0:1] != wall_list[-1][2:3]) :
        print("WARNING: wall is not making a loop, using default value for rotation")
        return 1              
    
    total_angle = 0
    i = 0
    j = 1
    while i < len(wall_list) :
        vect_A = [wall_list[i][2] - wall_list[i][0], wall_list[i][3] - wall_list[i][1]]
        vect_B = [wall_list[j][2] - wall_list[j][0], wall_list[j][3] - wall_list[j][1]]
        cosP = np.dot(vect_A,vect_B)/(npl.norm(vect_A)*npl.norm(vect_B))
        sinP = (np.linalg.det([vect_A,vect_B]))
        angle = (np.sign(sinP)*np.arccos(cosP))
        total_angle += angle
        i = i+1
        j = (i+1)%len(wall_list)
    total_angle = (total_angle/(2*math.pi))
    if(total_angle == -1):
        print("rotating clockwise")
        return -1
    elif(total_angle == 1):
        print("rotating anti clockwise")
        return 1
    else:
        print("WARNING: unusual angle,",total_angle)
        return total_angle
        
#calculate distance between point and closest point on wall
def distanceFromWall(x,y,p1x,p1y,p2x,p2y):
    """TODO"""
    return 1.0

#return 1 if close from wall, else 0
def closeFromWall(x,y,p1x,p1y,p2x,p2y):
    """TODO"""
    return 1
        
#return coordinates of a point moved away from a wall
def moveFromWall(x,y,p1x,p1y,p2x,p2y,distance):   
    """TODO"""
    return x,y
    
#treat one point according to policy
def handleSinglePoint(x,y,z,geoData,direction):
    """TODO"""
    return

#apply process to every point of the file
def handleAllPoint(trajData,geoData,direction):
    """TODO"""
    
    
        
#test line of lanch for ubuntu:
#python3 correct_trajectories.py -p ../demos/bottleneck -n traj_A0_300.txt -g geo_AO_300.xml -f 16
if __name__ == '__main__':
   args = getParserArgs()
   pathfile = args.pathfile
   sys.path.append(pathfile)
   nametraj = args.nametraj
   namegeo = args.namegeo
   print("INFO:\tEditing the trajectory from: <%s>"%(nametraj)) 
   fps = args.fps
   trajData = readTrajData(pathfile,nametraj,fps)
   geoData = readGeoData(pathfile,namegeo,fps)
   direction = computeDirection(geoData)
   
   """
  TODO:
      call handleAllPoint
      generate new Trajectory file and write it
  
   """


