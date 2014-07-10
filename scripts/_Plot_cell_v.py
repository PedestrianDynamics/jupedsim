from numpy import *
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
from Polygon import *         
import matplotlib.cm as cm
import pylab
from mpl_toolkits.axes_grid1 import make_axes_locatable
import matplotlib


t_start = 700
t_end = 701
for i in range(t_start,t_end):
   fig = plt.figure(figsize=(16, 12), dpi=300)
   ax1 = fig.add_subplot(111,aspect='equal')
   plt.rc("font", size=30)
   ax1.set_yticks([int(100*j) for j in range(-2,5)])
   for label in ax1.get_xticklabels() + ax1.get_yticklabels():
      label.set_fontsize(30)
   for tick in ax1.get_xticklines() + ax1.get_yticklines():
      tick.set_markeredgewidth(2)
      tick.set_markersize(6)
   ax1.set_aspect("equal") 
   velocity=array([])
   polysfile = open("./polygonko-240-240-240_"+str(i)+".dat")
   polys=polysfile.readlines()
   velocity=loadtxt("./speedko-240-240-240_"+str(i)+".dat")
   sm = cm.ScalarMappable(cmap=cm.jet)
   sm.set_array(velocity)
   sm.autoscale_None()
   sm.set_clim(vmin=0,vmax=1.5)
   index=0
   for poly in polys:
      exec("p = %s"%poly)
      xx = velocity[index]
      index += 1
      ax1.add_patch(pgon(p,facecolor=sm.to_rgba(xx), edgecolor='white',linewidth=2))
   points = loadtxt("./pointsko-240-240-240_"+str(i)+"_left.dat")
   ax1.plot(points[:,0],points[:,1],"bo",markersize = 20,markeredgewidth=2)
   points = loadtxt("./pointsko-240-240-240_"+str(i)+"_right.dat")
   ax1.plot(points[:,0],points[:,1],"ro",markersize = 20,markeredgewidth=2)
   ax1.set_xlim(-450,400)
   ax1.set_ylim(-260,400)
   plt.xlabel("x [cm]")
   plt.ylabel("y [cm]")
   #print density
   divider = make_axes_locatable(ax1)
   cax = divider.append_axes("right", size="2.5%", pad=0.2)
   cb=fig.colorbar(sm,ax=ax1,cax=cax)
   cb.set_label('Velocity [$m/s$]') 
   #plt.title('Frame '+ str(i))
   #plt.xlabel('X [cm]')
   #plt.ylabel('Y [cm]')
   plt.savefig(str(i)+"_v.png")
   plt.gcf().clear()
   polysfile.close()
#      plt.show()




