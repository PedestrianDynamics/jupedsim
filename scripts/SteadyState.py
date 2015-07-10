import argparse
import scipy.stats
from numpy import *
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from scipy.stats.stats import pearsonr
from scipy.stats.kde import gaussian_kde

frame = 16

alpha = 0.99
gamma = 0.99
q = scipy.stats.norm.ppf(alpha)
s_max = 100
dx = 0.01

NN = s_max
KK = 500
im = 3.2
d = 2*im/KK
xi = arange(-im, im+0.0001, d)
ia = ib = 0
dnorm = 0

header = 'start end'

def F(x):
    if x > q:
        return 1
    else:
        return -1
        
def func_b(i, k):
    return d * exp(-(xi[i]-xi[k]*acf) * (xi[i]-xi[k]*acf) / (2*(1-acf*acf))) / sqrt(2*math.pi*(1-acf*acf))

def getParserArgs():
    parser = argparse.ArgumentParser(description='Combine French data to one file')
    parser.add_argument("-p", "--filepath", default="./", help='give the path of the input file')
    parser.add_argument("-n", "--filename", default="test", help='give the name of the input file')    
    parser.add_argument("-rs", "--reference_rho_start", default=240, help='give the start frame of the reference process in density')
    parser.add_argument("-re", "--reference_rho_end", default=640, help='give the end frame of the reference process in density')
    parser.add_argument("-vs", "--reference_v_start", default=240, help='give the start frame of the reference process in speed')
    parser.add_argument("-ve", "--reference_v_end", default=640, help='give the end frame of the reference process in speed')
    parser.add_argument("-f", "--plotfigs", default="yes", help='choose to plot the figures or not')
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    rho_max = 8.0
    args = getParserArgs()
    filepath = args.filepath
    #sys.path.append(filepath)
    filename = args.filename
    ref_rho_start = int(args.reference_rho_start)
    ref_rho_end = int(args.reference_rho_end)
    ref_v_start = int(args.reference_v_start)
    ref_v_end = int(args.reference_v_end)
    plotfigs = args.plotfigs

# input data
data = loadtxt('%s%s'%(filepath, filename))
data = data[ data[:,1] != 0 ]
data[:,0] = data[:,0] - data[0,0]

# calculate ref_mean, ref_std, ref_acf for rho
ref_rho = data
ref_rho = ref_rho[ ref_rho[:,0] >= ref_rho_start ]
ref_rho = ref_rho[ ref_rho[:,0] <= ref_rho_end ]
ref_rho = ref_rho[:,1]
ref_rho_mean = mean(ref_rho)
ref_rho_std = std(ref_rho)
ref_rho_a = ref_rho[:-1]
ref_rho_b = ref_rho[1:]
ref_rho_acf = pearsonr(ref_rho_a, ref_rho_b)
ref_rho_acf = ref_rho_acf[0]

# calculate ref_mean, ref_std, ref_acf for v
ref_v = data
ref_v = ref_v[ ref_v[:,0] >= ref_v_start ]
ref_v = ref_v[ ref_v[:,0] <= ref_v_end ]
ref_v = ref_v[:,2]
ref_v_mean = mean(ref_v)
ref_v_std = std(ref_v)
ref_v_a = ref_v[:-1]
ref_v_b = ref_v[1:]
ref_v_acf = pearsonr(ref_v_a, ref_v_b)
ref_v_acf = ref_v_acf[0]

# calculate theta rho
acf = ref_rho_acf

for i in range(len(xi)):
    if ia == 0 and xi[i+1] > -q:
        ia = i
    if ib == 0 and xi[i] > q:
        ib = i
    dnorm += 1 / sqrt(2*math.pi) * exp(-xi[i]*xi[i]/2)
    
shape = ((NN+1)*(KK+1), 1)
bb = matrix(zeros(shape))
bb[-1:, 0] = dnorm

Ta = [matrix(zeros((KK+1, KK+1))) for i in range(NN+1)]
Tb = list(Ta)
Tc = list(Ta)
Td = [matrix(zeros((KK+1, 1))) for i in range(NN+1)]

for i in range(NN+1):    
    begin = (i)*(KK+1)
    end = begin + KK + 1
    Td[i] = bb[begin:end, 0]

B1 = matrix(zeros((KK+1, KK+1)))
B2 = matrix(B1)
Id = matrix(B1)

for i in range(KK+1):
    for j in range(KK+1):
        if i < ia or i >= ib:
            B1[i,j] = func_b(i,j)
        if i >= ia and i < ib:
            B2[i,j] = func_b(i,j)
        if i == j:
            Id[i,j] = 1

Tb[0] = matrix(B2 - Id)
Tc[0] = matrix(B2)
for i in range(1, NN):
    Ta[i] = matrix(B1)
    Tb[i] = matrix(-Id)
    Tc[i] = matrix(B2)
Ta[NN] = matrix(B1)
Tb[NN] = matrix(B1 - Id)

Tc[0] = linalg.solve(Tb[0], Tc[0])
Td[0] = linalg.solve(Tb[0], Td[0])
for i in range(1, NN+1):
    AA = Tb[i] - Ta[i] * Tc[i-1]
    Tc[i] = linalg.solve(AA, Tc[i])
    Td[i] = linalg.solve(AA, Td[i] - Ta[i] * Td[i-1])
for i in range(NN-1, -1, -1):
    Td[i] = Td[i] - Tc[i] * Td[i+1]

Tms = matrix(zeros(shape))
for i in range(NN+1):
    begin = (i)*(KK+1)
    end = begin + KK + 1
    Tms[begin:end, 0] = Td[i]
Tms = Tms / sum(d*Tms)

shape = ((NN+1), 1)
Tm = zeros(shape)
for j in range(NN+1):
    begin = (j)*(KK+1)
    end = begin + KK + 1
    Tm[j, 0] = sum(d * Tms[begin:end, 0])

Tps = Tm[0, 0]
theta = 1
while theta+1 < len(Tm) and Tps+Tm[theta, 0] < gamma:
    Tps = Tps + Tm[theta, 0]
    theta = theta + 1
rho_theta = theta
print('rho_theta = ', rho_theta)

# calculate theta v
acf = ref_v_acf

for i in range(len(xi)):
    if ia == 0 and xi[i+1] > -q:
        ia = i
    if ib == 0 and xi[i] > q:
        ib = i
    dnorm += 1 / sqrt(2*math.pi) * exp(-xi[i]*xi[i]/2)

shape = ((NN+1)*(KK+1), 1)
bb = matrix(zeros(shape))
bb[-1:, 0] = dnorm

Ta = [matrix(zeros((KK+1, KK+1))) for i in range(NN+1)]
Tb = list(Ta)
Tc = list(Ta)
Td = [matrix(zeros((KK+1, 1))) for i in range(NN+1)]

for i in range(NN+1):    
    begin = (i)*(KK+1)
    end = begin + KK + 1
    Td[i] = bb[begin:end, 0]

B1 = matrix(zeros((KK+1, KK+1)))
B2 = matrix(B1)
Id = matrix(B1)

for i in range(KK+1):
    for j in range(KK+1):
        if i < ia or i >= ib:
            B1[i,j] = func_b(i,j)
        if i >= ia and i < ib:
            B2[i,j] = func_b(i,j)
        if i == j:
            Id[i,j] = 1

Tb[0] = matrix(B2 - Id)
Tc[0] = matrix(B2)
for i in range(1, NN):
    Ta[i] = matrix(B1)
    Tb[i] = matrix(-Id)
    Tc[i] = matrix(B2)
Ta[NN] = matrix(B1)
Tb[NN] = matrix(B1 - Id)

Tc[0] = linalg.solve(Tb[0], Tc[0])
Td[0] = linalg.solve(Tb[0], Td[0])
for i in range(1, NN+1):
    AA = Tb[i] - Ta[i] * Tc[i-1]
    Tc[i] = linalg.solve(AA, Tc[i])
    Td[i] = linalg.solve(AA, Td[i] - Ta[i] * Td[i-1])
for i in range(NN-1, -1, -1):
    Td[i] = Td[i] - Tc[i] * Td[i+1]

Tms = matrix(zeros(shape))
for i in range(NN+1):
    begin = (i)*(KK+1)
    end = begin + KK + 1
    Tms[begin:end, 0] = Td[i]
Tms = Tms / sum(d*Tms)

shape = ((NN+1), 1)
Tm = zeros(shape)
for j in range(NN+1):
    begin = (j)*(KK+1)
    end = begin + KK + 1
    Tm[j, 0] = sum(d * Tms[begin:end, 0])

Tps = Tm[0, 0]
theta = 1
while theta+1 < len(Tm) and Tps+Tm[theta, 0] < gamma:
    Tps = Tps + Tm[theta, 0]
    theta = theta + 1
v_theta = theta
print('v_theta = ', v_theta)

# calculate statistics rho
statistics_rho = data
statistics_rho = statistics_rho[:,1]
file_rho_s = open('%s/cusum_rho_%s.txt'%(filepath, filename), 'w')
file_rho_s.write('# frame s \n')
rho_s_frame = 0
rho_s = s_max
file_rho_s.write('%.0f %.4f \n'%(rho_s_frame, rho_s))
for i in statistics_rho:
    rho_s_frame = rho_s_frame + 1
    rho_s = min(max(0, rho_s + F(abs((i-ref_rho_mean)/ref_rho_std))), s_max)
    file_rho_s.write('%.0f %.4f \n'%(rho_s_frame, rho_s))
file_rho_s.close()

# calculate statistics v
statistics_v = data
statistics_v = statistics_v[:,2]
file_v_s = open('%s/cusum_v_%s.txt'%(filepath, filename), 'w')
file_v_s.write('# frame s \n')
v_s_frame = 0
v_s = s_max
file_v_s.write('%.0f %.4f \n'%(v_s_frame, v_s))
for i in statistics_v:
    v_s_frame = v_s_frame + 1
    v_s = min(max(0, v_s + F(abs((i-ref_v_mean)/ref_v_std))), s_max)
    file_v_s.write('%.0f %.4f \n'%(v_s_frame, v_s))
file_v_s.close()

# choose steady state rho
statistics_rho = loadtxt('%s/cusum_rho_%s.txt'%(filepath, filename))
steady_rho = statistics_rho[ statistics_rho[:,1] < rho_theta ]
steady_rho_start_list = []
steady_rho_end_list = []
steady_rho_start = min(steady_rho[:,0]) - (s_max-rho_theta)
steady_rho_start_list.extend([steady_rho_start])
for i in arange(1, len(steady_rho), 1):
    if steady_rho[i,0] - steady_rho[i-1,0] != 1:
        steady_rho_end = steady_rho[i-1,0] - rho_theta
        steady_rho_end_list.extend([steady_rho_end])
        steady_rho_start = steady_rho[i,0] - (s_max-rho_theta)
        steady_rho_start_list.extend([steady_rho_start])
if len(steady_rho_start_list) != len(steady_rho_end_list):
    steady_rho_end = max(steady_rho[:,0]) - rho_theta
    steady_rho_end_list.extend([steady_rho_end])
steady_rho_list = []
steady_rho_list.append(steady_rho_start_list)
steady_rho_list.append(steady_rho_end_list)
steady_rho_array = array(steady_rho_list)
steady_rho_array = steady_rho_array.T
savetxt('%s/SteadyState_rho_%s.txt'%(filepath, filename), steady_rho_array, fmt='%d\t%d', header=header, newline='\r\n')

# choose steady state v
statistics_v = loadtxt('%s/cusum_v_%s.txt'%(filepath, filename))
steady_v = statistics_v[ statistics_v[:,1] < v_theta ]
steady_v_start_list = []
steady_v_end_list = []
steady_v_start = min(steady_v[:,0]) - (s_max-v_theta)
steady_v_start_list.extend([steady_v_start])
for i in arange(1, len(steady_v), 1):
    if steady_v[i,0] - steady_v[i-1,0] != 1:
        steady_v_end = steady_v[i-1,0] - v_theta
        steady_v_end_list.extend([steady_v_end])
        steady_v_start = steady_v[i,0] - (s_max-v_theta)
        steady_v_start_list.extend([steady_v_start])
if len(steady_v_start_list) != len(steady_v_end_list):
    steady_v_end = max(steady_v[:,0]) - v_theta
    steady_v_end_list.extend([steady_v_end])
steady_v_list =[]
steady_v_list.append(steady_v_start_list)
steady_v_list.append(steady_v_end_list)
steady_v_array = array(steady_v_list)
steady_v_array = steady_v_array.T
savetxt('%s/SteadyState_v_%s.txt'%(filepath, filename), steady_v_array, fmt='%d\t%d', header=header, newline='\r\n')

# calculate steady state
steady_start_list = []
steady_end_list = []
for i in range(len(steady_v_array[:,0])):
        for j in range(len(steady_rho_array[:,0])):
            if steady_v_array[i,0] <= steady_rho_array[j,0]:
                if steady_v_array[i,1] > steady_rho_array[j,0] and steady_v_array[i,1] < steady_rho_array[j,1]:
                    steady_start = steady_rho_array[j,0]
                    steady_end = steady_v_array[j,1]
                if steady_v_array[i,1] >= steady_rho_array[j,1]:
                    steady_start = steady_rho_array[j,0]
                    steady_end = steady_rho_array[j,1]
            if steady_v_array[i,0] > steady_rho_array[j,0] and steady_v_array[i,0] <= steady_rho_array[j,1]:
                if steady_v_array[i,1] <= steady_rho_array[j,1]:
                    steady_start = steady_v_array[j,0]
                    steady_end = steady_v_array[j,1]
                if steady_v_array[i,1] > steady_rho_array[j,1]:
                    steady_start = steady_v_array[j,0]
                    steady_end = steady_rho_array[j,1]
            steady_start_list.extend([steady_start])
            steady_end_list.extend([steady_end])
steady_list =[]
steady_list.append(steady_start_list)
steady_list.append(steady_end_list)
steady_array = array(steady_list)
steady_array = steady_array.T
savetxt('%s/SteadyState_%s.txt'%(filepath, filename), steady_array, fmt='%d\t%d', header=header, newline='\r\n')

if plotfigs == 'yes':

    # plot cusum rho
    fig = plt.figure(figsize=(11, 10), dpi=100)
    limit = (int((statistics_rho[-1,0]/frame)/10)+1)*10    
    plt.plot(statistics_rho[:,0]/frame, statistics_rho[:,1], 'b--', lw=2, label=r'S$_{k}$')
    plt.plot([0,limit], [rho_theta, rho_theta], 'r-', lw=2, label=r'$\theta$')
    plt.xlabel('t [s]', fontsize=25)
    plt.ylabel('Statistics rho', fontsize=25)
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    plt.xlim(0, limit)
    plt.ylim(-10, 120)
    plt.legend(numpoints=2, ncol=1, loc=1, fontsize=20)
    plt.savefig('%s/cusum_rho_%s.png'%(filepath, filename))
    
    # plot cusum v
    fig = plt.figure(figsize=(11, 10), dpi=100)
    limit = (int((statistics_v[-1,0]/frame)/10)+1)*10    
    plt.plot(statistics_v[:,0]/frame, statistics_v[:,1], 'b--', lw=2, label=r'S$_{k}$')
    plt.plot([0,limit], [v_theta, v_theta], 'r-', lw=2, label=r'$\theta$')
    plt.xlabel('t [s]', fontsize=25)
    plt.ylabel('Statistics v', fontsize=25)
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    plt.xlim(0, limit)
    plt.ylim(-10, 120)
    plt.legend(numpoints=2, ncol=1, loc=1, fontsize=20)
    plt.savefig('%s/cusum_v_%s.png'%(filepath, filename))
    
    # plot steady rho
    fig = plt.figure(figsize=(11,10), dpi=100)
    ax = fig.add_subplot(111)
    limit = (int((statistics_rho[-1,0]/frame)/10)+1)*10    
    plt.plot((data[:,0]-data[0,0])/frame, data[:,1], 'b-', lw=2)
    plt.plot([ref_rho_start/frame,ref_rho_start/frame], [0,50], 'g--', lw=2, label='reference process')
    plt.plot([ref_rho_end/frame,ref_rho_end/frame], [0,50], 'g--', lw=2)
    for i in range(len(steady_rho_array[:,0])):
        ax.add_patch(mpatches.Polygon([[steady_rho_array[i,0]/frame,0],[steady_rho_array[i,1]/frame,0],[steady_rho_array[i,1]/frame,50],[steady_rho_array[i,0]/frame,50]], closed=True, fill=False, color='r', hatch='/', label='steady state (rho)'))
    for i in range(len(steady_array[:,0])):
        ax.add_patch(mpatches.Polygon([[steady_array[i,0]/frame,0],[steady_array[i,1]/frame,0],[steady_array[i,1]/frame,50],[steady_array[i,0]/frame,50]], closed=True, fill=True, color='y', alpha=0.2, label='steady state (final)'))
    plt.xlabel('t [s]', fontsize=25)
    plt.ylabel(r'$\rho$ [m$^{-2}$]', fontsize=25)
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    plt.xlim(0, limit)
    plt.ylim(0, int(max(data[:,1]))+2)
    plt.legend(numpoints=1, ncol=1, loc=1, fontsize=20)
    plt.savefig('%s/SteadyState_rho_%s.png'%(filepath, filename))
    
    # plot steady v
    fig = plt.figure(figsize=(11,10), dpi=100)
    ax = fig.add_subplot(111)
    limit = (int((statistics_v[-1,0]/frame)/10)+1)*10    
    plt.plot((data[:,0]-data[0,0])/frame, data[:,2], 'b-', lw=2)
    plt.plot([ref_v_start/frame,ref_v_start/frame], [0,50], 'g--', lw=2, label='reference process')
    plt.plot([ref_v_end/frame,ref_v_end/frame], [0,50], 'g--', lw=2)
    for i in range(len(steady_v_array[:,0])):
        ax.add_patch(mpatches.Polygon([[steady_v_array[i,0]/frame,0],[steady_v_array[i,1]/frame,0],[steady_v_array[i,1]/frame,50],[steady_v_array[i,0]/frame,50]], closed=True, fill=False, color='r', hatch='/', label='steady state (v)'))
    for i in range(len(steady_array[:,0])):
        ax.add_patch(mpatches.Polygon([[steady_array[i,0]/frame,0],[steady_array[i,1]/frame,0],[steady_array[i,1]/frame,50],[steady_array[i,0]/frame,50]], closed=True, fill=True, color='y', alpha=0.2, label='steady state (final)'))
        plt.xlabel('t [s]', fontsize=25)
    plt.ylabel(r'$v$ [m$\cdot$s$^{-1}$]', fontsize=25)
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    plt.xlim(0, limit)
    plt.ylim(0, int(max(data[:,2]))+1)
    plt.legend(numpoints=1, ncol=1, loc=1, fontsize=20)
    plt.savefig('%s/SteadyState_v_%s.png'%(filepath, filename))
