from numpy import *
import matplotlib
import matplotlib.pyplot as plt
from Polygon import *         
import matplotlib.cm as cm
import pylab
import argparse
import sys, glob,os


def getParserArgs():
	parser = argparse.ArgumentParser(description='Plot N-t graph measured from method A')
	parser.add_argument("-p", "--pathfile", default="./", help='give the path of source file')
	parser.add_argument("-n", "--nametraj", help='give the name of the trajectory')
	parser.add_argument("-f", "--fps", default="16", type=int, help='give the frame rate of data')
	args = parser.parse_args()
	return args

def plotRhoT(pathfile,figname,title,data_Classic=None, data_Voronoi=None):
        fig = plt.figure(figsize=(16, 16), dpi=100)
        ax1 = fig.add_subplot(111,aspect='auto')
        plt.rc("font", size=30)
        plt.rc('pdf',fonttype = 42)
        if data_Classic is not None:
                plt.plot(data_Classic[:,0]/fps,data_Classic[:,1], 'b--', label="Classic method")
        if data_Voronoi is not None:
                plt.plot(data_Voronoi[:,0]/fps,data_Voronoi[:,1], 'r-', lw=3, label="Voronoi method")
        plt.xlabel("t [$s$]")
        plt.ylabel("density [$m^{-2}$]")
        plt.gca().set_xlim(left=0)
        #plt.gca().set_ylim(bottom=0)
        plt.ylim(0,8)
        plt.title("%s"%title)
        plt.legend()
        plt.savefig("%s/%s.png"%(pathfile,figname))
        plt.close()

def plotVT(pathfile,figname,title,data_Classic=None, data_Voronoi=None):
        fig = plt.figure(figsize=(16, 16), dpi=100)
        ax1 = fig.add_subplot(111,aspect='auto')
        plt.rc("font", size=30)
        plt.rc('pdf',fonttype = 42)
        if data_Classic is not None:
                plt.plot(data_Classic[:,0]/fps,data_Classic[:,2], 'b--', label="Classic method")
        if data_Voronoi is not None:
                plt.plot(data_Voronoi[:,0]/fps,data_Voronoi[:,2], 'r-', lw=3, label="Voronoi method")
        plt.xlabel("t [$s$]")
        plt.ylabel("velocity [$m^{-2}$]")
        plt.gca().set_xlim(left=0)
        #plt.gca().set_ylim(bottom=0)
        plt.ylim(0,2)
        plt.title("%s"%title)
        plt.legend()
        plt.savefig("%s/%s.png"%(pathfile,figname))
        plt.close()

if __name__ == '__main__':
   args = getParserArgs()
   pathfile = args.pathfile
   sys.path.append(pathfile)
   nametraj = args.nametraj
   print("INFO:\tPlotting time series of density and velocity from trajectory <%s>"%(nametraj)) 
   fps = args.fps
   f_Voronoi = glob.glob("%s/*_Voronoi*%s*.dat"%(pathfile,nametraj))
   f_Classic = glob.glob("%s/*_Classic*%s*.dat"%(pathfile,nametraj))
   if f_Classic:
           for fC in f_Classic:
                   ID = os.path.basename(fC).split("_id_")[1].split(".")[0]
                   figname_rho="rho_t_%s_id_%s"%(nametraj,ID)
                   figname_v="v_t_%s_id_%s"%(nametraj,ID)
                   title = "%s_id_%s"%(nametraj,ID)
                   data_Classic = loadtxt(fC)
                   plotRhoT(pathfile,figname_rho,title,data_Classic)
                   plotVT(pathfile,figname_v,title,data_Classic)
           
   if f_Voronoi:
           for fV in f_Voronoi:
                   ID = os.path.basename(fV).split("_id_")[1].split(".")[0]
                   figname_rho="rho_t_%s_id_%s"%(nametraj,ID)
                   figname_v="v_t_%s_id_%s"%(nametraj,ID)
                   title = "%s_id_%s"%(nametraj,ID)
                   data_Voronoi = loadtxt(fV)
                   fC = "%s/rho_v_Classic_%s_id_%s.dat"%(pathfile,nametraj,ID)
                   if (os.path.isfile(fC)):
                           data_Classic = loadtxt(fC)
                           plotRhoT(pathfile,figname_rho,title,data_Classic,data_Voronoi)
                           plotVT(pathfile,figname_v,title,data_Classic,data_Voronoi)
                   else:
                           plotRhoT(pathfile,figname_rho,title,data_Voronoi)
                           plotVT(pathfile,figname_v,title,data_Voronoi)
