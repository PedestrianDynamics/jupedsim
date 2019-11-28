 #!/usr/bin/python
import numpy as np
import os, sys, glob, math
from scipy import interpolate
from scipy.misc import comb
import logging, types, argparse

def getParserArgs():
    parser = argparse.ArgumentParser(description='Convert the experimental data to TraVisTo-format. Optionally smooth the trajectories')
    parser.add_argument("-s", "--smoth", action='store_const', const=1, default=0 , help='smooth the trajectories. Takes a bit longer')
    # action='store_const', const=1, default=0
    parser.add_argument("-f", "--fps", type=int , default=10, help='frames per seconds (default 10)')
    parser.add_argument("-m", "--m", action='store_const', const=0, default=1, help='trajectories are in m (default cm)')
    parser.add_argument("-d", "--df", type=int , default=10, help='Delta (frames) to calculate velocity  (default 10)')
    parser.add_argument("-l", "--log" , type=argparse.FileType('w'), default='log.dat', help="log file (default log.dat)")
    parser.add_argument("-p", "--path", default=".", help='give the directory of the trjectory files')
    args = parser.parse_args()
    return args


def Projection(a, b, c):
    """
    Projection of point c on segment [a,b]
    a, b and c are numpy.arrays
    """
    t = b - a                    # Vector ab
    dd = np.sqrt(sum(t**2))         # Length of ab
    if dd < 0.0001:
        return a
    t = t/dd                     # unit vector of ab
    #n = array( [-t[1], t[0] ] )  # normal unit vector to ab
    ac =  c - a                  # vector ac
    return a + sum(ac*t)*t       # OC = OA + AP; AP = ap*T


def within(a, b, c, eps=0.5):
    """
    Return true iff r is between p and q (inclusive).
    """

    if isinstance(a,types.ListType) and isinstance(b, types.ListType) and isinstance(c, types.ListType):
        # print "a",a
        # print "b",b
        # print "c",c
        # print "return ",a <= c <= b or b <= c <= a
        #return a <= c <= b or b <= c <= a
        if abs(min(a[0], b[0]) - max(a[0], b[0]))< eps:
            return min(a[0], b[0]) <= c[0] <= max(a[0], b[0])
        elif abs(min(a[1], b[1]) - max(a[1], b[1]))< eps:
            return min(a[1], b[1]) <= c[1] <= max(a[1], b[1])
        else:
            return min(a[0], b[0]) <= c[0] <= max(a[0], b[0])  and min(a[1], b[1]) <= c[1] <= max(a[1], b[1])
    else:
        logging.debug("input arguments: ")
        logging.debug("a, %s"%type(a))
        logging.debug("b, %s"%type(b))
        logging.debug("c, %s"%type(c))
        exit(logging.critical("within accepts <list>-arguments. Exit with error"))
        

def getNullpoints(path, lookup=5, nparts=3):
    """
    get nullpoints from <nparts> of path.
    """
    h = 1
    shift = int( len(path)/float(nparts) )
    start = 0
    to = start + shift
    once = 1            
    while to <= path.shape[0]:
        part_path = path[start:to, :]
        tmp = FindNullPoints(part_path, lookup)
        if len(tmp) <2: # no oscillations -> take the trajectory as it is
            if once:
                nullPoints = part_path[::h, :]
                once = 0
            else:
                nullPoints = np.vstack( ( nullPoints, part_path[::h, :] ) )
        else: # some oscillations in there
            if once:
                nullPoints = tmp
                once = 0
            else:
                nullPoints = np.vstack( ( nullPoints, tmp ) )
        start = to
        to = start + shift
        
    if len(nullPoints[:,1])>=2:
        nullPoints = Sort_Distance(nullPoints)
        #get part of trajectory between two distant points
        p_end=[]
        for (p,q) in zip(nullPoints[:-1, 1:],nullPoints[1:,1:]):
            distance = np.sum( (p-q)**2)
            if distance > 5000:
                p1 = path[ np.array([within(list(p), list(q), list(b) ) for b in path[:,1:]]) ]
                if len(p_end)==0:
                    p_end = p1[::5,:]
                else:
                    p_end = np.vstack( ( p_end, p1[::5,:] ) )
        if len(p_end) != 0:
            nullPoints = np.vstack( (nullPoints,p_end) )
            nullPoints = Sort_Distance(nullPoints)
        
        nullPoints = np.vstack( (path[0],nullPoints) )
        nullPoints = np.vstack( (nullPoints, path[-1]) )
    return nullPoints
    
def Sort_Distance(nullPoints):
    """
    sort nullPoints distance-wise
    """
    for i in range( len(nullPoints[:,1]) -1 ):
        minDistance = 10000000      
        logging.debug("i=%d, %s"%(i, ', '.join(map(str,nullPoints[i,1:]))))
        index = i+1
        for j in range(i+1,len(nullPoints[:,1])):
            logging.debug("  j=%d, %s"%(j, ', '.join(map(str,nullPoints[j,1:]))))
            distance = np.sum((nullPoints[i,1:] - nullPoints[j,1:])**2)
            logging.debug("   distance=%.3f"%distance)
            if distance < minDistance:
                minDistance = distance
                index = j
                logging.debug("\t\t-->index=%d"%index)
        logging.debug("final index=%d"%index)
        if index != i+1:
            tmp = nullPoints[i+1,:].copy()  
            nullPoints[i+1,:] = nullPoints[index,:]
            nullPoints[index,:] = tmp
            logging.debug("+++++SWAP++++")

    return nullPoints

def FindNullPoints(path,lookup=10):
   """
      Find the null points in the path.
      Find the vector from 'forward' to 'back' and from 'forward'
       to the 'middle' point
      When we are at a null point, these lie along the same vector.
   """
   middle = path[lookup:-1*lookup,1:3]
   back = path[:-2*lookup,1:3]
   forward = path[2*lookup:,1:3]

   s = forward - back
   d = forward - middle
   sn =  np.array((s[:,1],-s[:,0])).swapaxes(0,1)  #Vectors normal to 's'

   cosangle = (d*sn).sum(axis=1)                #Dot products of 'd' and 'sn'
   sign_changes =  cosangle[:-1]*cosangle[1:] < 0 #When this is zero a sign change occurs
   m =  path[lookup:-1*lookup:]
   #   print m
   #print sign_changes
   if sign_changes.any():
       nullpoints = m[sign_changes]
   else:
       nullpoints = m
   return nullpoints

def interpolate_polyline(polyline, smooth, num_points=100):
    """
    given an array of points find and delete duplicate points. Use
    the rest to calculate a spline
    """
    duplicates = []
    for i in range(1, len(polyline)):
        if np.allclose(polyline[i], polyline[i-1]):
            duplicates.append(i)
    if duplicates:
        polyline = np.delete(polyline, duplicates, axis=0)
    tck, u = interpolate.splprep(polyline.T, s=smooth)
    u = np.linspace(0.0, 1.0, num_points)
    return np.column_stack(interpolate.splev(u, tck))

def getTrajectory(nullpoints, path):
    traj = np.array([])
    for p in path:
        x = float(p[2])
        y = float(p[1])
        xNow, xBevor, xAfter = getBevorAfter(nullpoints, x, y)
        traj = np.append(traj, xNow)
    return traj.reshape(path.shape[0], 2)
    
def getBevorAfter(nullpoints, x, y):
    minDistance = 10000000
    actual_position = np.array([x,y])
    logging.debug("actual position (%.2f %.2f)"%(x, y))
    for i in range( len(nullpoints[:,0])):
        distance = np.sum((nullpoints[i,:] - actual_position )**2)
        if distance < minDistance:
            index = i
            minDistance = distance
    if index + 1 < len(nullpoints[:,0]):
        xAfter = nullpoints[index+1, :]
    else:
        xAfter = actual_position
    if index - 1 > 0:
        xBevor = nullpoints[index-1, :]
    else:
        xBevor = actual_position
    xNow = Projection(xAfter, xBevor, actual_position )
    return xNow, xBevor, xAfter

def getSpeed(data, agent, frame, fps=16, df=10, cm=0.01):
    """
    get speed of pedestrian <agent> at <frame> 
    cm=1 if trajectories are in meter. cm=0.01 if trajectories are in cm
    """
    d_ped =  data [ data[:,0] == agent ]                       
    if frame + df <= max(d_ped[:,1]) and frame - df >= min(d_ped[:,1]): 
        xAfter = d_ped[ d_ped[:,1] == (frame + df) ][0,2:4]
        xBevor = d_ped[ d_ped[:,1] == (frame - df) ][0,2:4]
        v =  cm * fps * np.sqrt( np.sum( (xAfter - xBevor )**2 ) )     / (2.0*df) # Vxy: xAfter and xBevor are vectors
    elif frame - df >= min(d_ped[:,1]):
        xAfter = d_ped[ d_ped[:,1] == frame ][0,2:4]
        xBevor = d_ped[ d_ped[:,1] == (frame - df) ][0,2:4]
        v =  cm * fps * np.sqrt(   np.sum( (xAfter - xBevor )**2 ) )   / (1.0*df) # Vxy: xAfter and xBevor are vectors 
    elif frame + df <= max(d_ped[:,1]):
        xAfter = d_ped[ d_ped[:,1] == (frame + df)][0,2:4]
        xBevor = d_ped[ d_ped[:,1] == frame ][0,2:4]
        v =  cm * fps * np.sqrt(   np.sum( (xAfter - xBevor )**2 ) )   / (1.0*df) # Vxy: xAfter and xBevor are vectors 
    else:
        logging.warning("agent %d lives less than %d frames"%(agent, df))
        #logging.warning("agent=%d, fr=%d (df=%d), min_f=%d, max_fr=%d"%(agent,frame,df,min(d_ped[:,1]), max(d_ped[:,1])))
        xAfter = d_ped[ d_ped[:,1] == max(d_ped[:,1]) ][0,2:4]
        xBevor = d_ped[ d_ped[:,1] == min(d_ped[:,1]) ][0,2:4]
        v =  cm * fps * np.sqrt(   np.sum( (xAfter - xBevor )**2 ) )   / (1.0*df) # Vxy: xAfter and xBevor are vectors
        #v = 0

    return v
#========================================================================================================
def write_header(out, Nagents, fps):
    out.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    out.write("<trajectories>\n\n")
    # write header
    out.write("<header version = \"0.6\">\n")
    out.write("\t<agents>%d</agents>\n"%Nagents)
    out.write("\t<seed>12542</seed>\n")
    out.write("\t<frameRate>%d</frameRate>\n"%fps)
    out.write("</header>\n") 

def write_geometry(out, walls):
    out.write("<geometry>\n")
    # ##########################################################
    # # Todo: get somehow the geometry data
    # ##########################################################
    #KO 300
    for (p1, p2) in zip(walls[:-1], walls[1:]):
        out.write("\t<wall>")
        out.write("\t\t<point xPos=\"%.2f\"  yPos=\"%.2f\"/>"%(p1[0], p1[1]))
        out.write("\t\t<point xPos=\"%.2f\"  yPos=\"%.2f\"/>"%(p2[0], p2[1]))
        out.write("\t</wall>\n")
    out.write("</geometry>\n")   



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
        # if frame%100 == 0:
        #     logging.info("++ frame:\t %d"%frame)
        d = data [ data[:,1] == frame ] # get data framewise
#================== begin write frame ==============
        out.write("<frame ID=\"%d\">\n"%frame)
        
        for (agent, x, y) in zip(d[:,0].astype(int), d[:,2], d[:,3]):        

            text = "\t<agent ID=\"%d\" \tx=\"%.2f\"\ty=\"%.2f\"\trA=\"%.2f\"\trB=\"%.2f\"\t eO=\"%.2f\"\teC=\"%d\"/>\n"%(agent, x*mTocm*0.01, y*mTocm*0.01, 0.20, 0.20, 0, 200)
            out.write(text)
            
        out.write("</frame>\n")
    #================== end write frame ================
