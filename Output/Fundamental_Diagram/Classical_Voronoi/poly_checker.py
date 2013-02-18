from numpy import *
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
#from Polygon import *         

for i in range(89,170):
   fig = plt.figure()
   ax1 = fig.add_subplot(111,aspect='equal')
   polys = open("VoronoiCell\\polygonko-240-050-240_"+str(i)+".dat").readlines()
   for poly in polys:
       exec("p = %s"%poly)
       ax1.add_patch(pgon(p,facecolor='maroon', edgecolor='orange',linewidth=3,alpha=1))
   ax1.set_xlim(-500,400)
   ax1.set_ylim(-240,400)
   plt.savefig(str(i)+".png")
   plt.close(fig)

