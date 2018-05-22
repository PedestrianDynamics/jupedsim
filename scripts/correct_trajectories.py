#!/usr/bin/env python3

#SCRIPT IN EARLY DEVELOPMENT
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
def readTrajData(pathfile,nameTraj,fps):
    
    print("reading from:%s/%s"%(pathfile,nameTraj))
    f_traj = open("%s/%s"%(pathfile,nametraj),"r")
    lines = f_traj.readlines()
    data = []
    for line in lines:
        line = line[:-1]
        numbers = line.split('\t')
        numbers = line.split(' ')
        if(line.startswith("#") != 1 and len(line) > 3):
            data.append(numbers)
    print(data[0])
    print(data[1])
    print(data[2])
    print(data[3])
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
    
#write a new file with corrected trajectories
def writeNewFile(trajData,filepath,nameTraj):
    print("getting copy info from:%s/%s"%(pathfile,nameTraj))
    f_traj = open("%s/%s"%(pathfile,nametraj),"r")
    lines = f_traj.readlines()
    f_traj.close()
    print("writing in:%s/%s_copy.txt"%(pathfile,nameTraj[:-4]))
    newFile = open("%s/%s_copy.txt"%(pathfile,nameTraj[:-4]),"w")
    i=0
    newFile.write("#This is an automatically generated trajectory file\r\n# that is supposed to correct the points into walls problem\r\n")
    while(lines[i].startswith("#")):    
        newFile.writelines(lines[i])
        i=i+1
    for line in trajData:
        newLine = "\t".join(line)
        newLine = newLine + "\r\n"
        newFile.write(newLine)
    newFile.close() 
    print("writing complete")
    return

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

#turn 3.0000 and 5.36666666666 into 3 and 5.3666
def floatToString(inputValue):
    return ('%.4f' % inputValue).rstrip('0').rstrip('.')

#return a*a+b*b
def distanceSquared(x1,y1,x2,y2):
    return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)

#calculate distance between point and closest point on wall, the distance is signed according to direction
def distanceFromWall(x,y,p1x,p1y,p2x,p2y,direction):
    
    #detect
    vect_A = [p2x - p1x, p2y - p1y]
    vect_B = [x - p1x, y - p1y]
    sinP = (np.linalg.det([vect_A,vect_B]))
    sinP = sinP * direction    
    
    a = distanceSquared(p1x,p1y,p2x,p2y)
    b = distanceSquared(p1x,p1y,x,y)
    c = distanceSquared(x,y,p2x,p2y)
    if (a == 0):
        return math.sqrt(math.sqrt(c))
    xSq = ((a+b-c)*(a+b-c))/(4*a)
    return np.sign(sinP)*math.sqrt(math.fabs(b - xSq))

#return 1 if close from wall (in a circle shape), else 0
def closeFromWall(x,y,p1x,p1y,p2x,p2y):
    bias = 0.1
    a = distanceSquared(p1x,p1y,p2x,p2y)
    b = distanceSquared(p1x,p1y,x,y)
    c = distanceSquared(x,y,p2x,p2y)
    if (a + bias > b + c):
        return 1
    else:
        return 0
        
#return coordinates of a point moved away from a wall
def moveFromWall(x,y,p1x,p1y,p2x,p2y,new_distance,direction):
    #print("DEBUG: ",x,y,p1x,p1y,p2x,p2y,new_distance,direction)
    if (x == p1x and y == p1y):
        x = x - 0.001
    if (x == p2x and y == p2y):
        x = x - 0.001
    aSq = distanceSquared(p1x,p1y,p2x,p2y)
    bSq = distanceSquared(p1x,p1y,x,y)
    cSq = distanceSquared(x,y,p2x,p2y)
    xSq = ((aSq+bSq-cSq)*(aSq+bSq-cSq))/(4*aSq)
    h = distanceFromWall(x,y,p1x,p1y,p2x,p2y,direction)
    
    #the cosine is needed to tell if x is negative
    vect_A = [p2x - p1x, p2y - p1y]
    vect_B = [x - p1x, y - p1y]
    cosP = np.dot(vect_A,vect_B)/(npl.norm(vect_A)*npl.norm(vect_B))    
    dist_x = math.sqrt(xSq) * np.sign(cosP)
    
    dist_a = math.sqrt(aSq)
    #print("DEBUG:",aSq,bSq,cSq,xSq,h,dist_x,dist_a)
    intersect_point_x = p1x + (dist_x/dist_a)*(p2x-p1x)
    intersect_point_y = p1y + (dist_x/dist_a)*(p2y-p1y)
    #print("DEBUG: the point at the base is",intersect_point_x,intersect_point_y)
    if(h==0):
        #the Point is exactly on the edge, to move it away, we rotate the point p1 around x 90 degrees
        x = intersect_point_x - (p2y - intersect_point_y)
        y = intersect_point_y + (p2x - intersect_point_x)
        #print("DEBUG: the new point far away is",x,y )
        aSq = distanceSquared(p1x,p1y,p2x,p2y)
        bSq = distanceSquared(p1x,p1y,x,y)
        cSq = distanceSquared(x,y,p2x,p2y)
        xSq = ((aSq+bSq-cSq)*(aSq+bSq-cSq))/(4*aSq)
        h = distanceFromWall(x,y,p1x,p1y,p2x,p2y,direction)
        dist_x = math.sqrt(math.fabs(xSq))
    ratio = new_distance/h
    x = intersect_point_x + ratio * (x - intersect_point_x)
    y = intersect_point_y + ratio * (y - intersect_point_y)
    return x,y
   
   
#treat one point according to policy
def handleSinglePoint(x,y,geoData,direction):
    
    backDistance = -1    #distance from behind the wall where correcting must start
    startDistance = 0.1  #minimal final distance from the wall
    endDistance = 0.5  #distance from front where correcting must start
    
    for wall in geoData:
        if (closeFromWall(x,y,wall[0],wall[1],wall[2],wall[3])):
            #print("close")
            distance = distanceFromWall(x,y,wall[0],wall[1],wall[2],wall[3],direction)
            if (backDistance <= distance and distance <= endDistance):
                #print("deserve treqtment")
                newDistance = (distance - backDistance) * ((endDistance - startDistance)/(endDistance - backDistance)) + startDistance
                x,y = moveFromWall(x,y,wall[0],wall[1],wall[2],wall[3],newDistance,direction)
            
    return x,y

#apply process to every point of the file
def handleAllPoint(trajData,geoData,direction):
    
    #reminder: trajData is modelled like [[ID,frame,x,y,z],...]
    #remider: geoData is modelled like [[p1x,p1y,p2x,p2y],[p2x,p2y,p3x,p3y]...]
    new_trajectories = []
    for index,inputs in enumerate(trajData):
        new_x,new_y = handleSinglePoint(float(inputs[2]),float(inputs[3]),geoData,direction)
        new_trajectories.append([inputs[0],inputs[1],floatToString(new_x),floatToString(new_y),inputs[4]])
        if (math.fmod(index,10000) == 0):
            print("Procesing: [",index,"/",len(trajData),"]")

    return new_trajectories
    

#sequences of unitary tests for debugging
def debugTest():
    
    #unitary test of computing distance:
   print("DEBUG: debugging distanceFromWall")
   d = distanceFromWall(0,0,0,0,0,0,1)
   print("=1.1= (0,0) is 0 far away from (0,0)/(0,0), we have:",d)
   
   d = distanceFromWall(1,0,0,0,0,0,1)
   print("=1.2= (1,0) is 1 far away from (0,0)/(0,0), we have:",d)   
   
   d = distanceFromWall(1,1,0,0,3,0,1)
   print("=1.3= (1,1) is 1 far away from (0,0)/(3,0), we have:",d)

   d = distanceFromWall(1,-1,0,0,3,0,1)
   print("=1.4= (1,-1) is -1 far away from (0,0)/(3,0), we have:",d)   

   d = distanceFromWall(1,1,3,0,0,0,1)
   print("=1.5= (1,1) is -1 far away from (3,0)/(0,0), we have:",d)

   d = distanceFromWall(1,-1,3,0,0,0,1)
   print("=1.6= (1,-1) is 1 far away from (3,0)/(0,0), we have:",d)   
    
    #unitary test of moving away one point:
   print("DEBUG: debugging moveFromWall")
   print("=2.1= we have a point at 1,1, we move it to be 2 units away from (0,0)(3,0)")
   print("we expect (1,2)")
   x,y = moveFromWall(1,1,0,0,3,0,2,1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.2= we have a point at 1,1, we move it to be 10 units away from (0,0)(3,0)")
   print("we expect (1,10)")
   x,y = moveFromWall(1,1,0,0,3,0,10,1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.3= we have a point at 1,1, we move it to be -2 units away from (0,0)(3,0)")
   print("we expect (1,-2)")
   x,y = moveFromWall(1,1,0,0,3,0,-2,1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.4= we have a point at 1,-1, we move it to be 2 units away from (0,0)(3,0)")
   print("we expect (1,2)")
   x,y = moveFromWall(1,-1,0,0,3,0,2,1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.5= we have a point at 1,1, we move it to be 3 units away from (0,0)(-4,0)")
   print("we expect (1,-3)")
   x,y = moveFromWall(1,1,0,0,-4,0,3,1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.6= we have a point at 0,2, we move it to be two units away from (0,0)(2,2)")
   print("we expect (-0.414,2.414)")
   x,y = moveFromWall(0,2,0,0,2,2,2,1)
   print("we have (%.3f,%.3f)"%(x,y))
   
   print("=2.7= we have a point at 1,1, we move it to be two units away from (0,0)(2,2)")
   print("we expect (-0.414,2.414)")
   x,y = moveFromWall(1,1,0,0,2,2,2,1)
   print("we have (%.3f,%.3f)"%(x,y))
   
   print("=2.8= we have a point at 1,1, we move it to be two units away from (0,0)(1,1)")
   print("we expect (-0.414,2.414)")
   x,y = moveFromWall(1,1,0,0,1,1,2,1)
   print("we have (%.3f,%.3f)"%(x,y))
   
   print("=2.9= we have a point at (5.5,-0.5), we move it to be 2 units away from (-5,0)(5,0)")
   print("we expect (5.5,2)")
   x,y = moveFromWall(5.5,-0.5,-5,0,5,0,2,1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("we flip the direction of the walls (right is now positive)")
   print("=2.10.1= we have a point at 1,1, we move it to be 2 units away from (0,0)(3,0)")
   print("we expect (1,-2)")
   x,y = moveFromWall(1,1,0,0,3,0,2,-1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.10.2= we have a point at 1,-1, we move it to be 10 units away from (0,0)(3,0)")
   print("we expect (1,-10)")
   x,y = moveFromWall(1,-1,0,0,3,0,10,-1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("=2.10.3= we have a point at 1,-1, we move it to be -2 units away from (0,0)(3,0)")
   print("we expect (1,2)")
   x,y = moveFromWall(1,-1,0,0,3,0,-2,-1)
   print("we have (%s,%s)"%(floatToString(x),floatToString(y)))
   
   
   
    #unitary test of moving automatically one point:
   print("DEBUG: debugging handleSinglePoint")
   walls = []
   walls.append([-5,0,5,0])
   walls.append([5,0,5,5])
   walls.append([5,5,-5,0])
   direction = 1
   print("Context: triangle with coordinates (5,0),(5,5),(-5,0)")
   x,y = handleSinglePoint(3.5,1.5,walls,direction)
   print("=3.1= (3.5,1.5), expected to become (3.5,1.5) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,1,walls,direction)
   print("=3.2= (4,1), expected to become (4,1) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,0.5,walls,direction)
   print("=3.3= (4,0.5), expected to become (4,0.5) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,0.25,walls,direction)
   print("=3.4= (4,0.25), expected to become (4,0.4333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,0,walls,direction)
   print("=3.5= (4,0), expected to become (4,0.3666) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,-0.5,walls,direction)
   print("=3.6= (4,-0.5), expected to become (4,0.2333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,-1,walls,direction)
   print("=3.7= (4,-1), expected to become (4,0.1) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,-2,walls,direction)
   print("=3.8= (4,-2), expected to become (4,-2) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4.75,0.25,walls,direction)
   print("=3.9= (4.75,0.25), expected to become (4.5666,0.4333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(5.5,-0.5,walls,direction)
   print("=3.10= (5.5,-0.5), expected to become (4.7666,0.2333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   
   return
   

    
        
#test line of lanch for ubuntu:
#python3 correct_trajectories.py -p ../demos/bottleneck -n traj_AO_300.txt -g geo_AO_300.xml -f 16
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
   
   #debugTest()
   
   new_trajectories = handleAllPoint(trajData,geoData,direction)
   writeNewFile(new_trajectories,pathfile,nametraj)
   
   
   
   
   

