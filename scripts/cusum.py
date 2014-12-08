import numpy as np
import scipy.stats as stats
import numpy.random
import matplotlib.pylab as plt

class cusum:
    ''' Change - Point - Detection
    
        detects change points according to "A cusum change-point detection algorithm for non-stationary sequences
        with applications to data network surveillance", DeOca et al., Journal of Systems and Software 83 (2010), 1288-1297
        
        
    '''
    
    def __init__(self, time_interval_in_seconds, window_len_in_seconds, historical_gap_in_seconds, alpha = 0.95, gamma = 0.1, nu = 8, M = 100):
        ''' constructor
            
        @param     time_interval_in_seconds     time interval between two observations
        @param     window_len_in_seconds        length of historical data
        @param     historical_gap_in_seconds    gap between current observation and historical data
        @param     alpha                        parameter to control the degree a deviation from the reference distribution is considered as critical
        @param     gamma                        controls the probability of false alarms
        @param     nu                           number of observations to be used in determining end time of an alarm and alarm level 
        @param     M                            number of sampled sequences used for determining appropriate threshold
            
        '''
        self.window_len      = window_len_in_seconds / time_interval_in_seconds
        self.historical_gap  = historical_gap_in_seconds / time_interval_in_seconds
        self.alpha           = alpha
        self.gamma           = gamma
        self.nu              = nu
        self.M               = M
        
        self.data            = []
        self.s_plus          = 0
        self.s_minus         = 0
        self.historical_data = []
        self.percentil       = alpha * 100
        self.s1              = []
        self.s2              = []
        self.h_values        = []
        self.alarms          = []
        
    def sample_from_data(self, data, length, minrange = 0, maxrange = 100, bins = 100):
        '''Inverse transform sampling
        
        samples data drawn from distribution of given observations
        
        @param    data        list of observations
        @param    length      number of samples
        
        @return               samples that are drawn according to distribution of given observations
        '''
        
        #first compute pdf
        pdf, edges = np.histogram(data, range=(minrange, maxrange), bins=bins, normed=True)
        pdf = pdf / np.sum(pdf)
        #now compute cdf
        cdf = np.cumsum(pdf)

        sample = []
        while len(sample) < length:
            #draw from uniform distribution
            u = np.random.rand()
            
            #compute value x such that cdf(x) = u
            x_idx = (cdf > u).nonzero()[0][0]
            x_chosen = edges [x_idx]
            sample.append(x_chosen)
            
        return sample
    
    def max_s_tracking(self, data, percentil = 0.9):
        '''compute cusum statistics for data sampled from distribution of historical data
        
        generates cusum statistics for sampled data in order to determine an appropriate threshold. 
        Suppose that the sequence of observations is drawn from the reference distribution, that is, no anomaly occurs. 
        Using a bootstrap resampling method, we can select an appropriate threshold so that the probability of false
        alarms is equal to percentil
        
        
        @param    data        data that is sampled according to distribution of historical data
        @param    percentil   probability of false alarm
        
        @return               tracking statistics. maximum of s+ and s-
        '''
        s_plus  = 0
        s_minus = 0
        percentil = percentil * 100
        s = []
        
        
        for idx in range(len(data)): 
            
            percentil_upper = stats.scoreatpercentile(data, percentil)
            percentil_lower = stats.scoreatpercentile(data, 1 - percentil)
                
            s_plus  = np.fmax(0, s_plus + data[idx] - percentil_upper)
            s_minus = np.fmax(0, s_plus + percentil_lower - data[idx])
             
            s.append(np.fmax(s_plus, s_minus))
            
            
        return np.max(s)
    
    def has_positive_slope(self, data):
        ''' tests if slope is positive
        
        fits a linear regression model to observations and tests if the slope of that fitted line is positive
        
        @param     data    list of observations
        @return            true if the slope of the line fitted to the data is positive
        '''
        
        
        (ar, br) = np.polyfit(np.arange(len(data)), data, 1)
        angle = np.rad2deg(np.arctan(ar) / 1.0)
        return ar > 0, angle / 90.0
    
    def change_point(self, x):
        ''' detects change point
        
        based on historical data, a change point is detected, if its deviation from historical data is high
        
        @param     x    the new incoming observation
        
        @return         alarm is 0 if no alarm is triggered. alarm <0 if an alarm is
                        triggered due to decreasing signal and it is >0 if an alarm is triggered due to an increasing signal. 
                        The absolute value corresponds to the alarm level where 1 (or -1) is the highest alarm level 
        
        
        '''
        
        # collect historical data
        self.historical_data = []
        idx = int(len(self.data) - self.historical_gap)
        while idx >= 0 and len(self.historical_data) < self.window_len:
            self.historical_data.append(self.data[idx])
            idx -= 1
        
        if len(self.historical_data) > 1:
            #determine thresholds point
            max_s = []
            iteration = 0
            while iteration < self.M:
                sample = self.sample_from_data(self.historical_data, len(self.historical_data), minrange = 0, maxrange = np.max(self.historical_data), bins = 10000)
                m = self.max_s_tracking(sample)
                max_s.append(m)
                iteration += 1
                
                
            h = stats.scoreatpercentile(max_s, (1-self.gamma) * 100)

            
            
        
            #use historical data to determine upper and lower percentil
            percentil_upper = stats.scoreatpercentile(self.historical_data, self.percentil)
            percentil_lower = stats.scoreatpercentile(self.historical_data, 1 - self.percentil)
            
            
            s_plus  = np.fmax(0, self.s_plus  + x               - percentil_upper)
            s_minus = np.fmax(0, self.s_minus + percentil_lower - x)

            
            self.s1.append(s_plus)
            self.s2.append(s_minus)
            self.h_values.append(h)
            
            if bool(s_plus > h) + bool(s_minus > h) == 1: # xor
                if len(self.data) > 0 and self.alarms[-1] != 0:
                    start = np.fmax(0, len(self.data) - self.nu + 1)
                    if s_plus > h:
                        slope_test_data = self.s1[start : len(self.data) + 1]
                    else:
                        slope_test_data = self.s2[start : len(self.data) + 1]
                   
                    
                    slope, alarmlevel = self.has_positive_slope(slope_test_data)
                    if slope:
                        self.alarms.append( (bool(s_plus > h) - bool(s_minus > h)) * abs(alarmlevel)) # >0 if increasing signal, <0 if decreasing signal
                    else: 
                        # end of alarm. in the paper, the timepoint where si is at its maximum is declared to be the end point. 
                        # but this time is actually in the past, so we take this point as the end of the alarm
                        if bool(s_plus > h):
                            s_plus  = 0
                        else:
                            s_minus = 0
                        self.alarms.append( 0 )
                    
                else:
                    self.alarms.append( (bool(s_plus > h) - bool(s_minus > h)) * 0.1 )# >0 if increasing signal, <0 if decreasing signal
                    
            else:
                self.alarms.append( 0 )
                
        else:
            self.alarms.append( 0 )
            self.s1.append(-1)
            self.s2.append(-1)
            self.h_values.append(-1)
        
        self.data.append(x)
        alarm_level = self.alarms[-1]
        
        #just raise an alarm if previous timepoint was also alarming
        if len(self.alarms) > 1 and self.alarms[-2] == 0:
            alarm_level = 0
            
        # do not raise an alarm if we do not have enough historical data
        if len(self.alarms) < np.ceil(0.2 * self.window_len):
            alarm_level = 0
        
        if np.abs(alarm_level) < 0.001:
            alarm_level = 0
           
        return alarm_level, self.s1[-1], self.s2[-1], self.h_values[-1]


if __name__ == "__main__":
    
    data = list(np.random.randint(0,10,100))
    data.extend(list(np.random.randint(50,60,50)))
    alarms = []
    cus = cusum (1,10,5)
    for d in data:
       a, s1, s2, h = cus.change_point(d)
       alarms.append(a)
    
    plt.figure()
    plt.title("Data")
    plt.plot(data)
    
    plt.figure()
    plt.title("Alarm Levels")
    plt.plot(alarms)
    
    plt.show()
