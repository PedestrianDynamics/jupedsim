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
	parser.add_argument("-n", "--namefile", help='give the name of rho_v file',nargs='+')
	parser.add_argument("-s", "--steadyfile", help='give the name of steady state file',nargs='+')
	parser.add_argument("-f", "--figname", default="FD.png", help='give the name of figure for saving')
	args = parser.parse_args()
	return args

def plotRhoJs(pathfile,figname,namefile,steadyfile):
        fig = plt.figure(figsize=(16, 16), dpi=100)
        ax1 = fig.add_subplot(111,aspect='auto')
        plt.rc("font", size=30)
        plt.rc('pdf',fonttype = 42)
        figname="rho_Js_"+figname
        for f_rho_v, f_steady in zip(namefile,steadyfile):
                d_rho_v=loadtxt(f_rho_v,ndmin=2)
                d_steady= loadtxt(f_steady,ndmin=2)
                steady_rho_v=zeros(len(d_rho_v[0]))
                for i in range(len(d_steady)):
                        rho_v=d_rho_v[d_rho_v[:,0]>d_steady[i,0]]
                        rho_v=rho_v[rho_v[:,0]<d_steady[i,1]]
                        steady_rho_v=vstack((steady_rho_v,rho_v))
                steady_rho_v = delete(steady_rho_v, (0), axis=0)
                plt.plot(steady_rho_v[:,1],steady_rho_v[:,1]*steady_rho_v[:,2], 'o', lw=3, label=f_rho_v)
        plt.xlabel("density [$m^{-2}$]")
        plt.ylabel("specific flow [$(ms)^{-1}$]")
        plt.gca().set_xlim(left=0)
        plt.ylim(0,3)
        plt.legend()
        plt.savefig("%s/%s"%(pathfile,figname))
        plt.close()

def plotRhoV(pathfile,figname,namefile,steadyfile):
        fig = plt.figure(figsize=(16, 16), dpi=100)
        ax1 = fig.add_subplot(111,aspect='auto')
        plt.rc("font", size=30)
        plt.rc('pdf',fonttype = 42)
        figname="rho_v_"+figname
        for f_rho_v, f_steady in zip(namefile,steadyfile):
                d_rho_v=loadtxt(f_rho_v,ndmin=2)
                d_steady= loadtxt(f_steady,ndmin=2)
                steady_rho_v=zeros(len(d_rho_v[0]))
                for i in range(len(d_steady)):
                        rho_v=d_rho_v[d_rho_v[:,0]>d_steady[i,0]]
                        rho_v=rho_v[rho_v[:,0]<d_steady[i,1]]
                        steady_rho_v=vstack((steady_rho_v,rho_v))
                steady_rho_v = delete(steady_rho_v, (0), axis=0)
                plt.plot(steady_rho_v[:,1],steady_rho_v[:,2], 'o', lw=3, label=f_rho_v)
        plt.xlabel("density [$m^{-2}$]")
        plt.ylabel("velocity [$m/s$]")
        plt.gca().set_xlim(left=0)
        plt.ylim(0,2)
        plt.legend()
        plt.savefig("%s/%s"%(pathfile,figname))
        plt.close()

if __name__ == '__main__':
   args = getParserArgs()
   pathfile = args.pathfile
   sys.path.append(pathfile)
   namefile = args.namefile
   print(args.namefile)
   steadyfile= args.steadyfile
   figname = args.figname
   if(len(namefile)!= len(steadyfile)):
           print("Warning:\tThe number of rho_v files is not the same as the files of steady state!")
           sys.exit()
   else:
           print("INFO:\tPlotting Fundamental diagram of giving data")
           plotRhoV(pathfile,figname,namefile,steadyfile)
           plotRhoJs(pathfile,figname,namefile,steadyfile)


