#!/usr/bin/env python
from numpy import*
import matplotlib.cm as cm
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable




shape=(64,66)
density=zeros(shape)
fig = plt.figure(figsize=(15, 12), dpi=100)
ax1 = fig.add_subplot(111, aspect='1')
plt.rc("font", size=40)

for j in range(320,800):
    density+=loadtxt("density/Prf_d_eo-240-240-240.xml_"+str(j)+".dat")
density=density/480.0

print(density.shape)
shape1=(66,68)
density1=zeros(shape1)
for i in range(0,64):
	for k in range(0,66):
		density1[i+2][k+2]=density[i][k]

im = plt.imshow(density1, cmap=cm.jet, interpolation='nearest',origin='lower',vmin=0,vmax=5.0, extent=[-2.4,4.2,-2.4,4])
#plt.title('Density profile')
#plt.colorbar(im,shrink=0.65)
ax1.set_xlabel("x [m]")
ax1.set_ylabel("y [m]")
divider = make_axes_locatable(ax1)
cax = divider.append_axes("right", size="2.5%", pad=0.3)
cb=plt.colorbar(im,cax=cax)
cb.set_label('Density [$m^{-2}$]')
plt.savefig("density_average_eo-240-240-240.pdf")
plt.gcf().clear()
savetxt("density"+str(320)+"_"+str(800)+"_average.dat",density,fmt='%.3f',delimiter="   ")
#cb.set_label('density probability') 
#plt.show()



