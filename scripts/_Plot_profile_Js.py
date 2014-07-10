from numpy import*
import matplotlib.cm as cm
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable



density=loadtxt("./density320_800_average.dat")
velocity=loadtxt("./velocity320_800_average.dat")
flow=density*velocity

shape1=(66,68)
flow1=zeros(shape1)
fig = plt.figure(figsize=(15, 12), dpi=100)
ax1 = fig.add_subplot(111,aspect='1')
plt.rc("font", size=40)
for i in range(0,64):
	for k in range(0,66):
		flow1[i+2][k+2]=flow[i][k]
im = plt.imshow(flow1, cmap=cm.jet, interpolation='nearest',origin='lower',vmin=0,vmax=2.5, extent=[-2.4,4.2,-2.4,4])
#plt.title('specific flow profile [$1/m \cdot s$]')
#plt.title('$b_{ent} = 2.4 m$')
plt.xlabel('x [m]')
plt.ylabel('y [m]')
divider = make_axes_locatable(ax1)
cax = divider.append_axes("right", size="2.5%", pad=0.2)
cb=fig.colorbar(im,ax=ax1,cax=cax,format='%.1f')
cb.set_label('Specific flow [$1/m \cdot s$]')
#plt.savefig("flux_average_ko-240-240-240_2014.png")
plt.savefig("flux_average_eo-240-240-240_2014.pdf")
savetxt("flow.dat",flow,fmt='%.3f',delimiter="   ")
