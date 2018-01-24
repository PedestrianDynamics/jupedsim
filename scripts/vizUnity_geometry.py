try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


import logging
import numpy as np
import sys
from xml.dom import minidom

logfile='log.dat'
logging.basicConfig(filename=logfile, level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

g = open("geometry.txt", "w")

def usage():
    print("usage: python3 %s geometry_file trajectory_file"%sys.argv[0])
    print("This will create two files:\
               \n \t- Trajectories ---> b090_combined.txt \n \t- and geometry ---> geometry.txt")
    print("mv geometry.txt b090_combined.txt to the location where SumoVizUnity leaves")


def parse_trajectories(filename):
    """
    parse trajectories in JuPedSim-xml-format and output results
    in the following format:
    time id  x  y  z  level density
    (no sorting of the data is performed)
    returns:
    fps: frames per second
    N: number of pedestrians
    data: trajectories (numpy.array) [t, agent_id, x, y, z, level, density]
    """
    logging.info("parsing <%s>"%filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file. exit')
        exit()
    N = int(xmldoc.getElementsByTagName('agents')[0].childNodes[0].data)
    fps = xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data #type unicode
    fps = float(fps)
    logging.info("Npeds = %d, fps = %d"%(N, fps))
    frames = xmldoc.childNodes[0].getElementsByTagName('frame')
    data = []
    for frame in frames[::int(fps)]: # 1 frame per second.
        frame_number = int(frame.attributes["ID"].value)
        for agent in frame.getElementsByTagName("agent"):
            agent_id = int(agent.attributes["ID"].value)
            x = float(agent.attributes["x"].value)
            y = float(agent.attributes["y"].value)
            z = float(agent.attributes["z"].value)
            t = frame_number/fps
            level = 1     # @todo
            density = 0.1 # @todo
            data += [t, agent_id, x, y, z, level, density]
    data = np.array(data).reshape((-1, 7))
    return fps, N, data


def get_polygon(poly):
    X = []
    Y = []
    for p in poly.getchildren(): # vertex
        X.append(p.attrib['px'])
        Y.append(p.attrib['py'])

    return X, Y

def write_polygon(X, Y, Type, name, height):
    print("Polygon", end=' ', file=g)
    for (x, y) in zip(X, Y):
        print("%.1f %.1f"%(float(x), float(y)), end=' ', file=g)

    print("%s %s %.2f"%(name, Type, height), file=g)

def setMinMax(X, Y, minX, maxX, minY, maxY):

    if float(min(X)) < minX:
        minX = min(X)

    if float(min(Y)) < minY:
        minY = min(Y)

    if float(max(X)) > maxX:
        maxX = max(X)

    if float(max(Y)) > maxY:
        maxY = max(Y)

    return float(minX), float(maxX), float(minY), float(maxY)

def parse_geometry(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    npoly = 0
    nobs = 0
    minX = 10000
    maxX = -10000
    minY = 10000
    maxY = -10000

    for node in root.iter():

        tag = node.tag
        # print "subroom tag", tag
        if tag == "polygon":
            npoly += 1
            X, Y = get_polygon(node)

            minX, maxX, minY, maxY = setMinMax(X, Y, minX, maxX, minY, maxY)
            write_polygon(X, Y, "wall", "w%d"%npoly, 2)
        elif tag == "obstacle":
            # print "obstacle tag",tag
            for n in node.getchildren():
                nobs += 1
                X, Y = get_polygon(n)
                write_polygon(X, Y, "obstacle", "o%d"%nobs, 1)
                # print "obstacle", X, Y

    # some trees
    dx = 5
    dy = 5
    print("Polygon %.1f %.1f t1 tree 1.0"%(minX-dx, minY-dy), file=g)
    print("Polygon %.1f %.1f t3 tree 1.0"%(maxX+dx, maxY+dy), file=g)

if __name__ == "__main__":
    if len(sys.argv) <= 2:
        usage()
        exit()

    open("log.dat", "w").close() # touch logfile
    geometry_filename = sys.argv[1]
    trajectory_filename = sys.argv[2]
    logging.info("Geometry file: %s"%(geometry_filename))
    logging.info("Trajectory file: %s"%(trajectory_filename))

    parse_geometry(geometry_filename)

    fps, N, data = parse_trajectories(trajectory_filename)
    np.savetxt("b090_combined.txt", data, fmt="%.1f %d %.1f %.1f %.1f %d %.1f")
