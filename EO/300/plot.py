#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt
import glob

files = glob.glob("*.txt")

for file in files:
    print "file ",file
    d = np.loadtxt(file)
    x = d[:,2]
    y = d[:,3]
    plt.plot(x,y,'bo')

plt.xlabel("x[cm]")
plt.ylabel("y[cm]")
plt.show()
