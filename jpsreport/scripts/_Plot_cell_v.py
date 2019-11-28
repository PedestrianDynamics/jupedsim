#!/usr/bin/env python3
import argparse
import sys
import os
import logging
import numpy as np

import Polygon as pol

import matplotlib.cm as cm
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon

#import pylab
from mpl_toolkits.axes_grid1 import make_axes_locatable

from _Plot_cell_rho import get_geometry_boundary
from _Plot_cell_rho import parse_xml_traj_file
from _Plot_cell_rho import plot_geometry

logfile = "v_log.txt"
logging.basicConfig(filename=logfile,
                    level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')


def getParserArgs():
    parser = argparse.ArgumentParser(description='Plot Voronoi cell. Color depends on velocity')
    parser.add_argument("-f", "--filepath", default="./", help='give the path of source file')
    parser.add_argument("-n", "--namefile", help='give the name of the source file')
    parser.add_argument("-g", "--geoname", help='give the name of the geometry file')
    parser.add_argument("-p", "--trajpath", help='give the path of the trajectory file')
    parser.add_argument("-i", "--index",
                        action='store_const',
                        const=True,
                        default=False,
                        help='plot index of pedestrians along with the Voronoi polygons')
    arguments = parser.parse_args()
    return arguments

if __name__ == '__main__':
    args = getParserArgs()
    filepath = args.filepath
    sys.path.append(filepath)
    namefile = args.namefile
    geoFile = args.geoname
    trajpath = args.trajpath
    geoLocation = filepath.split("Output")[0]
    trajName = namefile.split(".")[0]
    trajType = namefile.split(".")[1].split("_")[0]
    trajFile = os.path.join(trajpath, trajName+"."+trajType)
    print("plot_cell_v trajpath %s" %trajpath)
    print("plot_cell_v trajName %s"%trajName+"."+trajType)
    print("plot_cell_v trajFile %s" % trajFile)

    frameNr = int(namefile.split("_")[-1])
    geominX, geomaxX, geominY, geomaxY = get_geometry_boundary(geoFile)

    fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
    ax1 = fig.add_subplot(111, aspect='equal')
    plt.rc("font", size=30)
    ax1.set_yticks([int(1.00*j) for j in range(-2, 5)])
    for label in ax1.get_xticklabels() + ax1.get_yticklabels():
        label.set_fontsize(30)
    for tick in ax1.get_xticklines() + ax1.get_yticklines():
        tick.set_markeredgewidth(2)
        tick.set_markersize(6)
    ax1.set_aspect("equal")
    plot_geometry(geoFile)

    velocity = np.array([])
    polygon_path = os.path.join(filepath, "polygon")
    polygon_filename = os.path.join(polygon_path, '.'.join((namefile, "dat")))
    if(not os.path.exists(polygon_filename)):
        sys.exit("ERROR %s:  File does not exist. <%s>"%(argv[0], polygon_filename))

    File = open(polygon_filename)
    polys = File.readlines()
    velocity_path = os.path.join(filepath, "speed")
    velocity_filename = os.path.join(velocity_path, '.'.join((namefile, "dat")))
    if(not os.path.exists(velocity_filename)):
        sys.exit("ERROR %s:  File does not exist. <%s>"%(argv[0], velocity_filename))

    velocity = np.loadtxt(velocity_filename)
    sm = cm.ScalarMappable(cmap=cm.jet)
    sm.set_array(velocity)
    sm.autoscale_None()
    sm.set_clim(vmin=0, vmax=1.5)
    index = 0
    for poly in polys:
        poly = poly.split("|")
        poly_index = poly[0].strip()
        Poly = poly[1].strip()
        exec("p = %s"%Poly)
        xx = velocity[index]
        index += 1
        ax1.add_patch(pgon(p,facecolor=sm.to_rgba(xx), edgecolor='white',linewidth=2))
        # todo
        #if plotIndex:
        # ax1.text(pol.Polygon(polygons[j]).center()[0], pol.Polygon(polygons[j]).center()[1], poly_index,
        #          horizontalalignment='center',
        #          verticalalignment='center',
        #          fontsize=20, color='red',
        #          transform=ax1.transAxes)

    if(trajType == "xml"):
        fps, N, Trajdata = parse_xml_traj_file(trajFile)
    elif(trajType == "txt"):
        Trajdata = np.loadtxt(trajFile)

    Trajdata = Trajdata[Trajdata[:, 1] == frameNr]
    ax1.plot(Trajdata[:, 2], Trajdata[:, 3], "bo", markersize=20, markeredgewidth=2)

    ax1.set_xlim(geominX-0.2, geomaxX+0.2)
    ax1.set_ylim(geominY-0.2, geomaxY+0.2)
    plt.xlabel("x [m]")
    plt.ylabel("y [m]")
    plt.title("%s"%namefile)
    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", size="2.5%", pad=0.2)
    cb = fig.colorbar(sm, ax=ax1, cax=cax, format='%.1f')
    cb.set_label('Velocity [$m/s$]')
    figname = os.path.join(filepath, "v_"+namefile+".png")
    logging.info("SAVE: %s", figname)
    plt.savefig(figname)
    plt.close()
