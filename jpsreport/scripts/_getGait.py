from numpy import *
import matplotlib
import matplotlib.pyplot as plt
import argparse
from numpy import *
import glob,os
from mpl_toolkits.axes_grid1 import make_axes_locatable
import matplotlib.cm as cm
from matplotlib.patches import Arc
from scipy.signal import argrelextrema

def getParserArgs():
    parser = argparse.ArgumentParser(description='Combine French data to one file')
    parser.add_argument("-N", "--pedNum", type=int , default=16, help='The number of pedestrians in the run (default 16)')
    parser.add_argument("-r", "--runName", default="XP01", help='give the name of each run')
    args = parser.parse_args()
    return args

def estimated_autocorrelation(x):
    n = len(x)
    variance = x.var()
    x = x-x.mean()
    r = correlate(x, x, mode = 'full')[-n:]
    assert allclose(r, array([(x[:n-k]*x[-(n-k):]).sum() for k in range(n)]))
    result = r/(variance*(arange(n, 0, -1)))
    return result

def savitzky_golay(y, window_size, order, deriv=0, rate=1):
    r"""Smooth (and optionally differentiate) data with a Savitzky-Golay filter.
    The Savitzky-Golay filter removes high frequency noise from data.
    It has the advantage of preserving the original shape and
    features of the signal better than other types of filtering
    approaches, such as moving averages techniques.
    Parameters
    ----------
    y : array_like, shape (N,)
        the values of the time history of the signal.
    window_size : int
        the length of the window. Must be an odd integer number.
    order : int
        the order of the polynomial used in the filtering.
        Must be less then `window_size` - 1.
    deriv: int
        the order of the derivative to compute (default = 0 means only smoothing)
    Returns
    -------
    ys : ndarray, shape (N)
        the smoothed signal (or it's n-th derivative).
    Notes
    -----
    The Savitzky-Golay is a type of low-pass filter, particularly
    suited for smoothing noisy data. The main idea behind this
    approach is to make for each point a least-square fit with a
    polynomial of high order over a odd-sized window centered at
    the point.
    Examples
    --------
    t = np.linspace(-4, 4, 500)
    y = np.exp( -t**2 ) + np.random.normal(0, 0.05, t.shape)
    ysg = savitzky_golay(y, window_size=31, order=4)
    import matplotlib.pyplot as plt
    plt.plot(t, y, label='Noisy signal')
    plt.plot(t, np.exp(-t**2), 'k', lw=1.5, label='Original signal')
    plt.plot(t, ysg, 'r', label='Filtered signal')
    plt.legend()
    plt.show()
    References
    ----------
    .. [1] A. Savitzky, M. J. E. Golay, Smoothing and Differentiation of
       Data by Simplified Least Squares Procedures. Analytical
       Chemistry, 1964, 36 (8), pp 1627-1639.
    .. [2] Numerical Recipes 3rd Edition: The Art of Scientific Computing
       W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P. Flannery
       Cambridge University Press ISBN-13: 9780521880688
    """
    import numpy as np
    from math import factorial

    try:
        window_size = np.abs(np.int(window_size))
        order = np.abs(np.int(order))
    except (ValueError, msg):
        raise ValueError("window_size and order have to be of type int")
    if window_size % 2 != 1 or window_size < 1:
        raise TypeError("window_size size must be a positive odd number")
    if window_size < order + 2:
        raise TypeError("window_size is too small for the polynomials order")
    order_range = range(order+1)
    half_window = (window_size -1) // 2
    # precompute coefficients
    b = np.mat([[k**i for i in order_range] for k in range(-half_window, half_window+1)])
    m = np.linalg.pinv(b).A[deriv] * rate**deriv * factorial(deriv)
    # pad the signal at the extremes with
    # values taken from the signal itself
    firstvals = y[0] - np.abs( y[1:half_window+1][::-1] - y[0] )
    lastvals = y[-1] + np.abs(y[-half_window-1:-1][::-1] - y[-1])
    y = np.concatenate((firstvals, y, lastvals))
    return np.convolve( m[::-1], y, mode='valid')


if __name__ == '__main__':
    args = getParserArgs()
    Npeds=args.pedNum
    runName=args.runName
    print(runName)
    #ID=0
    file="./ring_adults_07_05/ring_adults_07_05_hdv_shifted.txt"    
    traj=loadtxt(file)
    Persons=unique(traj[:,0])
    freq_size=zeros((1,5))
    fps = 25
    for pers in Persons:
        data=traj[traj[:,0]==pers]
        v_x=copy(data[5:-5,2])
        v_y=copy(data[5:-5,3])
        a_x = copy(v_x[5:-5])
        a_y = copy(v_y[5:-5])
        T_Curvature = zeros((len(a_x),2))
        C = copy(v_y[5:-5])
        frame_step = 10 ## frame_step should be even
        window_size = 41
        order = 3
        figname=file+"_pers_%d"%(pers)+".png"
        step_frequency=zeros((1,5))
        #----------------------claculating velocity, accelaration and curvature--------------------------
        for i in range(len(v_x)):
            v_x[i]=fps*(data[i+frame_step,2]-data[i,2])/(frame_step*100.)
            v_y[i]=fps*(data[i+frame_step,3]-data[i,3])/(frame_step*100.)
        for j in range(len(a_x)):
            a_x[j]=fps*(v_x[j+frame_step]-v_x[j])/frame_step
            a_y[j]=fps*(v_y[j+frame_step]-v_y[j])/frame_step
            T_Curvature[j,0]=j
            T_Curvature[j,1]= (v_x[j+5]*a_y[j]-v_y[j+5]*a_x[j])/pow(sqrt(pow(v_x[j+5],2)+pow(v_y[j+5],2)),3)
            

        #-------------------------Smooth the time series of curvatures----------------------------
        smoothed_curv = savitzky_golay(T_Curvature[:,1], window_size, order)
        peak_id=argrelextrema(smoothed_curv, greater_equal, mode='wrap')[0]
        peak=smoothed_curv[peak_id]
        valley_id=argrelextrema(smoothed_curv, less_equal, mode='wrap')[0]
        valley=smoothed_curv[valley_id]
        fig = plt.figure(figsize=(13, 17), dpi=100)
        fig.subplots_adjust(hspace=0.4)
        ax1 = fig.add_subplot(211,aspect='equal')
        plt.rc("font", size=40)
        ax1.plot(data[:,2]/100.,data[:,3]/100., 'r-', lw=0.4)
        for i in peak_id:
            pmax=data[data[:,1]==(i+frame_step/2)]
            ax1.plot(pmax[:,2]/100.,pmax[:,3]/100., 'bo', lw=0.4)
            step_frequency=append(step_frequency,pmax,axis=0)
        for j in valley_id:
            pmin=data[data[:,1]==(j+frame_step/2)]
            ax1.plot(pmin[:,2]/100.,pmin[:,3]/100., 'go', lw=0.4)
            step_frequency=append(step_frequency,pmin,axis=0)
        ax1.set_ylim(-4,4)
        ax1.set_xlim(-6,6)
        fs=40
        plt.xlabel("$x\, [m]$", fontsize=fs)
        plt.ylabel("$y\, [m]$", fontsize=fs)
        ax0 = fig.add_subplot(212,aspect='auto')
        ax0.plot(T_Curvature[:,0]/fps,T_Curvature[:,1], 'ro', markeredgecolor='r',lw=0.4,markersize=3)
        ax0.plot(T_Curvature[:,0]/fps,smoothed_curv, 'g-', lw=0.8)
        ax0.plot(peak_id/fps,peak, 'bo', lw=0.8,markersize=8)
        ax0.plot(valley_id/fps,valley, 'bo', lw=0.8,markersize=8)
        plt.xlabel("$t\, [s]$", fontsize=fs)
        plt.ylabel("$Curvature$", fontsize=fs)
        plt.savefig(figname)
        plt.close()
        step_frequency=delete(step_frequency,0,axis=0)
        step_frequency=step_frequency[argsort(step_frequency[:,1])]
        fstep_frequency=file+"_gait_Pers_%d"%(pers)+".dat"
        freq_size=append(freq_size,diff(step_frequency,axis=0),axis=0)
        savetxt(fstep_frequency,step_frequency, fmt='%d\t%d\t%.3f\t%.3f\t%.3f',delimiter='\t',newline='\r\n')

        fig = plt.figure(figsize=(15, 12), dpi=100)
        ax2 = fig.add_subplot(211,aspect='auto')
        ax2.set_ylim(-2,2)
        plt.xlabel("$t\, [s]$", fontsize=fs)
        plt.ylabel("$Autocorrelation$", fontsize=fs)
        figname=file+"_gait_autocorrelation_Pers_%d"%(pers)+".png"
        plt.plot(T_Curvature[:,0]/fps,estimated_autocorrelation(T_Curvature[:,1]),'r-')
        fcurvature_name=file+"_curvature_%d"%(pers)+".dat"
        savetxt(fcurvature_name,T_Curvature, fmt='%.3f\t%.3f',delimiter='\t',newline='\r\n')
        plt.savefig(figname)
        plt.close()

    freq_size=delete(freq_size,0,axis=0)
    fig = plt.figure(figsize=(15, 12), dpi=100)
    ax3 = fig.add_subplot(111,aspect='auto')
    ax3.set_ylim(0,2)
    ax3.set_xlim(0,5)
    plt.xlabel("$t\, [s]$", fontsize=fs)
    plt.ylabel("$step size [m]$", fontsize=fs)
    figname=file+"_freq_stepsize.png"
    plt.plot(freq_size[:,1]/fps,sqrt(pow(freq_size[:,2],2)+pow(freq_size[:,3],2))/100.,'ro')
    freq_size[:,0]=freq_size[:,1]/fps
    freq_size[:,1]=sqrt(pow(freq_size[:,2],2)+pow(freq_size[:,3],2))/100.
    freq_size[:,2]=freq_size[:,1]/freq_size[:,0]
    freq_size=delete(freq_size,[3,4],axis=1)
    ffreq_size=file+"_freq_size.dat"
    savetxt(ffreq_size,freq_size, fmt='%.3f\t%.3f\t%.3f',delimiter='\t',newline='\r\n')
    plt.savefig(figname)
    plt.close()
