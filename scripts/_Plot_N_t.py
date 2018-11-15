from numpy import *
import matplotlib
import matplotlib.pyplot as plt
from Polygon import *
import matplotlib.cm as cm
import pylab
import argparse
import sys
import os

def getParserArgs():
        parser = argparse.ArgumentParser(description='Plot N-t graph measured from method A')
        parser.add_argument("-p", "--pathfile", default="./", help='give the path of source file')
        parser.add_argument("-n", "--namefile", help='give the name of the source file')
        #parser.add_argument("-f", "--fps", default="16", type=float, help='give the frame rate of data')
        args = parser.parse_args()
        return args


if __name__ == '__main__':
   args = getParserArgs()
   pathfile = args.pathfile
   sys.path.append(pathfile)
   namefile = args.namefile
   #fps = args.fps
   figname=namefile.split(".dat")[0]
   title = figname.split("Flow_NT_")[1]
   fig = plt.figure(figsize=(16, 16), dpi=100)
   ax1 = fig.add_subplot(111,aspect='auto')
   plt.rc("font", size=30)
   plt.rc('pdf',fonttype = 42)
   data_file = os.path.join(pathfile, namefile)
   if not os.path.exists(data_file):
       sys.exit("ERROR %s: file does not exist <%s>" %(sys.argv[0], data_file))

   data_NT = loadtxt(data_file)
   #"%s/%s"%(pathfile,namefile))
   #plt.plot(data_NT[:,0]/fps,data_NT[:,1], 'r-')
   plt.plot(data_NT[:,0],data_NT[:,1], 'r-')
   plt.xlabel("t [s]")
   plt.ylabel("N [-]")
   plt.title("%s"%title)
   figname = os.path.join(pathfile, figname)
   plt.savefig("%s.png"%figname)
   plt.close()
