import argparse
import numpy as np
import matplotlib.pyplot as plt

def get_parser_args_Nt():
    """
    parse arguments for plot_Nt.py
    """
    parser = argparse.ArgumentParser(description='Plot N-t graph measured by method A')
    parser.add_argument("-f", "--inifile", help='jpsreport inifile (required)', required=True)
    parser.add_argument("-i", "--id", help='id of the measurement area (optional)', nargs='?', default=-1, type=int)
    args = parser.parse_args()
    return args

def plot_nt(measurement_id, files):
    """
    plot N(t) curves (Method A)
    args:
    measurement area id (int)
    """
    legend = False
    fig = plt.figure(dpi=300)
    ax1 = fig.add_subplot(111, aspect='auto')
    for f in files:
        data = np.loadtxt(f)
        Id = int(f.split("id_")[-1].split(".")[0])
        if measurement_id == -1:
            plt.plot(data[:, 0], data[:, 1], label="{}".format(Id))
            legend = True
            continue
        
        if Id == measurement_id:
            plt.plot(data[:, 0], data[:, 1])

    plt.xlabel("$t$", size=18)
    plt.ylabel("$N$", size=18)
    if legend:
        plt.legend()
    plt.grid(alpha=0.7)
    figname = "Nt_{}.png".format(measurement_id)
    plt.savefig(figname)
    print("---> ", figname)

def get_ids_nt_files(nt_files):
    """
    extract ids from nt_files
    return a sorted list.
    """
    ids = set([-1])
    for f in nt_files:
        i = int(f.split("id_")[-1].split(".")[0])
        ids.add(i)
        
    return list(sorted(ids))

