"""
plot fundamental diagram produced by runtest
just to check the results without reruning the test 
"""
import glob
import matplotlib.pyplot as plt
import numpy as np

simfiles = glob.glob("Output/Fundamental_Diagram/Individual_FD/*.dat")
expfiles = glob.glob("data/*.txt")

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)

once = 1
for s in simfiles:
    dsim = np.loadtxt(s)
    if once:
        plt.plot(dsim[:, 2], dsim[:, 3], ".b", label="simulation")
        once = 0
    else:
        plt.plot(dsim[:, 2], dsim[:, 3], ".b")

once = 1        
for e in expfiles:
    dexp = np.loadtxt(e)
    if once:
        plt.plot(dexp[:, 0], dexp[:, 1], "xr", label="experiment")
        once = 0
    else:
        plt.plot(dexp[:, 0], dexp[:, 1], "xr")

plt.ylabel(r"$v\; [m/s]$", size=20)
plt.xlabel(r"$\rho \; [1/m^2]$", size=20)
plt.legend(loc="best", numpoints=1)
fig.set_tight_layout(True)
plt.savefig("fd2d_tmp.png", dpi=300)
