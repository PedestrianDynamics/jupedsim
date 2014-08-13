from numpy import *
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
from Polygon import *         
import matplotlib.cm as cm
import pylab
from mpl_toolkits.axes_grid1 import make_axes_locatable


rho_max = 5.0
t_start = 700
t_end = 701
for i in range(t_start,t_end):
   fig = plt.figure(figsize=(16, 12), dpi=100)
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
   polys = open("./polygonko-240-240-240_"+str(i)+".dat").readlines()
   for poly in polys:
      exec("p = %s"%poly)
      p=tuple(tuple(i/100.0 for i in inner) for inner in p)
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
      p=tuple(tuple(i/100.0 for i in inner) for inner in p)
      xx=1.0/Polygon(p).area()
		#print xx
      if xx>rho_max:
         xx=rho_max
      ax1.add_patch(pgon(p,facecolor=sm.to_rgba(xx), edgecolor='white',linewidth=2))
   points = loadtxt("./pointsko-240-240-240_"+str(i)+"_left.dat")
   ax1.plot(points[:,0]/100.,points[:,1]/100.,"bo",markersize = 20,markeredgewidth=2)
   points = loadtxt("./pointsko-240-240-240_"+str(i)+"_right.dat")
   ax1.plot(points[:,0]/100.,points[:,1]/100.,"ro",markersize = 20,markeredgewidth=2)
   ax1.set_xlim(-4.50,4.00)
   ax1.set_ylim(-2.60,4.00)
   plt.xlabel("x [m]")
   plt.ylabel("y [m]")
   #print density
   divider = make_axes_locatable(ax1)
   cax = divider.append_axes("right", size="2.5%", pad=0.2)
   cb=fig.colorbar(sm,ax=ax1,cax=cax,format='%.1f')
   cb.set_label('Density [$m^{-2}$]') 
   #plt.title('Frame '+ str(i))
   plt.savefig(str(i)+".pdf")
   plt.close()
#      plt.show()


