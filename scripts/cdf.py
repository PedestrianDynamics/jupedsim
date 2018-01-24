import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import sys
from sys import exit

fps = 8
x0 = 61
name = "times.txt"



def rolling_flow(fps, N, data, x0, name, windows):
    """
    measure the flow at a vertical line given by <x0>
    trajectories are given by <times> in the following format: id    frame    x    y
    input:
    - fps: frame per second
    - N: number of peds
    - times: trajectories
    - x0: x-coordinate of the vertical measurement line
    output:
    - flow
    """
    if not isinstance(data, np.ndarray):       
        exit(-1)
    peds = np.unique(data[:, 0]).astype(int)
    frames = np.unique(data[:, 1]).astype(int)
    maxframe = np.max(frames)
    times = []
    fr = 0
    delta_fr = 100
    while fr + delta_fr <= maxframe:
        data_fr = data[np.logical_and(data[:, 1] >=fr,  data[:, 1] <= fr + delta_fr)]
        peds = np.unique(data_fr[:, 0]).astype(int)
        fr += delta_fr
        for ped in peds:
            d = data_fr[data_fr[:, 0] == ped]
            passed = d[d[:, 2] >= x0]
            if passed.size == 0:  # pedestrian did not pass the line
                continue
            first = min(passed[:, 1])
            #print "ped= ", ped, "first=",first
            times.append(first)



            
    times = np.sort(times)
    serie = pd.Series(times)
    minp = 100; #windows = 200 #int(len(times)/10);
    minp = min(minp, windows)
    flow = fps*(windows-1)/(serie.rolling(windows, min_periods=minp).max()  - serie.rolling(windows, min_periods=minp).min() )
    flow = flow[~np.isnan(flow)] # remove NaN
    wmean = flow.rolling(windows, min_periods=minp).mean()
 
    np.savetxt(name, np.array(times))
    
    print("min(times)=%f max(times)=%f"%(min(times), max(times)))
    return fps * float(N-1) / (max(times) - min(times)), np.mean(wmean), wmean, times


def model(x, a, b, c):
  return a*np.exp(-b*x**c)


# P(X<=x, Y<=y)
def CDF2D(x, y, times1, times2):
    mat = np.empty(shape=[len(times1), len(times2)])
    cond = np.logical_and(times1 <= x, times2 <= y)
    return len(mat[cond])/len(times1)/len(times2)

# X= np.dstack(np.meshgrid(times1, times2)).reshape((len(times1)*len(times2), 2))
# x= X[:, 0]
# y = X[:, 1]



# empirical CDF P(x<=X)
def CDF(x, times):    
    return float(len(times[times <= x]))/len(times)


#--------------------------------------------------------
if __name__ == "__main__":
    print("loading %s" %sys.argv[1])
    #data = np.loadtxt(sys.argv[1])
    data = pd.read_csv(sys.argv[1], sep="\s+", header=0, skiprows=8)
    data = np.array(data)
    N = len(np.unique(data[:, 0]))
    print("finished loading")
    print("calculating rolling_flow")
    windows = 200
    J, meanJ, rolling_J, times = rolling_flow(fps, N, data, x0, name, windows)
    print("finished calculation")

    diff = np.diff(times)
    diff = np.sort(diff)


    vF = np.vectorize(CDF, excluded=['times'])

    plt.subplot(211)
    plt.plot(times, 1-vF(x=times, times=times))
    plt.ylabel(r"$P(T>t)$", size=16)

    plt.xlabel(r"$t / s$", size=16)
    plt.loglog()

    plt.subplot(212)
    y_diff = 1-vF(x=diff, times=diff)

    p0 = np.array([1, 1, 1])
    # coeffs, matcov = curve_fit(model, diff, y_diff)
    # y_fit = model(diff, coeffs[0], coeffs[1], coeffs[2])
    plt.plot(diff, y_diff, "rx", ms=5)
    plt.plot(diff, y_diff, "r--", alpha=0.4)
    plt.ylabel(r"$P(T>\delta)$", size=16)
    plt.xlabel(r"$t / s$", size=16)
    plt.loglog()
    
    plt.savefig("fig.png", dpi=600)
    
