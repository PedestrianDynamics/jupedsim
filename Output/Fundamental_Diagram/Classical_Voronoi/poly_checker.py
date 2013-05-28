from numpy import *
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
from Polygon import *         
import matplotlib.cm as cm
import pylab
from mpl_toolkits.axes_grid1 import make_axes_locatable

for i in range(59,300):
   fig = plt.figure(figsize=(16, 12), dpi=100)
   ax1 = fig.add_subplot(111,aspect='equal')
   ax1.set_yticks([int(100*j) for j in range(0,4)])
   for label in ax1.get_xticklabels() + ax1.get_yticklabels():
      label.set_fontsize(15)
   for tick in ax1.get_xticklines() + ax1.get_yticklines():
      tick.set_markeredgewidth(2)
      tick.set_markersize(6)
   ax1.set_aspect("equal") 
   density=array([])
   polys = open("polygonb090_"+str(i)+".dat").readlines()
   for poly in polys:
      exec("p = %s"%poly)
      xx=10000/Polygon(p).area()
      if xx>5:
         xx=5
      density=append(density,xx)
   Maxd=density.max()
   Mind=density.min()
   erro=ones(shape(density))*Mind
   if(Maxd-Mind!=0):
      density=5.0*(density-erro)/(Maxd-Mind)
   sm = cm.ScalarMappable(cmap=cm.jet)
   sm.set_array(density)
   sm.autoscale_None()
   sm.set_clim(vmin=2,vmax=3.5)
   for poly in polys:
      exec("p = %s"%poly)
      xx=10000/Polygon(p).area()
		#print xx
      if xx>5:
         xx=5
      ax1.add_patch(pgon(p,facecolor=sm.to_rgba(xx), edgecolor='white',linewidth=2))
   points = loadtxt("pointsb090_"+str(i)+".dat")
   ax1.plot(points[:,0],points[:,1],"ko",markeredgecolor="w",markersize = 10,markeredgewidth=2)
   ax1.set_xlim(-250,250)
   ax1.set_ylim(-300,500)
   #print density
   divider = make_axes_locatable(ax1)
   cax = divider.append_axes("right", size="2.5%", pad=0.2)
   cb=fig.colorbar(sm,ax=ax1,cax=cax)
   cb.set_label('Density [$m^{-2}$]') 
   plt.title('Frame '+ str(i))
   plt.savefig(str(i)+".png")
   plt.gcf().clear()
#      plt.show()


