from numpy import *
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
from Polygon import *         
import matplotlib.cm as cm
import pylab
from mpl_toolkits.axes_grid1 import make_axes_locatable
import argparse
import sys


def getParserArgs():
	parser = argparse.ArgumentParser(description='Combine French data to one file')
	parser.add_argument("-f", "--filepath", default="./", help='give the path of source file')
	parser.add_argument("-n", "--namefile", help='give the name of the source file')
	parser.add_argument("-x1", "--geominx", type=float, help='give the minmum x of the geometry')
	parser.add_argument("-x2", "--geomaxx", type=float, help='give the maxmum x of the geometry')
	parser.add_argument("-y1", "--geominy", type=float, help='give the minmum y of the geometry')
	parser.add_argument("-y2", "--geomaxy", type=float, help='give the maxmum y of the geometry')
	args = parser.parse_args()
	return args


if __name__ == '__main__':
   rho_max = 8.0
   args = getParserArgs()
   filepath = args.filepath
   sys.path.append(filepath)
   namefile = args.namefile
   geominX = args.geominx
   geomaxX = args.geomaxx
   geominY = args.geominy
   geomaxY = args.geomaxy
   fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
   ax1 = fig.add_subplot(111,aspect='equal')
   plt.rc("font", size=30)
   plt.rc('pdf',fonttype = 42)
   ax1.set_yticks([int(1*j) for j in range(-2,5)])
   for label in ax1.get_xticklabels() + ax1.get_yticklabels():
      label.set_fontsize(30)
   for tick in ax1.get_xticklines() + ax1.get_yticklines():
      tick.set_markeredgewidth(2)
      tick.set_markersize(6)
   ax1.set_aspect("equal") 
   density=array([])
   polys = open("%s/polygon%s.dat"%(filepath,namefile)).readlines()
   for poly in polys:
      exec("p = %s"%poly)
      #p=tuple(tuple(i/100.0 for i in inner) for inner in p)
      xx=1.0/Polygon(p).area()
      if xx>rho_max:
         xx=rho_max
      density=append(density,xx)
   Maxd=density.max()
   Mind=density.min()
   erro=ones(shape(density))*Mind
   density=rho_max*(density-erro)/(Maxd-Mind)
   sm = cm.ScalarMappable(cmap=cm.jet)
   sm.set_array(density)
   sm.autoscale_None()
   sm.set_clim(vmin=0,vmax=5)
   for poly in polys:
      exec("p = %s"%poly)
      #p=tuple(tuple(i/100.0 for i in inner) for inner in p)
      xx=1.0/Polygon(p).area()
		#print xx
      if xx>rho_max:
         xx=rho_max
      ax1.add_patch(pgon(p,facecolor=sm.to_rgba(xx), edgecolor='white',linewidth=2))
   points = loadtxt("%s/points%s.dat"%(filepath,namefile))
   ax1.plot(points[:,0],points[:,1],"bo",markersize = 20,markeredgewidth=2)
   ax1.set_xlim(geominX,geomaxX)
   ax1.set_ylim(geominY,geomaxY)
   plt.xlabel("x [m]")
   plt.ylabel("y [m]")
   #print density
   plt.title("%s"%namefile)
   divider = make_axes_locatable(ax1)
   cax = divider.append_axes("right", size="2.5%", pad=0.2)
   cb=fig.colorbar(sm,ax=ax1,cax=cax,format='%.1f')
   cb.set_label('Density [$m^{-2}$]') 
   plt.savefig("%s/rho_%s.png"%(filepath,namefile))
   plt.close()


