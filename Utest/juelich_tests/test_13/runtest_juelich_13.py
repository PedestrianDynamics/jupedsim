#!/usr/bin/env python3
import numpy as np
import os, logging, time, sys
from sys import argv, exit
import subprocess, glob
from os import path
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
    from utils import get_maxtime, parse_file, flow 
    os.chdir("..")
    TRUNK = os.getcwd()
    print("TRUNk", TRUNK)
    os.chdir(DIR)
    #----------------------------------------
    logging.info("change directory back to %s"%DIR)
    
    flows = {}
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
        width_size = float(inifile.split("geometry_")[1].split("_")[0])
        cmd = "%s --inifile=%s"%(executable, inifile)
        #--------------------- SIMULATION ------------------------  
        #os.chdir(TRUNK) #cd to the simulation directory      
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('\n--------------\n start simulating with exe=<%s>'%(cmd))
        logging.info('width_size = <%.2f>'%width_size)
        #------------------------------------------------------
        subprocess.call([executable, "--inifile=%s"%inifile])
        #------------------------------------------------------
        logging.info('end simulation ...\n--------------\n')
        trajfile = "trajectories/traj" + inifile.split("ini")[2]
        logging.info('trajfile = <%s>'%trajfile)
        #--------------------- PARSING & FLOW-MEASUREMENT --------
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exist"%trajfile)
            exit(FAILURE)
        maxtime = get_maxtime(inifile)
        fps, N, traj = parse_file(trajfile)
        J = flow(fps, N, traj, 61)
        if width_size not in flows:
            flows[width_size] = [J]
        else:
            flows[width_size].append(J)
       
        logging.info("W = %f;  Flow = %f"%(width_size, J))
                     
    #logging.debug("flows: (%s)"%', '.join(map(str, flows)))
        #------------------------------------------------------------------------------ 
    logging.debug("flows: (%s)"%', '.join(map(str, flows)))
    
    # ----------------------- PLOT RESULTS ----------------------
    flow_file = "flow.txt"
    ff = open(flow_file, "w")
    logging.info('write flow values in \"%s\"'%flow_file)
    ff.write("# width: flow values (for different seeds)\n")
    for key, value in list(flows.items()):
        ff.write("%f : %s\n" % (key, ", ".join([str(j) for j in value])))


    M = np.array([np.mean(i) for i in list(flows.values())]) # std pro width
    S = np.array([np.std(i) for i in list(flows.values())])  # std pro width

    ff.write("Means \n")
    ff.write(", ".join([str(m) for m in M]))

    ff.write("\n\nStd \n")
    ff.write(", ".join([str(s) for s in S]))
    ff.write("\n")
    ff.close()
    #########################################################################
    ms = 8
    #plot(widths, flows, "o-b", lw = 2, ms = ms, label = "simulation")
    indexsort = np.argsort( list(flows.keys()) )
    F = np.array( list(flows.keys()) )[indexsort]
    plt.plot(F, np.array(M)[indexsort], "o-", lw=2, label='Simulation', color='blue')
    plt.errorbar(F , np.array(M)[indexsort] , yerr=np.array(S)[indexsort], fmt='-o')

    jexp, names = get_empirical_flow()
    plt.errorbar(jexp[:, 0], jexp[:, 1], yerr=jexp[:, 2], fmt="D-", color='r', ecolor='r', linewidth=2, capthick=2, label = "%s"%", ".join(names))
    plt.axes().set_aspect(1./plt.axes().get_data_ratio())
    columns = np.vstack((F, np.array(M)[indexsort]))
    gg = open("flow_col.txt", "w")
    for i in range(len(F)):
        gg.write("%f  %f\n" % (columns[0][i], columns[1][i]))
        
    gg.close()
    plt.legend(loc='best', numpoints=1)
    plt.grid()
    plt.xlabel(r'$w\; [\, \rm{m}\, ]$',fontsize=18)
    plt.ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$',fontsize=18)
    # xticks(jexp[:, 0])
    plt.xlim([np.min(jexp[:, 0]) - 0.1,  np.max(jexp[:, 0]) + 0.1])
    plt.ylim([np.min(jexp[:, 1]) - 0.3,  np.max(jexp[:, 1]) + np.max(jexp[:, 2]) + 0.3])

    err = 0
    num = 0
    for (w, j) in zip(jexp[:, 0], jexp[:, 1]):
        for key, values in list(flows.items()):
            if key == w:
                num += 1
                err +=  np.sqrt((np.mean(values)-j)**2)
                print("%3.3f  %3.1f  %3.1f  -  %3.3f  %3.3f"%(err, key, w, j, np.mean(values)))
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
