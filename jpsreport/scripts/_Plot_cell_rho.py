# todo https://stackoverflow.com/questions/12881848/draw-polygons-more-efficiently-with-matplotlib
import os
import sys
import logging
import argparse
from xml.dom import minidom
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

import numpy as np
#matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
import matplotlib.cm as cm

import Polygon as pol
#import pylab
import pandas as pd
from mpl_toolkits.axes_grid1 import make_axes_locatable

logfile = "rho_log.txt"
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')

def getParserArgs():
    parser = argparse.ArgumentParser(description='Combine French data to one file')
    parser.add_argument("-f", "--filepath", default="./", help='give the path of source file')
    parser.add_argument("-n", "--namefile", help='give the name of the source file')
    parser.add_argument("-g", "--geoname", help='give the name of the geometry file')
    parser.add_argument("-p", "--trajpath", help='give the path of the trajectory file')
    parser.add_argument("-i", "--index", action='store_const', const=True,
                        default=False, help='plot index of pedestrians along with the Voronoi polygons')
    args = parser.parse_args()
    return args

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
        X.append(float(p.attrib['px']))
        Y.append(float(p.attrib['py']))

    return X, Y

def get_geometry_boundary(geometry):
    tree = ET.parse(geometry)
    root = tree.getroot()
    geominX = []
    geomaxX = []
    geominY = []
    geomaxY = []
    for node in root.iter():
        tag = node.tag
        if tag == "polygon":
            X, Y = get_polygon(node)
            geominX.append(min(X))
            geomaxX.append(max(X))
            geominY.append(min(Y))
            geomaxY.append(max(Y))
        elif tag == "obstacle":
            for n in node.getchildren():
                X, Y = get_polygon(n)
                geominX.append(min(X))
                geomaxX.append(max(X))
                geominY.append(min(Y))
                geomaxY.append(max(Y))
    geominX = min(geominX)
    geomaxX = max(geomaxX)
    geominY = min(geominY)
    geomaxY = max(geomaxY)
    return geominX, geomaxX, geominY, geomaxY

def plot_geometry(geometry):
    tree = ET.parse(geometry)
    root = tree.getroot()
    for node in root.iter():
        tag = node.tag
        if tag == "polygon":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "k", lw=4)
        elif tag == "obstacle":
            for n in node.getchildren():
                X, Y = get_polygon(n)
                plt.plot(X, Y, "g", lw=4)
        elif tag == "crossing":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--b", lw=0.9, alpha=0.2)
        elif tag == "HLine":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--b", lw=0.9, alpha=0.2)
        elif tag == "transition":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--r", lw=1.6, alpha=0.2)


def parse_xml_traj_file(filename):
    """
    parse trajectories in jpsvis-format and output results
    in the following  format: id    frame    x    y
    (no sorting of the data is performed)
    returns:
    fps: frames per second
    N: number of pedestrians
    data: trajectories (numpy.array) [id fr x y]
    """
    logging.info("parsing <%s>", filename)
    try:
        xmldoc = minidom.parse(filename)
    except:
        logging.critical('could not parse file. exit')
        exit(0)
    N = int(xmldoc.getElementsByTagName('agents')[0].childNodes[0].data)
    fps = xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data #type unicode
    fps = round(float(fps))

    #print ("fps=", fps)
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

#@profile
def main():
    rho_max = 8.0
    args = getParserArgs()
    filepath = args.filepath
    sys.path.append(filepath)
    namefile = args.namefile
    geoFile = args.geoname
    trajpath = args.trajpath
    #geoLocation = filepath.split("Output")[0]
    trajName = namefile.split(".")[0]
    trajType = namefile.split(".")[1].split("_")[0]
    trajFile = os.path.join(trajpath, trajName+"."+trajType) #trajpath+trajName+"."+trajType
    plotIndex = args.index
    logging.info("plot_cell_rho trajpath %s", trajpath)
    logging.info("plot_cell_rho  trajName %s", trajName+"."+trajType)
    logging.info("plot_cell_rho trajFile %s", trajFile)
    try:
        frameNr = int(namefile.split("_")[-1])
    except ValueError:
        exit("ERROR: Could not parse fps")

    geominX, geomaxX, geominY, geomaxY = get_geometry_boundary(geoFile)
    fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
    ax1 = fig.add_subplot(111, aspect='equal')
    #plt.rc("font", size=30)
    #plt.rc('pdf', fonttype=42)
    #ax1.set_yticks([int(1*j) for j in range(-2, 5)])
    #for label in ax1.get_xticklabels() + ax1.get_yticklabels():
    #    label.set_fontsize(30)

    #for tick in ax1.get_xticklines() + ax1.get_yticklines():
    #    tick.set_markeredgewidth(2)
    #    tick.set_markersize(6)
    #    ax1.set_aspect("equal")

    plot_geometry(geoFile)
    density = np.array([])
    density_orig = np.array([])
    polygons = [] # polygons converted from string
    polygon_path = os.path.join(filepath, "polygon")
    polygon_filename = os.path.join(polygon_path, '.'.join((namefile, "dat")))
    if(not os.path.exists(polygon_filename)):
        sys.exit("ERROR %s:  File does not exist. <%s>"%(argv[0], polygon_filename))

    File = open(polygon_filename)
    polys = File.readlines()
    File.close()
    # plot the blue dots (head) first, to avoid intervening with "text"
    # ---------------------------------------------------
    # if  not plotIndex:
    #     if(trajType == "xml"):
    #         fps, N, Trajdata = parse_xml_traj_file(trajFile)
    #     elif(trajType == "txt"):
    #         try:
    #             Trajdata = np.array(pd.read_csv(trajFile, comment="#", delimiter=r"\s+"))
    #         except:
    #             Trajdata = np.loadtxt(trajFile)

    #     Trajdata = Trajdata[Trajdata[:, 1] == frameNr]
    #     ax1.plot(Trajdata[:, 2], Trajdata[:, 3], "bo", markersize=20, markeredgewidth=2)
    # ---------------------------------------------------

    #polys = open("%s/polygon%s.dat"%(filepath,namefile)).readlines()
    poly_index = []
    areas = []
    for poly in polys:
        poly = poly.split("|")
        poly_index.append(poly[0].strip())
        Poly = poly[1].strip()
        exec("p = %s"%Poly)
        pp = locals()['p']
        polygons.append(pp)
        area = pol.Polygon(pp).area()
        xx = 1.0/area
        if xx > rho_max:
            xx = rho_max

        density = np.append(density, xx)
        areas.append(area)

    density_orig = np.copy(density)
    Maxd = density.max()
    Mind = density.min()
    erro = np.ones(np.shape(density))*Mind
    density = rho_max*(density-erro)/(Maxd-Mind)
    sm = cm.ScalarMappable(cmap=cm.jet)
    sm.set_array(density)
    sm.autoscale_None()
    sm.set_clim(vmin=0, vmax=10)
    maxArea = np.max(areas)
    meanArea = np.mean(areas)
    for j, poly in enumerate(polys):
        ax1.add_patch(pgon(polygons[j], fc=sm.to_rgba(density_orig[j]), ec='white', lw=2))
        bcolor = sm.to_rgba(density_orig[j]) #inverse background color
        icolor = [1 - c for c in bcolor]
        icolor[-1] = bcolor[-1] # alpha
        if plotIndex:
            ax1.text(pol.Polygon(polygons[j]).center()[0],
                     pol.Polygon(polygons[j]).center()[1],
                     poly_index[j],
                     fontsize=25*areas[j]/maxArea, color=icolor)

    ax1.set_xlim(geominX-0.2, geomaxX+0.2)
    ax1.set_ylim(geominY-0.2, geomaxY+0.2)
    plt.xlabel("x [m]")
    plt.ylabel("y [m]")
    plt.axhline(y=2.5)
    plt.axvline(x=0.0)
    plt.title("%s"%namefile)
    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", size="2.5%", pad=0.2)
    cb = fig.colorbar(sm, ax=ax1, cax=cax, format='%.1f')
    cb.set_label('Density [$m^{-2}$]')
    figname = os.path.join(filepath, "rho_"+namefile+".png")
    logging.info("SAVE: %s", figname)
    plt.savefig(figname)
    plt.close()


if __name__ == '__main__':
    main()
