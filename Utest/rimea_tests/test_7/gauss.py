import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import numpy as np

weidman_max_vel = np.array([1.61, 1.54, 1.41, 0.76])
weidman_min_vel = np.array([0.58, 1.41, 0.68, 0.46])
means = 0.5*(weidman_max_vel+weidman_min_vel)   # [1.095, 1.475, 1.045, 0.61]
sigmas = [0.2, 0.02, 0.13, 0.05] # Parameters mus and sigmas are used in the master-inifile

j = 1
for (mu, sigma, mi, ma) in zip(means, sigmas, weidman_min_vel, weidman_max_vel):
    plt.subplot("41%d"%j)
    x = np.linspace(mi, ma, 100)
    plt.plot(x, mlab.normpdf(x, mu, sigma), lw=2)
    plt.plot([mi, mi], [0, 4.5], "r", lw=2)
    plt.plot([ma, ma], [0, 4.5], "r", lw=2)
    j += 1

plt.tight_layout()
plt.show()
