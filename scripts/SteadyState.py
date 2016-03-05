import os
from sys import exit
import argparse
import scipy.stats
from numpy import *
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from scipy.stats.stats import pearsonr
import itertools

VERSION = 0.8
script_dir = os.path.dirname(os.path.realpath(__file__))
demos_dir = os.path.join(os.path.dirname(os.path.abspath(script_dir)), "demos")
demo_file = os.path.join(demos_dir, "SteadyState", "rho_v_Voronoi_traj_AO_b240.txt_id_1.dat")

alpha = 0.99
gamma = 0.99
q = scipy.stats.norm.ppf(alpha)
s_max = 100
dx = 0.01

NN = s_max
KK = 500
im = 3.2
d = 2 * im / KK
xi = arange(-im, im + 0.0001, d)

def overlap(start1, end1, start2, end2):
    """
    Does the range (start1, end1) overlap with (start2, end2)?
    not (end1 < start2 or end2 < start1)
    De Morgan's laws mean that we can change this condition to:
    """
    return end1 >= start2 and end2 >= start1

def get_start_end(startframes, endframes):
    is_overlap = 1
    at_least_one_overlap = 0
    while is_overlap:
        is_overlap = 0
        startframes0 = []
        endframes0 = []
        for (s, e) in zip(startframes, endframes):
            for (s0, e0) in zip(startframes, endframes):
                if s != s0 and e != e0 and overlap(s, e, s0, e0):
                    overlap_s = max(s, s0)
                    overlap_e = min(e, e0)
                    is_overlap = 1
                    # add start of overlap segment
                    if not overlap_s in startframes0:
                        startframes0.append(overlap_s)
                    # add end of overlap segment
                    if not overlap_e in endframes0:
                        endframes0.append(overlap_e)

        if is_overlap:
            startframes = startframes0
            endframes = endframes0
            at_least_one_overlap = 1

    if at_least_one_overlap: # this is for the case that we never had overlaps
        return startframes, endframes
    else:
        return startframes0, endframes0

def F(x):
    """
    :param x:
    :return:
    """
    if x > q:
        return 1
    else:
        return -1

def func_b(i, k, acf):
    """
    :param i:
    :param k:
    :param acf:
    :return:
    """
    return d * exp(-(xi[i] - xi[k] * acf) * (xi[i] - xi[k] * acf) / (2 * (1 - acf * acf))) / sqrt(
        2 * math.pi * (1 - acf * acf))

def getParserArgs():
    """

    :return:
    """
    parser = argparse.ArgumentParser(
        description='Detection of Steady State. JuPedSim v%0.1f.' % VERSION)
    parser.add_argument("-a", "--automatic", action='store_true',
                        help="use calculated reference start and end frames. If given -rs and -re are ignored")
    parser.add_argument("-f", "--input_file", default=demo_file,
                        help='Full name of the input file (default %s). File format is | frame | rho | velocity |' % demo_file)
    parser.add_argument("-c", "--columns", nargs='+', type=int, default=[0, 1, 2],
                        help="columns to read from file (default 0, 1, 2)")
    parser.add_argument("-xl", "--xlabel", type=str, default="t /s",
                        help="xlabel")
    parser.add_argument("-yl", "--ylabels", nargs='+', type=str, default=["\\rho\; /m^{-2}", "v\; /m/s"],
                        help="ylabels. separate units with \"\;\"")
    parser.add_argument("-rs", "--reference_start", nargs='+', type=int, default=[240, 240],
                        help='Start frame of the reference process in density (default 240)')
    parser.add_argument("-re", "--reference_end", nargs='+', type=int, default=[640, 640],
                        help='End frame of the reference process in density (default 640)')
    parser.add_argument("-p", "--plotfigs", default="yes",
                        help='Plot figures (default yes)')
    parser.add_argument("-r", "--fps", type=int, default=16,
                        help='Frame per second (default 16)')
    args = parser.parse_args()
    return args

def set_ref_series(data, column, ref_start, ref_end):
    # calculate ref_mean, ref_std, ref_acf for data[:, column]
    """
    data:
    column:
    start:
    end:
    return: acf, mean, std
    :param data:
    :param column:
    :param ref_start:
    :param ref_end:
    :return:
    """
    ref_series = data
    ref_series = ref_series[ref_series[:, 0] >= ref_start - minframe]
    ref_series = ref_series[ref_series[:, 0] <= ref_end - minframe]
    ref_series = ref_series[:, column]
    ref_series_mean = mean(ref_series)
    ref_series_std = std(ref_series)
    ref_series_a = ref_series[:-1]
    ref_series_b = ref_series[1:]
    ref_series_acf = pearsonr(ref_series_a, ref_series_b)
    ref_series_acf = ref_series_acf[0]
    return ref_series_acf, ref_series_mean, ref_series_std

def get_theta(ia, ib, dnorm, acf):
    """

    :param ia:
    :param ib:
    :param dnorm:
    :param acf:
    :return:
    """
    shape = ((NN + 1) * (KK + 1), 1)
    bb = matrix(zeros(shape))
    bb[-1:, 0] = dnorm
    Ta = [matrix(zeros((KK + 1, KK + 1))) for i in range(NN + 1)]
    Tb = list(Ta)
    Tc = list(Ta)
    Td = [matrix(zeros((KK + 1, 1))) for i in range(NN + 1)]
    for i in range(NN + 1):
        begin = (i) * (KK + 1)
        end = begin + KK + 1
        Td[i] = bb[begin:end, 0]
    B1 = matrix(zeros((KK + 1, KK + 1)))
    B2 = matrix(B1)
    Id = matrix(B1)
    for i in range(KK + 1):
        for j in range(KK + 1):
            if i < ia or i >= ib:
                B1[i, j] = func_b(i, j, acf)
            if i >= ia and i < ib:
                B2[i, j] = func_b(i, j, acf)
            if i == j:
                Id[i, j] = 1
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
    for i in range(1, NN + 1):
        AA = Tb[i] - Ta[i] * Tc[i - 1]
        Tc[i] = linalg.solve(AA, Tc[i])
        Td[i] = linalg.solve(AA, Td[i] - Ta[i] * Td[i - 1])
    for i in range(NN - 1, -1, -1):
        Td[i] = Td[i] - Tc[i] * Td[i + 1]
    Tms = matrix(zeros(shape))
    for i in range(NN + 1):
        begin = (i) * (KK + 1)
        end = begin + KK + 1
        Tms[begin:end, 0] = Td[i]
    Tms = Tms / sum(d * Tms)
    shape = ((NN + 1), 1)
    Tm = zeros(shape)
    for j in range(NN + 1):
        begin = (j) * (KK + 1)
        end = begin + KK + 1
        Tm[j, 0] = sum(d * Tms[begin:end, 0])
    Tps = Tm[0, 0]
    theta = 1
    while theta + 1 < len(Tm) and Tps + Tm[theta, 0] < gamma:
        Tps = Tps + Tm[theta, 0]
        theta += 1
    rho_theta = theta
    return rho_theta

def init_parameters():
    """
    init ia, ib and dnorm
    :return:
    """
    ia = ib = 0
    dnorm = 0
    for i in range(len(xi)):
        if ia == 0 and xi[i + 1] > -q:
            ia = i
        if ib == 0 and xi[i] > q:
            ib = i
        dnorm += 1 / sqrt(2 * math.pi) * exp(-xi[i] * xi[i] / 2)

    return ia, ib, dnorm

def calculate_statistics(data, column, ref_mean, ref_std):
    """
    writes result in file
    :param data:
    :param column:
    :param ref_mean:
    :param ref_std:
    """
    statistics_series = data[:, column]
    file_s = open('%s/cusum_%d_%s.txt' % (filepath, column, filename), 'w')
    file_s.write('# frame s \n')
    s_frame = minframe
    s = s_max
    file_s.write('%.0f %.4f \n' % (s_frame, s))
    for i in statistics_series:
        s_frame += 1
        s = min(max(0, s + F(abs((i - ref_mean) / ref_std))), s_max)
        file_s.write('%.0f %.4f \n' % (s_frame, s))
    file_s.close()

def choose_steady_state(column, theta):
    """

    :param column:
    :param theta:
    :return:
    """
    statistics = loadtxt('%s/cusum_%d_%s.txt' % (filepath, column, filename))
    ss = open('%s/SteadyState_%d_%s.txt' % (filepath, column, filename), 'w')
    ss.write('# start end ratio mean std \n')
    steady = statistics[statistics[:, 1] < theta]
    steady_start = min(steady[:, 0]) - (s_max - theta)
    for i in arange(1, len(steady), 1):
        if steady[i, 0] - steady[i - 1, 0] != 1:
            steady_end = steady[i - 1, 0] - theta
            if steady_start < steady_end:
                series_data = data
                series_data = series_data[series_data[:, 0] >= steady_start]
                series_data = series_data[series_data[:, 0] <= steady_end]
                data_ratio = len(series_data[:, 0]) / len(data[:, 0]) * 100
                data_mean = mean(series_data[:, column])
                data_std = std(series_data[:, column])
                ss.write('%.0f %.0f %.2f %.4f %.4f \n' % (
                    steady_start, steady_end, data_ratio, data_mean, data_std))
            steady_start = steady[i, 0] - (s_max - theta)
    steady_end = max(steady[:, 0]) - theta
    if steady_start < steady_end:
        series_data = data
        series_data = series_data[series_data[:, 0] >= steady_start]
        series_data = series_data[series_data[:, 0] <= steady_end]
        data_ratio = len(series_data[:, 0]) / len(data[:, 0]) * 100
        data_mean = mean(series_data[:, column])
        data_std = std(series_data[:, column])
        ss.write('%.0f %.0f %.2f %.4f %.4f \n' % (
            steady_start, steady_end, data_ratio, data_mean, data_std))
    ss.close()
    info_serie = atleast_2d(loadtxt('%s/SteadyState_%d_%s.txt' % (filepath, column, filename)))
    return info_serie, statistics

def plot_series(statistics, theta, column):
    fig = plt.figure(figsize=(11, 10), dpi=100)
    if xlabel == "frame":
        fps = 1.0  # hack
    else:
        fps = frame

    limit = (int((statistics[-1, 0] / fps) / 10) + 1) * 10
    plt.plot(statistics[:, 0] / fps, statistics[:, 1], 'b--', lw=2, label=r'S$_{k}$')
    plt.plot([0, limit], [theta, theta], 'r-', lw=2, label=r'$\theta$')
    plt.xlabel(xlabel, fontsize=25)
    plt.ylabel('Statistics %d'%column, fontsize=25)
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    plt.xlim(0, limit)
    plt.ylim(-10, 120)
    plt.legend(numpoints=2, ncol=1, loc=1, fontsize=20)
    print('--> %s/cusum_%d_%s.png' % (filepath, column, filename))
    plt.savefig('%s/cusum_%d_%s.png' % (filepath, column, filename))
    plt.close()


def plot_steady_state(statistics, data, ref_start, ref_end, info, start, end, column):
    fig = plt.figure(figsize=(11, 10), dpi=100)
    ax = fig.add_subplot(111)
    if xlabel == "frame":
        fps = 1.0  # hack
    else:
        fps = frame

    limit = (int((statistics[-1, 0] / fps) / 10) + 1) * 10
    plt.plot((data[:, 0] + minframe) / fps, data[:, column], 'b-', lw=2)
    plt.plot([ref_start / fps, ref_start / fps], [0, 50], 'g--', lw=2, label='reference')
    plt.plot([ref_end / fps, ref_end / fps], [0, 50], 'g--', lw=2)
    for i in range(len(info[:, 0])):
        ax.add_patch(mpatches.Polygon([[info[i, 0] / fps, 0],
                                       [info[i, 1] / fps, 0],
                                       [info[i, 1] / fps, 50],
                                       [info[i, 0] / fps, 50]],
                                      closed=True, fill=False,
                                      color='r', hatch='/', label='steady ($%s$)'%ylabels[column-1].split("\;")[0]))

    if start != end:
        ax.add_patch(mpatches.Polygon([[start / fps, 0],
                                       [end / fps, 0],
                                       [end / fps, 50],
                                       [start / fps, 50]],
                                      closed=True, fill=True,
                                      color='y', alpha=0.2, label='steady (final)'))

    plt.xlabel(xlabel, fontsize=25)
    plt.ylabel(r'$%s$'%ylabels[column-1], fontsize=25)
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    plt.xlim(0, limit)
    plt.ylim(0, int(max(data[:, 1])) + 2)
    plt.legend(numpoints=1, ncol=1, loc=1, fontsize=20)
    print('--> %s/SteadyState_%d_%s.png' % (filepath, column, filename))
    plt.savefig('%s/SteadyState_%d_%s.png' % (filepath, column, filename))
    plt.close()

if __name__ == '__main__':
    rho_max = 8.0
    args = getParserArgs()
    input_file = args.input_file
    ref_start = args.reference_start
    ref_end = args.reference_end
    plotfigs = args.plotfigs
    frame = args.fps
    columns = args.columns
    xlabel = args.xlabel
    ylabels = args.ylabels
    # sanity check

    if not args.automatic: # in case references are manually given, lengths should be correct
        assert (len(columns)-1) == len(ref_start) == len(ref_end) == len(ylabels),\
            "mismatch lengths.\n\t columns: %s (first is frame)\n\t ref_start: %s\n\t ref_end: %s\n\t ylabels: %s"%\
            (", ".join(map(str, columns)), ", ".join(map(str, ref_start)), ", ".join(map(str, ref_end)), ", ".join(map(str, ylabels)))

    # read input data
    try:
        data = loadtxt('%s' % (input_file), usecols=columns)# [0, 6, 8]
    except IOError:
        exit("Can not open file <%s>" % input_file)

    data = data[data[:, 1] != 0] # todo: why?
    if args.automatic:
        ref_start = []
        ref_end = []
        for c in range(1, len(columns)):
            start_frame = data[0, 0]
            end_frame = data[-1, 0]
            ref_start.append(int(1./3*(end_frame-start_frame)))
            ref_end.append(int(2./3*(end_frame-start_frame)))

        print("Automatic mode:")
        print("\t ref_start: %s"%", ".join(map(str, ref_start)))
        print("\t ref_end  : %s"%", ".join(map(str, ref_end)))

    minframe = data[0, 0]
    data[:, 0] = data[:, 0] - minframe

    # get filepath and filename
    filename = os.path.basename(input_file).split(".")[0]
    filepath = os.path.dirname(input_file)
    filepath = os.path.join(filepath, "results_%s"%filename)
    if not os.path.exists(filepath):
        os.makedirs(filepath)

    print('file path = %s' % filepath)
    print('file name = %s' % filename)

    ia, ib, dnorm = init_parameters()
    starts = []  # collect start of steady state for all series
    ends = [] # collect end of steady state for all series
    infos = []
    all_statistics = []
    for i in range(len(columns)-1):
        ref_acf, ref_mean, ref_std = set_ref_series(data, i+1, ref_start[i], ref_end[i])
        # calculate theta rho
        theta = get_theta(ia, ib, dnorm, ref_acf)
        print('theta[%d] = %.0f' % (i, theta))
        # calculate statistics
        calculate_statistics(data, i+1, ref_mean, ref_std)
        # choose steady state
        info, statistics = choose_steady_state(i+1, theta)
        infos.append(info)
        all_statistics.append(statistics)
        print('+------------------------------------------------------------------------------+')
        for j in range(info.shape[0]):
            print('steady state of series %d (%d): from %d (%.1f s) to %d (%.1f s) [ratio=%.2f, mean=%.2f, std=%.2f]' % (
                i, j,
                info[j][0], info[j][0] / frame, info[j][1], info[j][1] / frame,
                info[j][2], info[j][3], info[j][4]))

            ends.append(info[j][1])
            starts.append(info[j][0])

        if plotfigs == 'yes':
            # plot cusum
            plot_series(statistics, theta, i+1)

        print('+------------------------------------------------------------------------------+')
        os.remove('%s/cusum_%d_%s.txt' % (filepath, i+1, filename))

    # choose steady state
    ss = open('%s/SteadyState_%s.txt' % (filepath, filename), 'w')
    ss.write('# start end ratio \n')
    print ("start frames: %s" % ", ".join(map(str, starts)))
    print ("end frames: %s" % ", ".join(map(str, ends)))
    mix_starts, mix_ends = get_start_end(starts, ends)
    if mix_starts and mix_ends: #lists are not empty --> there is overlap(s)
        for (mix_start, mix_end) in zip(mix_starts, mix_ends):
            ss_data_ratio = (mix_end - mix_start) / len(data[:, 0]) * 100
            ss.write('%.0f %.0f %.2f \n' % (mix_start, mix_end, ss_data_ratio))
            print('+ final steady state is from %d (%.1f s) to %d (%.1f s)  [ratio=%.2f]' %
                  (mix_start, mix_start / frame, mix_end, mix_end / frame, ss_data_ratio))

        print('Steady state detected successfully!')
        print('+------------------------------------------------------------------------------+')
    else:
        print('Steady state detected with problems: ')
        print('mix_starts: %f'%mix_starts)
        print('mix_ends: %f'%mix_ends)
        print('+------------------------------------------------------------------------------+')

    if plotfigs == 'yes':
        # plot steady
        for (mix_start, mix_end) in zip(mix_starts, mix_ends):
            info = [mix_start, mix_end]
            for i in range(len(columns)-1):
                plot_steady_state(
                    all_statistics[i],
                    data,
                    ref_start[i],
                    ref_end[i],
                    infos[i],
                    mix_start,
                    mix_end,
                    i+1
                )

    ss.close()
