import logging
import numpy as np
import pandas as pd
from xml.dom import minidom

SUCCESS = 0
FAILURE = -1
critical_value = 0.9


try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


def get_polygon(poly):
    X = []
    Y = []
    # for poly in node.getchildren():
    #     # print "polygon tag: ", poly.tag
    #     # if poly.tag == "obstacle":
    #     #     # print poly.getchildren()
    #     #     # for pobst in poly.getchildren():
    #     #     #     # print pobst.tag
    #     #     #     for q in pobst.getchildren(): # vertex
    #     #     #         X.append( q.attrib['px'] )
    #     #     #         Y.append( q.attrib['py'] )
    #     #     pass
    #     # else:
    for p in poly.getchildren(): # vertex
        X.append(p.attrib['px'])
        Y.append(p.attrib['py'])

    return X, Y

def plot_geometry(filename):
    tree = ET.parse(geometry)
    root = tree.getroot()
    for node in root.iter():
        tag = node.tag
        # print "subroom tag", tag
        if tag == "polygon":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "k", lw=2)
        elif tag == "obstacle":
            # print "obstacle tag",tag
            for n in node.getchildren():
                # print "N: ", n
                X, Y = get_polygon(n)
                # print "obstacle", X, Y
                plt.plot(X, Y, "g", lw=2)
        elif tag == "crossing":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--b", lw=0.9, alpha=0.2)
        elif tag == "HLine":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--b", lw=0.9, alpha=0.2)
        elif tag == "transition":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--r", lw=1.6, alpha=0.2)




def is_inside(trajectories, left, right, down, up):
    """
    up --> . _____.
           |      |
  down --> . -----.
           ^      ^
          left   right
    """
    condition = (trajectories[0, 2] > left) & \
                (trajectories[0, 2] < right) & \
                (trajectories[0, 3] > down) & \
                (trajectories[0, 3] < up)
    return condition.all()

def PassedLine(p, e):
    """
    check if pedestrian (given by matrix p) passed the line [x1, x2, y1, y2] with x1<x2 and  y1<y2
    """
    assert (isinstance(e, list) or isinstance(e, np.ndarray)) and len(e) == 4,\
        "exit should be a list with len=4"

    x1 = e[0]
    y1 = e[1]
    x2 = e[2]
    y2 = e[3]
    A = np.array([x1, y1])
    B = np.array([x2, y2])

    is_left_and_right = (any(np.cross(B-A, p[:, 2:]-A)) < 0) & (any(np.cross(B-A, p[:, 2:]-A)) > 0)
    is_between = any(np.dot(p[:, 2:]-A, B-A) > 0) & any(np.dot(p[:, 2:]-B, A-B) > 0)

    return  is_left_and_right and is_between


def PassedLineX(p, exit):
    """
    check if pedestrian (given by matrix p) passed the vertical line x, [y1, y2] y1<y2
    """
    eps = 0.1 # fps should be big enough, otherwise eps maybe too bis for a guess.
    x = exit[0]
    y1 = exit[1]
    y2 = exit[2]

    X = p[:, 2]
    Y = p[:, 3]

    return (np.logical_and(np.logical_and(X >= x-eps, X <= x+eps), np.logical_and(Y >= y1, Y <= y2) ) ).any()


def PassedLineY(p, exit):
    """
    check if pedestrian (given by matrix p) passed the horizontal line y, [x1, x2] x1<x2
    """
    eps = 0.1 # fps should be big enough, otherwise eps maybe too bis for a guess.
    y = exit[0]
    x1 = exit[1]
    x2 = exit[2]

    X = p[:, 2]
    Y = p[:, 3]

    return (np.logical_and(np.logical_and(Y >= y-eps, Y <= y+eps), np.logical_and(X >= x1, X <= x2) ) ).any()

def get_num_threads(filename):
    """
    get num_threads
    """
    logging.info("parsing <%s>"%filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file %s. exit'%filename)
        exit(FAILURE)
    num_threads = float(xmldoc.getElementsByTagName('num_threads')[0].firstChild.nodeValue)
    return num_threads

def get_maxtime(filename):
    """
    get max sim time
    """
    logging.info("parsing <%s>"%filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file %s. exit'%filename)
        exit(FAILURE)
    maxtime = float(xmldoc.getElementsByTagName('max_sim_time')[0].firstChild.nodeValue)
    return maxtime

def get_outflow(filename):
    """
    get outflow of doors
    """
    tree = ET.parse(filename)
    root = tree.getroot()
    ids = []
    outflow = []
    for node in root.iter():
        tag = node.tag
        if tag == "traffic_constraints":
            for doors in node.getchildren(): # doors
                for door in doors.getchildren(): # door
                    ids.append(int(door.attrib['trans_id']))
                    outflow.append(float(door.attrib['outflow']))
    return ids, outflow

def parse_file(filename):
    """
    parse trajectories in Travisto-format and output results
    in the following  format: id    frame    x    y
    (no sorting of the data is performed)
    returns:
    fps: frames per second
    N: number of pedestrians
    data: trajectories (numpy.array) [id fr x y]
    """
    logging.info("parsing <%s>"%filename)

    tree = ET.parse(filename)
    root = tree.getroot()

    for header in root.iter('header'):
        fps = header.find('frameRate').text

    try:
        fps = float(fps)
    except:
        print ("ERROR: could not read <fps>")
        exit()

    for header in root.iter('header'):
        N = header.find('agents').text

    try:
        N = int(N)
    except:
        print ("ERROR: could not read <agents>")
        exit()

    data = np.empty(shape=[0, 5])
    for node in root.iter():
        tag = node.tag
        if tag == "frame":
            frame = node.attrib['ID']
            for agent in node.getchildren():
                x = agent.attrib['x']
                y = agent.attrib['y']
                z = agent.attrib['z']
                ID = agent.attrib['ID']
                data = np.vstack((data, list(map(float, [ID, frame, x, y, z]) ) ) )


                # data += [ID, frame, x, y, z]

    # data = np.array(data, dtype=float).reshape((-1, 5))
    return fps, N, data

def parse_file_deprecated(filename):
    """
    parse trajectories in Travisto-format and output results
    in the following  format: id    frame    x    y
    (no sorting of the data is performed)
    returns:
    fps: frames per second
    N: number of pedestrians
    data: trajectories (numpy.array) [id fr x y]
    """
    logging.info("parsing <%s>"%filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file. exit')
        exit(FAILURE)
    N = int(xmldoc.getElementsByTagName('agents')[0].childNodes[0].data)
    fps = xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data #type unicode
    fps = float(fps)
    fps = int(fps)
    #print "fps=", fps
    #fps = int(xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data)
    logging.info("Npeds = %d, fps = %d"%(N, fps))
    frames = xmldoc.childNodes[0].getElementsByTagName('frame')
    data = []
    for frame in frames:
        frame_number = int(frame.attributes["ID"].value)
        for agent in frame.getElementsByTagName("agent"):
            agent_id = int(agent.attributes["ID"].value)
            x = float(agent.attributes["x"].value)
            y = float(agent.attributes["y"].value)
            data += [agent_id, frame_number, x, y]
    data = np.array(data).reshape((-1, 4))
    return fps, N, data


def rolling_flow(fps, N, data, x0, name):
    """
    measure the flow at a vertical line given by <x0>
    trajectories are given by <data> in the following format: id    frame    x    y
    input:
    - fps: frame per second
    - N: number of peds
    - data: trajectories
    - x0: x-coordinate of the vertical measurement line
    output:
    - flow
    """
    logging.info('Measure flow at %f'%x0)
    if not isinstance(data, np.ndarray):
        logging.critical("flow() accepts data of type <ndarray>. exit")
        exit(FAILURE)
    peds = np.unique(data[:, 0]).astype(int)
    times = []
    for ped in peds:
        d = data[data[:, 0] == ped]
        passed = d[d[:, 2] >= x0]
        if passed.size == 0:  # pedestrian did not pass the line
            logging.critical("Pedestrian <%d> did not pass the line at <%.2f>"%(ped, x0))
            exit(FAILURE)
        first = min(passed[:, 1])
        #print "ped= ", ped, "first=",first
        times.append(first)
    if len(times) < 2:
        logging.warning("Number of pedestrians passing the line is small. return 0")
        return 0

    times = np.sort(times)
    serie = pd.Series(times)
    minp = 100; windows = int(len(times)/10);
    minp = min(minp, windows)
    flow = fps*(windows-1)/(serie.rolling(windows, min_periods=minp).max()  - serie.rolling(windows, min_periods=minp).min() )
    flow = flow[~np.isnan(flow)] # remove NaN
    wmean = flow.rolling(windows, min_periods=minp).mean()

    np.savetxt(name, np.array(times))

    logging.info("min(times)=%f max(times)=%f"%(min(times), max(times)))
    return np.mean(wmean) #fps * float(N-1) / (max(times) - min(times))


def flow(fps, N, data, x0):
    """
    measure the flow at a vertical line given by <x0>
    trajectories are given by <data> in the following format: id    frame    x    y
    input:
    - fps: frame per second
    - N: number of peds
    - data: trajectories
    - x0: x-coordinate of the vertical measurement line
    output:
    - flow
    """
    logging.info('Measure flow at %f'%x0)
    if not isinstance(data, np.ndarray):
        logging.critical("flow() accepts data of type <ndarray>. exit")
        exit(FAILURE)
    peds = np.unique(data[:, 0]).astype(int)
    times = []
    for ped in peds:
        d = data[data[:, 0] == ped]
        passed = d[d[:, 2] >= x0]
        if passed.size == 0:  # pedestrian did not pass the line
            logging.critical("Pedestrian <%d> did not pass the line at <%.2f>"%(ped, x0))
            exit(FAILURE)
        first = min(passed[:, 1])
        #print "ped= ", ped, "first=",first
        times.append(first)
    if len(times) < 2:
        logging.warning("Number of pedestrians passing the line is small. return 0")
        return 0

    logging.info("min(times)=%f max(times)=%f"%(min(times), max(times)))
    return fps * float(N-1) / (max(times) - min(times))

def CalcBiVarCDF(x,y,xGrid,yGrid):
    """
    Calculate the bivariate CDF of two given input signals on predefined grids.
    input:
      - x: array of size n1
      - y: array of size n2
      - xGrid: array of size m1
      - yGrid: array of size m2
    output:
      - CDF2D: matrix
    """
    nPoints = np.size(x);
    xGridLen = np.size(xGrid);
    yGridLen = np.size(yGrid);
    CDF2D = np.zeros([xGridLen,yGridLen]);
    for i in range(xGridLen):
        for j in range(yGridLen):
            for k in range(nPoints):
                if ((x[k] <= xGrid[i]) and (y[k] <= yGrid[j])):
                     CDF2D[i,j] += 1;

    CDF2D = CDF2D / nPoints;
    return CDF2D


def CDFDistance(x1, y1, x2, y2):
    """
    For two input 2D signals calculate the distance between their CDFs.
    input:
      - x1: array of size n
      - y2: array of size n
      - x2: array of size m
      - y2: array of size m
    output:
      - KSD: not negative number
    """
    xPoints = 100;
    yPoints = 100;
    x = np.hstack((x1, x2))
    xCommonGrid = np.linspace(np.min(x), np.max(x), xPoints);
    y = np.hstack((y1, y2))
    yCommonGrid = np.linspace(np.min(y), np.max(y), yPoints);
    CDF1 = CalcBiVarCDF(x1,y1,xCommonGrid,yCommonGrid);
    CDF2 = CalcBiVarCDF(x2,y2,xCommonGrid,yCommonGrid);
#    KSD = np.linalg.norm(CDF1-CDF2)/(np.linalg.norm(CDF1)+np.linalg.norm(CDF1); # Frobenius norm (p=2)
    KSD = np.max(np.abs(CDF1-CDF2)); # Kolmogorov-Smirnov distance (p=inf)
    return KSD
