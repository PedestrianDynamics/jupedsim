from numpy import *
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
#from Polygon import *         

for i in range(1,2):
   fig = plt.figure()
   ax1 = fig.add_subplot(111,aspect='equal')
   #polys = open("VoronoiCell\\polygonko-240-050-240_"+str(i)+".dat").readlines()
   polys = open("test_"+str(i)+".dat").readlines()
   for poly in polys:
       exec("p = %s"%poly)
       ax1.add_patch(pgon(p,facecolor='maroon', edgecolor='orange',linewidth=1,alpha=1))
   ax1.set_xlim(-1500,1500)
   ax1.set_ylim(-6000,1000)
   plt.savefig(str(i)+".png")
   plt.close(fig)

