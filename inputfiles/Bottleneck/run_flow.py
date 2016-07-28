#!/usr/bin/env python3
import numpy as np
from xml.dom import minidom
import os, argparse, logging, time
from os import path, system
from matplotlib.pyplot import *
from sys import argv ,exit
#----------------------------
parser = argparse.ArgumentParser(description='Simulate the bottleneck-scenario with GCFM and calculte the flow with respect to the width of the bottleneck. Widths are [0.8, 0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3, 2.5]')
parser.add_argument("-p", "--plt", type=int , choices=xrange(0, 2), required=True, help='Plot and save the results in flow_pngs/')
parser.add_argument("-s", "--sim" , type=int , choices=xrange(0, 2), required=True, help="Make simulations. Calles rebuild.exe with different ini-files. Old sim-trajectories will be deleted")
parser.add_argument("-i" , "--ini" , type=int , choices=xrange(0, 2), required=True, help="Produce ini-files based on the <ini-Bottleneck.xml> file. Tipp: Change parameters in ini-Bottleneck.xml and use this option to spread the changes to all ini-files")
parser.add_argument("-l" , "--log" , type=argparse.FileType('w'), default='log.txt', help="log file (default log.txt). Tipp: tail -f log.txt to online monitor the progress of this script")
args = parser.parse_args()
#-------------------- CONTROL VARS ----------------------
SIM = args.sim # 0: shutdown simulations, 1: make simulations
MAKE_INI = args.ini # generate new ini-files. 
PLOT = args.plt # plot or not?
logfile = args.log.name
#--------------------------------------------------------
logging.basicConfig(filename=logfile, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

logging.info('SIM = %d'%SIM)
logging.info('MAKE_INI = %d'%MAKE_INI)
logging.info('PLOT = %d'%PLOT)
logging.info('logfile = \"%s\"'%logfile)

#-------------------- DIRS ------------------------------
HOME = path.expanduser("~")
TRUNK = HOME + "/Workspace/peddynamics/JuPedSim/JPSgcfm/trunk/"
GEODIR = TRUNK + "inputfiles/Bottleneck/"
TRAJDIR = TRUNK + "outputfiles/"
CWD = os.getcwd()
#--------------------------------------------------------

def sh(script):
	# run bash command
	system("bash -c '%s'" % script)
	
def parse_file(filename):
	"""
	parse trajectories in Travisto-format and output results
	in the following  format: id    frame    x    y
	(no sorting of the data is performed)
	returns
	N: number of pedestrians
	data: trajectories
	"""
	logging.info("parsing <%s>"%filename)
	try:
		xmldoc = minidom.parse(filename)
	except:
		exit(logging.critical('could not parse file. exit'))
	N = int(xmldoc.getElementsByTagName('agents')[0].childNodes[0].data)

	fps = int(xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data)
	logging.info ("Npeds = %d, fps = %d"%(N, fps))
	frames = xmldoc.childNodes[0].getElementsByTagName('frame')
	data = []
	for frame in frames:
		frame_number = int(frame.attributes["ID"].value)
		for agent in frame.getElementsByTagName("agent"):
			agent_id = int(agent.attributes["ID"].value)
			x = float(agent.attributes["xPos"].value)
			y = float(agent.attributes["yPos"].value)
			data += [agent_id, frame_number, x, y]
	data = np.array(data).reshape((-1,4))
	return fps, N, data
           
def flow(fps, N, data, x0):
	"""
	measure the flow at a vertical line given by <x0>
	trajectories are given by <data> in the following format: id    frame    x    y
	input: 
	- fps: frame per second
	- N: number of peds
	- data: trajectories
	- x0: x-coordinate of the vertical measurement line
	output:
	- flow
	"""
	logging.info('measure flow')
	if not isinstance(data, np.ndarray):
		exit(logging.critical("flow() accepts data of type <ndarray>. exit"))
	peds = np.unique(data[:,0]).astype(int)
	times = []
	for ped in peds:
		d = data[ data[:,0] == ped ]
		first = min( d[ d[:,2] >= x0 ][:,1] )
		times.append( first )
	if len(times) < 2:
		logging.warning("Number of pedestrians passing the line is small. return 0")
		return 0	
	flow = fps * float(N-1) / ( max(times) - min(times) )
	return flow


if __name__ == "__main__":
	time1 = time.clock()
	widths = [0.8, 0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3, 2.5]
	flows = []
	if MAKE_INI:
		sh("python3 makeBottleneckInifile.py")

	for w in widths:
	#------------------------------------------------------------------------------	
		geofile = GEODIR + str(w) + "_" + "bottleneck.xml"
		trajfile = TRAJDIR + str(w) + "_" + "TrajBottleneck.xml"
		inifile =  TRUNK + str(w) + "_ini-Bottleneck.xml"
		if not path.exists(geofile):
			exit(logging.critical("geofile <%s> does not exist"%geofile))
		if not path.exists(inifile):
			exit(logging.critical("inifile <%s> does not exist"%inifile))

		#--------------------- SIMULATION ------------------------	
		if SIM:
			if path.exists(trajfile): #delete old simulations
				remove = "remove <%s>"%trajfile
				sh(remove)
			os.chdir(TRUNK) #cd to the simulation directory
			cmd = "./jpscore --inifile=%s"%inifile
			logging.info('start simulating ...')
			sh(cmd) #make simulation
			os.chdir(CWD) # cd back to the working directory
			
		#--------------------- PARSING & FLOW-MEASUREMENT --------
		if not path.exists(trajfile):
			exit(logging.critical("trajfile <%s> does not exit"%trajfile))
		fps, N, traj = parse_file(trajfile)
		J = flow(fps, N, traj, 6100)
		flows.append(J)
	#------------------------------------------------------------------------------	
	logging.debug("flows: (%s)"%', '.join(map(str, flows)))
	
	# ----------------------- PLOT RESULTS ----------------------
	if len(flows) > 1 and PLOT:
		#timestamp = str( int (time.time() ) )
		timestamp = time.strftime("%d-%b-%Y-%H_%M_%S", time.gmtime())
		flow_file = "flow_pngs/flow_" + timestamp + ".txt"
		ff = open(flow_file, "w")
		logging.info('write flow values in \"%s\"'%flow_file)
		print >>ff, flows
		ff.close
		ms = 10 #marker size
		plot(widths, flows, "o-", lw = 2, ms = ms, label = "simulation")
		sey = np.loadtxt("bck-b-scaling/seyfried-j-b.dat")
		kretz = np.loadtxt("bck-b-scaling/kretz-j-b.dat")
		muel32 = np.loadtxt("bck-b-scaling/mueller-bg-32-No.dat")
		muel26 = np.loadtxt("bck-b-scaling/mueller-bg-26-No.dat")
		lid = np.loadtxt("bck-b-scaling/Flow_vs_b_v2.dat")
		plot(sey[:,0], sey[:,3], "s", ms = ms, label = "Seyfried")
		plot(kretz[:,0], kretz[:,1], "D", ms = ms, label = "Kretz")
		#plot(muel32[:,0], muel32[:,1]/muel32[:,2], "*", ms = ms, label = "Mueller b_g=3.2m")
		#plot(muel26[:,0], muel26[:,1]/muel26[:,2], "x", ms = ms, label = "Mueller b_g=2.6m")
		plot(lid[:,0]/100.0 , 150.0/(lid[:,4]-lid[:,1]), "^k", ms = ms, label = "Liddle")
		axes().set_aspect(1./axes().get_data_ratio())  # Das Bild ist quadratisch
		legend(loc='best')
		grid()
		xlabel(r'$w\; [\, \rm{m}\, ]$',fontsize=18)
		ylabel(r'$J\; [\, \frac{1}{\rm{s}}\, ]$',fontsize=18)
		xticks([0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3, 2.5])
		xlim([0.7, 2.6])
		ylim([1, 6])
		
		figname = "flow_pngs/flow_" + timestamp + ".png"
		logging.info("saving figure in \"%s\" "%figname)
		savefig(figname)
		#logging.info("svn add %s"%figname)
		#sh("svn add %s"%figname)
		#logging.info("svn add %s"%flow_file)
		#sh("svn add %s"%flow_file)
		#show()
	time2 = time.clock()
	logging.info("time elapsed %.2f [s]"%(time2-time1))
