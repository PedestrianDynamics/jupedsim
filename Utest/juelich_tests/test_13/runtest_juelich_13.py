#!/usr/bin/env python3
import numpy as np
import os, logging, time, sys
from sys import argv, exit
import subprocess, glob
from os import path
import multiprocessing 
import matplotlib.pyplot as plt

def get_empirical_flow():
    files = glob.glob("experiments/*.txt")
    width_flow = {}
    names = []
    for f in files:
        names.append(os.path.basename(f).split(".")[0])
        data = np.loadtxt(f)
        widths = data[:, 0]
        flows = data[:, 1]

        for (width, flow) in zip(widths, flows):
            if width not in width_flow:
                width_flow[width] = [flow]
            else:
                width_flow[width].append(flow)

    results = []
    for w in list(width_flow.keys()):
        mean_J = np.mean(width_flow[w])
        std_J = np.std(width_flow[w])
        results.append([w, mean_J, std_J])

    return np.sort(np.array(results), axis=0), names


def process_inifile(inifile):
    width_size = float(inifile.split("geometry_")[1].split("_")[0])
    trajfile = "trajectories/traj" + inifile.split("ini")[2]
    if not path.exists(trajfile):
        logging.critical("trajfile <%s> does not exist"%trajfile)
        exit(FAILURE)

    fps, N, traj = parse_file(trajfile)
    J = flow(fps, N, traj, 61)
    return [N, width_size, J]

#=========================
testnr = 13
#========================

#--------------------------------------------------------
logfile="log_test_%d.txt"%testnr
f=open(logfile, "w")
f.close()
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
DIR= os.path.dirname(os.path.realpath(argv[0]))
CWD = os.getcwd()
print(DIR)
#--------------------------------------------------------
if __name__ == "__main__":
    if CWD != DIR:
        logging.info("working dir is %s. Change to %s"%(os.getcwd(), DIR))
        os.chdir(DIR)

    logging.info("change directory to ..")
    os.chdir("../..")
    lib_path = os.getcwd()
    print(lib_path)
    logging.info("call makeini.py with -f %s/master_ini.xml"%DIR)
    subprocess.call(["python", "makeini.py", "-f", "%s/master_ini.xml"%DIR])
    os.chdir(DIR)
    #-------- get directory of the code TRUNK
    os.chdir("../..")
    # TRUNK = os.getcwd()
    # os.chdir(DIR)
    #    lib_path = os.path.abspath(lib_path)
    sys.path.append(lib_path)
    from utils import SUCCESS, FAILURE
    from utils import parse_file, flow
    # from utils import get_maxtime
    os.chdir("..")
    TRUNK = os.getcwd()
    print("TRUNk", TRUNK)
    os.chdir(DIR)
    #----------------------------------------
    logging.info("change directory back to %s"%DIR)
    
    
    tolerance = 0.5# todo: maybe too large
    time1 = time.clock()
    for e in ["png", "txt"]:
        if os.path.isfile("flow.%s"%e):
            os.remove("flow.%s"%e)
        
    # geofile = "%s/geometry.xml"%DIR
    inifiles = glob.glob("inifiles/*.xml")
    # if not path.exists(geofile):
    #     logging.critical("geofile <%s> does not exist"%geofile)
    #     exit(FAILURE)
        
    executable = "%s/bin/jpscore"%TRUNK
    if not path.exists(executable):
        logging.critical("executable <%s> does not exist yet."%executable)
        exit(FAILURE)
        
    for inifile in inifiles:
        if not path.exists(inifile):
            logging.critical("inifile <%s> does not exist"%inifile)
            exit(FAILURE)
        print(inifile)
        cmd = "%s --inifile=%s"%(executable, inifile)
        #--------------------- SIMULATION ------------------------  
        #os.chdir(TRUNK) #cd to the simulation directory      
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('\n--------------\n start simulating with exe=<%s>'%(cmd))
        # logging.info('width_size = <%.2f>'%width_size)
        #------------------------------------------------------
        subprocess.call([executable, "--inifile=%s"%inifile])
        #------------------------------------------------------
        logging.info('end simulation ...\n--------------\n')


    # #--------------------- PARSING & FLOW-MEASUREMENT --------        
    pool = multiprocessing.Pool()
    flows = pool.map(process_inifile, inifiles)
    flows = np.array(flows)
    # ----------------------- PLOT RESULTS ----------------------
    flow_file = "flow.txt"
    ff = open(flow_file, "w")
    logging.info('write flow values in \"%s\"'%flow_file)
    # 0   1  2
    # N   W  J
    widths = np.unique(flows[:, 1])
    W = []
    M = []
    S = []
    ff.write("# width  mean  std\n")
    for (i, w) in enumerate(widths):
        X = flows[ flows[:, 1] == w]
        m = np.mean(X[:, 2])
        s = np.std(X[:, 2])

        W.append(w)
        M.append(m)
        S.append(s)
        ff.write("%.2f   %.2f   %.2f\n" %(w, m, s))


    ff.write("\n")
    ff.close()
    #########################################################################
    ms = 8
    plt.errorbar(W, M, yerr=S, fmt="o-", linewidth=1.2, capthick=2, capsize=4)
    plt.plot(W, M, "o", label='Simulation', color='blue')

    plt.legend(loc='best', numpoints=1)
    plt.grid()
    plt.xlabel(r'$w\; [\, \rm{m}\, ]$',fontsize=18)
    plt.ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$',fontsize=18)
    # xticks(jexp[:, 0])
    jexp, names = get_empirical_flow()
    plt.errorbar(jexp[:, 0], jexp[:, 1], yerr=jexp[:, 2], fmt="D-", color='r', ecolor='r', linewidth=2, capthick=2, label = "%s"%", ".join(names))
    plt.axes().set_aspect(1./plt.axes().get_data_ratio()) 
    plt.xlim([np.min(jexp[:, 0]) - 0.1,  np.max(jexp[:, 0]) + 0.1])
    plt.ylim([np.min(jexp[:, 1]) - 0.3,  np.max(jexp[:, 1]) + np.max(jexp[:, 2]) + 0.3])

    err = 0
    num = 0
    for (w, j) in zip(jexp[:, 0], jexp[:, 1]):
            for (m, w0) in zip(M, W):
                if w == w0:
                    num += 1
                    err +=  np.sqrt((m-j)**2)

    err /= num
    plt.title(r"$\frac{1}{N}\sqrt{{\sum_w {(\mu(w)-E(w)})^2 }}=%.2f\; (tol=%.2f)$"%(err, tolerance), y=1.02)
    
    plt.savefig("flow.png")
    #show()
    #########################################################################
    
    time2 = time.clock()
    logging.info("time elapsed %.2f [s]."%(time2-time1))
    logging.info("err = %.2f, tol=%.2f"%(err, tolerance))
    
    if err > tolerance:
        logging.info("exit with failure")
        exit(FAILURE)
    else:
        logging.info("exit with success")
        exit(SUCCESS)
