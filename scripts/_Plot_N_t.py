from numpy import *
import matplotlib
import matplotlib.pyplot as plt
from Polygon import *         
import matplotlib.cm as cm
import pylab
import argparse
import sys


def getParserArgs():
	parser = argparse.ArgumentParser(description='Plot N-t graph measured from method A')
	parser.add_argument("-p", "--pathfile", default="./", help='give the path of source file')
	parser.add_argument("-n", "--namefile", help='give the name of the source file')
	parser.add_argument("-f", "--fps", default="16", type=float, help='give the frame rate of data')
	args = parser.parse_args()
	return args


if __name__ == '__main__':
   args = getParserArgs()
   pathfile = args.pathfile
   sys.path.append(pathfile)
   namefile = args.namefile
   fps = args.fps
   figname=namefile.split(".dat")[0]
   title = figname.split("traj_")[1]
   fig = plt.figure(figsize=(16, 16), dpi=100)
   ax1 = fig.add_subplot(111,aspect='auto')
   plt.rc("font", size=30)
   plt.rc('pdf',fonttype = 42)
   data_NT = loadtxt("%s/%s"%(pathfile,namefile))
   plt.plot(data_NT[:,0]/fps,data_NT[:,1], 'r-')
   plt.xlabel("t [s]")
   plt.ylabel("N [-]")
   plt.title("%s"%title)
   plt.savefig("%s/%s.png"%(pathfile,figname))
   plt.close()



