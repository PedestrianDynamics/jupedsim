#!/usr/bin/env python
from numpy import*
import matplotlib.cm as cm
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
import argparse
import sys,glob


def getParserArgs():
	parser = argparse.ArgumentParser(description='Plot Profiles of density, velocity and flow')
	parser.add_argument("-p", "--pathtraj", default="./", help='give the path of trajectory file')
	parser.add_argument("-n", "--nametraj", help='give the name of the trajectory file')
	parser.add_argument("-b", "--beginsteady", type=int, help='give the frame for the begining of steady state')
	parser.add_argument("-e", "--endsteady", type=int, help='give the frame for the ending of steady state')
	parser.add_argument("-x1", "--geominx", type=float, help='give the minmum x of the geometry')
	parser.add_argument("-x2", "--geomaxx", type=float, help='give the maxmum x of the geometry')
	parser.add_argument("-y1", "--geominy", type=float, help='give the minmum y of the geometry')
	parser.add_argument("-y2", "--geomaxy", type=float, help='give the maxmum y of the geometry')
	args = parser.parse_args()
	return args

if __name__ == '__main__':
    args = getParserArgs()
    pathtraj = args.pathtraj
    sys.path.append(pathtraj)
    nametraj = args.nametraj
    beginsteady = args.beginsteady
    endsteady = args.endsteady
    geominX = args.geominx
    geomaxX = args.geomaxx
    geominY = args.geominy
    geomaxY = args.geomaxy

    path_profile_file = "%s/Output/Fundamental_Diagram/Classical_Voronoi/field"%(pathtraj)
    f_Voronoi = glob.glob("%s/density/*%s*%d.dat"%(path_profile_file,nametraj,beginsteady))
    shape=shape(loadtxt(f_Voronoi[0]))
    density=zeros(shape)
    velocity=zeros(shape)
    #-------------------------------------density profile-----------------------------------------------------------------------
    fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
    ax1 = fig.add_subplot(111, aspect='1')
    plt.rc("font", size=40)
    for j in range(beginsteady,endsteady):
        density+=loadtxt("%s/density/Prf_d_%s_"%(path_profile_file,nametraj)+str(j)+".dat")
    density=density/(endsteady-beginsteady)
    im = plt.imshow(density, cmap=cm.jet, interpolation='nearest',origin='lower',vmin=0,vmax=amax(density), extent=[geominX,geomaxX,geominY,geomaxY])
    ax1.set_xlabel("x [m]")
    ax1.set_ylabel("y [m]")
    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", size="2.5%", pad=0.3)
    cb=plt.colorbar(im,cax=cax)
    cb.set_label('Density [$m^{-2}$]')
    plt.savefig("%s/profile_rho_%s.png"%(path_profile_file,nametraj))
    plt.close()

    #---------------------------------velocity profile-------------------------------------------------------------
    fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
    ax1 = fig.add_subplot(111, aspect='1')
    plt.rc("font", size=40)
    for j in range(beginsteady,endsteady):
        velocity+=loadtxt("%s/velocity/Prf_v_%s_"%(path_profile_file,nametraj)+str(j)+".dat")
    velocity=velocity/(endsteady-beginsteady)
    im = plt.imshow(velocity, cmap=cm.jet, interpolation='nearest',origin='lower',vmin=0,vmax=amax(velocity), extent=[geominX,geomaxX,geominY,geomaxY])
    ax1.set_xlabel("x [m]")
    ax1.set_ylabel("y [m]")
    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", size="2.5%", pad=0.3)
    cb=plt.colorbar(im,cax=cax)
    cb.set_label('Velocity [$m/s$]')
    plt.savefig("%s/profile_v_%s.png"%(path_profile_file,nametraj))
    plt.close()

#---------------------------------flow profile-------------------------------------------------------------
    fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
    ax1 = fig.add_subplot(111, aspect='1')
    plt.rc("font", size=40)
    flow=density*velocity
    im = plt.imshow(flow, cmap=cm.jet, interpolation='nearest',origin='lower',vmin=0,vmax=amax(flow), extent=[geominX,geomaxX,geominY,geomaxY])
    ax1.set_xlabel("x [m]")
    ax1.set_ylabel("y [m]")
    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", size="2.5%", pad=0.3)
    cb=plt.colorbar(im,cax=cax)
    cb.set_label('Specific flow [$1/m \cdot s$]')
    plt.savefig("%s/profile_flux_%s.png"%(path_profile_file,nametraj))
    plt.close()


