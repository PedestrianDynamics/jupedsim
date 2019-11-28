#!/usr/bin/env python3

"""
=== TRAJECTORY CORRECTOR FOR JPSreport ===
author: Arno Gay-Bauer 2018-05-24

This programm takes a trajectory file and a geometry file and move
the points outside of obstacles and away from walls.

In order to do that every point close to an obstacle is first slightly
moved away to avoid jumps of trajectories around corner.
Then, every point close from walls and obstacles get pushed away from walls 
so that there is no point inside obstacles.

"""

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
	args = parser.parse_args()
	return args
 
#convert the txt file into point list
#list of [ID,frame,x,y,z]
def readTrajData(pathfile,nameTraj):
    
    print("reading from:%s/%s"%(pathfile,nameTraj))
    f_traj = open("%s/%s"%(pathfile,nametraj),"r")
    lines = f_traj.readlines()
    data = []
    for line in lines:
        line = line.rstrip()
        numbers = line.split()
        if not line.startswith("#") and len(line) > 3:
            data.append(numbers)

    return data

#convert the xml file of geography into
#list of ["type=obstacle|walls",direction,list of [p1x, p1y, p2x, p2y]]
def readGeoData(pathfile,namegeo):
    f_geo = etree.parse("%s/%s"%(pathfile,namegeo))
    wall_list = f_geo.findall("rooms/room/subroom/polygon")
    obstacle_list = f_geo.findall("rooms/room/subroom/obstacle/polygon")
    geometry_list = []
    i = 0
    
    for obstacle in obstacle_list:
        vertices = []
        i = 0
        while i < len(obstacle)-1 :
            vertice = [float(obstacle[i].attrib["px"])
                    ,float(obstacle[i].attrib["py"])
                    ,float(obstacle[i+1].attrib["px"])
                    ,float(obstacle[i+1].attrib["py"])]
            vertices.append(vertice)   
            i=i+1
        geometry_list.append(["obstacle",-1*computeDirection(vertices),vertices])
        
    for wall in wall_list:
        vertices = []
        i = 0
        while i < len(wall)-1 :
            vertice = [float(wall[i].attrib["px"])
                    ,float(wall[i].attrib["py"])
                    ,float(wall[i+1].attrib["px"])
                    ,float(wall[i+1].attrib["py"])]
            vertices.append(vertice)        
            i=i+1
        geometry_list.append(["walls",computeDirection(vertices),vertices])

    return geometry_list
    
#write a new file with corrected trajectories
def writeNewFile(trajData,filepath,nameTraj):
    print("getting copy info from:%s/%s"%(pathfile,nameTraj))
    f_traj = open("%s/%s"%(pathfile,nametraj),"r")
    lines = f_traj.readlines()
    f_traj.close()
    print("writing in:%s/%s_copy.txt"%(pathfile,nameTraj[:-4]))
    newFile = open("%s/%s_copy.txt"%(pathfile,nameTraj[:-4]),"w")
    i=0
    newFile.write("#This is an automatically generated trajectory file\r\n#that is supposed to move points away from walls \r\n")
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
def computeDirection(vertices):
    
    if (vertices[0][0:1] != vertices[-1][2:3]) :
        print("WARNING: wall is not making a loop, using default value for rotation")
        return 1              
    
    total_angle = 0
    i = 0
    j = 1
    while i < len(vertices) :
        vect_A = [vertices[i][2] - vertices[i][0], vertices[i][3] - vertices[i][1]]
        vect_B = [vertices[j][2] - vertices[j][0], vertices[j][3] - vertices[j][1]]
        cosP = np.dot(vect_A,vect_B)/(npl.norm(vect_A)*npl.norm(vect_B))
        sinP = (np.linalg.det([vect_A,vect_B]))
        angle = (np.sign(sinP)*np.arccos(cosP))
        total_angle += angle
        i = i+1
        j = (i+1)%len(vertices)
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
def closeFromWall(x,y,p1x,p1y,p2x,p2y,bias=0):
    a = distanceSquared(p1x,p1y,p2x,p2y)
    b = distanceSquared(p1x,p1y,x,y)
    c = distanceSquared(x,y,p2x,p2y)
    if (a + bias > b + c):
        return 1
    else:
        return 0
        
#return 1 if close from wall (in a triangle shape), else 0
def closeFromWallTriangle(x,y,p1x,p1y,p2x,p2y):
    a = [p2x - p1x, p2y - p1y]
    b = [x - p1x, y - p1y]
    c = [x - p2x, y - p2y]
    a = np.abs(a)
    b = np.abs(b)
    c = np.abs(c)
    if (a[0]+a[1] >= b[0]+b[1] and a[0]+a[1] >= c[0]+c[1] ):
        return 1
    else:
        return 0

#return true if the outside of the wall is in sight of the point
def wallFacingPoint(x,y,p1x,p1y,p2x,p2y,direction):
    h = distanceFromWall(x,y,p1x,p1y,p2x,p2y,direction)
    if (h >= 0):
        return 1
    else:
        return 0
    
        
#return coordinates of a point moved away from a wall
def moveFromWall(x,y,p1x,p1y,p2x,p2y,new_distance,direction):
    #print("DEBUG: ",x,y,p1x,p1y,p2x,p2y,new_distance,direction)
    #if the points are exactly on the corners of the edges, we move them slightly
    if (x == p1x and y == p1y):
        x = x - 0.001
    if (x == p2x and y == p2y):
        x = x - 0.001
    #the triangle is p1 p2 (x,y) where dist_a is the wall segment length 
    #and dist_x  the length between p1 and the projection of (x,y) on the wall
    aSq = distanceSquared(p1x,p1y,p2x,p2y)
    bSq = distanceSquared(p1x,p1y,x,y)
    cSq = distanceSquared(x,y,p2x,p2y)
    xSq = ((aSq+bSq-cSq)*(aSq+bSq-cSq))/(4*aSq)
    h = distanceFromWall(x,y,p1x,p1y,p2x,p2y,direction)
    #the cosine is needed to tell if the projection is outside the segment
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
        #if the Point is exactly on the edge, to move it away, we rotate the point p1 around x by 90 degrees
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
def handleSinglePoint(x,y,geoData):
    
    for geometry in geoData:
        direction = geometry[1]
        
        if(geometry[0] == "walls"):
            backDistance = -1       #distance from behind the wall where correcting must start
            startDistance = 0.1     #minimal final distance from the wall
            endDistance = 0.5       #distance from front where correcting must start     
            for edge in geometry[2]:
                if (closeFromWall(x,y,edge[0],edge[1],edge[2],edge[3])):
                    distance = distanceFromWall(x,y,edge[0],edge[1],edge[2],edge[3],direction)
                    if (backDistance <= distance and distance <= endDistance):
                        #x' = (x-a)*((c-b)/(c-a))+b  
                        newDistance = (distance - backDistance) * ((endDistance - startDistance)/(endDistance - backDistance)) + startDistance
                        x,y = moveFromWall(x,y,edge[0],edge[1],edge[2],edge[3],newDistance,direction)            
            
        else: #obstacle
            backDistance = -0.5     #distance from behind the wall where correcting must start
            startDistance = 0.01    #minimal final distance from the wall
            endDistance = 0.10      #distance from front where correcting must start  
            
            #gently pushing the points to avoid jumps around corners
            for edge in geometry[2]:
                if (closeFromWall(x,y,edge[0],edge[1],edge[2],edge[3],4*endDistance) 
                    and wallFacingPoint(0,0,edge[0],edge[1],edge[2],edge[3],direction)):
                    distance = distanceFromWall(x,y,edge[0],edge[1],edge[2],edge[3],direction)
                    if (backDistance <= distance and distance <= startDistance):
                        #x' =((c-a)/(b-a))*(x-a)+a
                        newDistance = ((endDistance - backDistance)/(startDistance - backDistance)) * (distance - backDistance) + backDistance 
                        x,y = moveFromWall(x,y,edge[0],edge[1],edge[2],edge[3],newDistance,direction)  
            
            #moving points away from inside the walls
            for edge in geometry[2]:
                if (closeFromWallTriangle(x,y,edge[0],edge[1],edge[2],edge[3]) and wallFacingPoint(0,0,edge[0],edge[1],edge[2],edge[3],direction)):
                    distance = distanceFromWall(x,y,edge[0],edge[1],edge[2],edge[3],direction)
                    if (backDistance <= distance and distance <= endDistance):
                        #x' = (x-a)*((c-b)/(c-a))+b 
                        newDistance = (distance - backDistance) * ((endDistance - startDistance)/(endDistance - backDistance)) + startDistance
                        x,y = moveFromWall(x,y,edge[0],edge[1],edge[2],edge[3],newDistance,direction) 
               
    return x,y
    
def smoothPoint(points):
    new_x = 0
    new_y = 0
    length = len(points)
    for point in points:
        new_x = new_x + float(point[2])/length
        new_y = new_y + float(point[3])/length
    return new_x,new_y
    
#apply process to every point of the file
def handleAllPoint(trajData,geoData):
    
    #reminder: trajData is modelled like [[ID,frame,x,y,z],...]
    #remider: geoData is modelled like list of [type="obstacle"|"walls",direction,list of [p1x, p1y, p2x, p2y]]
    new_trajectories = []
    for index,inputs in enumerate(trajData):
        new_x,new_y = handleSinglePoint(float(inputs[2]),float(inputs[3]),geoData)
        new_trajectories.append([inputs[0],inputs[1],floatToString(new_x),floatToString(new_y),inputs[4]])
        if (math.fmod(index,10000) == 0):
            print("Processing: [",index,"/",len(trajData),"]")
    
    smooth_order = 3
    for index,inputs in enumerate(new_trajectories):
        if (smooth_order < index and index < len(new_trajectories)-smooth_order 
            and new_trajectories[index + smooth_order][0] == new_trajectories[index][0]
            and new_trajectories[index - smooth_order][0] == new_trajectories[index][0]):
                new_x,new_y = smoothPoint(new_trajectories[index - smooth_order : index + smooth_order])
                new_trajectories[index] = [inputs[0],inputs[1],floatToString(new_x),floatToString(new_y),inputs[4]]
        if (math.fmod(index,10000) == 0):
            print("Smoothing: [",index,"/",len(trajData),"]")   
           
    return new_trajectories
    
#display the graphs for trajectories
def plotTrajectories(geoData,old_traj,new_traj): 
    
    #put the data into displayable arrays
    geometryCurvesX = []
    geometryCurvesY = []
    for geometry in geoData:
        #print("DEBUG: geo=",geometry)
        edges = geometry[2]
        geoCurveX = []
        geoCurveY = []
        geoCurveX.append(edges[0][0])
        geoCurveY.append(edges[0][1])
        for edge in edges:
            geoCurveX.append(edge[2])
            geoCurveY.append(edge[3])
        geometryCurvesX.append(geoCurveX)
        geometryCurvesY.append(geoCurveY)
        
    oldCurveX = []
    oldCurveY = []
    i = 0
    trajectX = []
    trajectY = []
    for data in old_traj:
        if(int(data[0]) == i):
            trajectX.append(float(data[2]))
            trajectY.append(float(data[3]))
        else:
            oldCurveX.append(trajectX)
            oldCurveY.append(trajectY)
            trajectX = []
            trajectY = []
            trajectX.append(float(data[2]))
            trajectY.append(float(data[3]))
            i = i+1
            
    newCurveX = []
    newCurveY = []
    i = 0
    trajectX = []
    trajectY = []
    for data in new_traj:
        if(int(data[0]) == i):
            trajectX.append(float(data[2]))
            trajectY.append(float(data[3]))
        else:
            newCurveX.append(trajectX)
            newCurveY.append(trajectY)
            trajectX = []
            trajectY = []
            trajectX.append(float(data[2]))
            trajectY.append(float(data[3]))
            #print("DEBUG: curve #",i)
            i = i+1

    #display all the trajectories at once on the same graph
    scope = range(len(oldCurveX)) 
    
    plt.figure(1)
    #all the trajectories before calculation
    plt.subplot(121)
    plt.axis([-4,4,-4,4])
    plt.title('old trajectories')
    for i in range(len(geometryCurvesX)):
        plt.plot(geometryCurvesX[i],geometryCurvesY[i], 'r-')
    for i in scope:     
        plt.plot(oldCurveX[i],oldCurveY[i], 'bo', ms= 1)
    #all the trajectories after calculation
    plt.subplot(122)
    plt.axis([-4,4,-4,4])
    plt.title('new trajectories')
    for i in range(len(geometryCurvesX)):
        plt.plot(geometryCurvesX[i],geometryCurvesY[i], 'r-')
    for i in scope:     
        plt.plot(newCurveX[i],newCurveY[i], 'bo', ms= 1)

    #uncomment next line to get all the curves
    #scope = range(len(oldCurveX))
    scope = [4]
    #open one new window for each trajectory in the scope (scope correspond to the id of the trajectory)
    for index,j in enumerate(scope):
        plt.figure(index+2)    
        #old trajectory on global scale
        plt.subplot(221)
        plt.axis([-4,4,-4,4])
        plt.title('old trajectory')
        for i in range(len(geometryCurvesX)):
            plt.plot(geometryCurvesX[i],geometryCurvesY[i], 'r-')  
        plt.plot(oldCurveX[j],oldCurveY[j], 'bo', ms= 1)
        #new trajectory on global scale
        plt.subplot(222)
        plt.axis([-4,4,-4,4])
        plt.title('new trajectory')
        for i in range(len(geometryCurvesX)):
            plt.plot(geometryCurvesX[i],geometryCurvesY[i], 'r-')
        plt.plot(newCurveX[j],newCurveY[j], 'bo', ms= 1)
        #old trajectory seen from close
        plt.subplot(223)
        plt.axis([-1,1,-1,1])
        plt.title('old trajectory (closeup)')
        for i in range(len(geometryCurvesX)):
            plt.plot(geometryCurvesX[i],geometryCurvesY[i], 'r-')
        plt.plot(oldCurveX[j],oldCurveY[j], 'bo', ms= 1)
        plt.plot(oldCurveX[j],oldCurveY[j], 'k', lw=0.5)
        #new trajectory seen from close
        plt.subplot(224)
        plt.axis([-1,1,-1,1])
        plt.title('new trajectory (closeup)')
        for i in range(len(geometryCurvesX)):
            plt.plot(geometryCurvesX[i],geometryCurvesY[i], 'r-')     
        plt.plot(newCurveX[j],newCurveY[j], 'bo', ms= 1)
        plt.plot(newCurveX[j],newCurveY[j], 'k', lw=0.5)
        
        
    plt.show()

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
   geoData.append(["walls",direction,walls])
   print("Context: triangle with coordinates (5,0),(5,5),(-5,0)")
   x,y = handleSinglePoint(3.5,1.5,geoData)
   print("=3.1= (3.5,1.5), expected to become (3.5,1.5) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,1,geoData)
   print("=3.2= (4,1), expected to become (4,1) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,0.5,geoData)
   print("=3.3= (4,0.5), expected to become (4,0.5) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,0.25,geoData)
   print("=3.4= (4,0.25), expected to become (4,0.4333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,0,geoData)
   print("=3.5= (4,0), expected to become (4,0.3666) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,-0.5,geoData)
   print("=3.6= (4,-0.5), expected to become (4,0.2333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,-1,geoData)
   print("=3.7= (4,-1), expected to become (4,0.1) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4,-2,geoData)
   print("=3.8= (4,-2), expected to become (4,-2) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(4.75,0.25,geoData)
   print("=3.9= (4.75,0.25), expected to become (4.5666,0.4333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   x,y = handleSinglePoint(5.5,-0.5,geoData)
   print("=3.10= (5.5,-0.5), expected to become (4.7666,0.2333) == (%s,%s)"%(floatToString(x),floatToString(y)))
   
   print("DEBUG: debugging closeFromWallTriangle")
   d = closeFromWallTriangle(1,0.5,0,0,2,0)
   print("=4.1= is (1,0.5) close from (0,0),(2,0), expected 1 :",d)
   d = closeFromWallTriangle(1,1,0,0,2,0)
   print("=4.2= is (1,1) close from (0,0),(2,0), expected 1 :",d) 
   d = closeFromWallTriangle(1,2,0,0,2,0)
   print("=4.3= is (1,2) close from (0,0),(2,0), expected 0 :",d) 
   d = closeFromWallTriangle(0,1,0,0,2,0)
   print("=4.4= is (0,1) close from (0,0),(2,0), expected 0 :",d) 
   d = closeFromWallTriangle(0.3,0.2,0,0,2,0)
   print("=4.5= is (0.3,0.2) close from (0,0),(2,0), expected 1 :",d)
   d = closeFromWallTriangle(0.3,-0.2,0,0,2,0)
   print("=4.5= is (0.3,-0.2) close from (0,0),(2,0), expected 1 :",d) 
   
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
   trajData = readTrajData(pathfile,nametraj)
   geoData = readGeoData(pathfile,namegeo)
   
   
   
   new_trajectories = handleAllPoint(trajData,geoData)
   
   plotTrajectories(geoData,trajData,new_trajectories)
   
   writeNewFile(new_trajectories,pathfile,nametraj)
   
   
   #debugTest()
   
   

