# run script without arguments for help
import sys
import os
from xml.dom.minidom import parse
import glob
import argparse
import numpy as np
import matplotlib.pyplot as plt

def getParserArgs():
        parser = argparse.ArgumentParser(description='Plot N-t graph measured by method A')
        parser.add_argument("-f", "--inifile", help='jpsreport inifile (required)', required=True)
        parser.add_argument("-i", "--id", help='id of the measurement area (optional)', nargs='?', default=-1, type=int)
        args = parser.parse_args()
        return args

def plot_nt(measurement_id):
    """
    plot N(t) curves (Method A)
    args:
    measurement area id (int)
    """
    legend = False
    fig = plt.figure(figsize=(16, 16), dpi=100)
    ax1 = fig.add_subplot(111,aspect='auto')
    plt.rc("font", size=30)
    plt.rc('pdf',fonttype = 42)
    for f in nt_files:
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

def get_ids(nt_files):
    """
    extract ids from nt_files
    return a sorted list.
    """
    ids = set([-1])
    for f in nt_files:
        i = int(f.split("id_")[-1].split(".")[0])
        ids.add(i)
        
    return list(sorted(ids))
    
if __name__ == '__main__':
    args = getParserArgs()
    jpsreport_inifile = args.inifile
    Id = args.id
    if not os.path.exists(jpsreport_inifile):
        sys.exit("{} does not exist".format(jpsreport_inifile))
        
    d = parse(jpsreport_inifile)
    output_dir =  d.getElementsByTagName('output')[0].attributes.items()[0][1]
    jpsreport_ini_dir = os.path.dirname(jpsreport_inifile)
    flow_nt_files = os.path.join(jpsreport_ini_dir, output_dir, "Fundamental_Diagram", "FlowVelocity", "Flow_NT_*.dat")
    method_A =  d.getElementsByTagName('method_A')[0].attributes.items()[0][1]
    if method_A.lower() == "false":
        sys.exit("method A ist false. Nothing to plot") 

    nt_files = glob.glob(flow_nt_files)
    ids = get_ids(nt_files)
    if not Id in ids:
        sys.exit("{} not in {}".format(Id, ", ".join(map(str,ids))))
    
    plot_nt(Id)
    
