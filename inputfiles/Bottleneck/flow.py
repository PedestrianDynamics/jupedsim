import numpy as np
from xml.dom import minidom
from sys import argv ,exit
import os
import time
from os import path, system
from matplotlib.pyplot import *

SIM = 0 # 0: shutdown simulations, 1: make simulations

HOME = path.expanduser("~")
TRUNK = HOME + "/Workspace/peddynamics/JuPedSim/JPSgcfm/trunk/"
GEODIR = TRUNK + "inputfiles/Bottleneck/"
TRAJDIR = TRUNK + "outputfiles/"
CWD = os.getcwd()



# if len(argv) == 1:
#    print "Usage: python dom1.py file"
#    exit()
# file = argv[1]


def sh(script):
	# run bash command
	system("bash -c '%s'" % script)
	
def parse_file(file):
	"""
	parse trajectories in Travisto-format and output results
	in the following  format: id    frame    x    y
	(no sorting of the data is performed)
	returns
	N: number of pedestrians
	data: trajectories
	"""
	xmldoc = minidom.parse(file)
	N = int(xmldoc.getElementsByTagName('agents')[0].childNodes[0].data)
	fps = int(xmldoc.getElementsByTagName('frameRate')[0].childNodes[0].data)
	print "Npeds = ", N
	print "fps = ",fps
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
	- N: number od peds
	- data: trajectories
	- x0: x-coordinate of the vertical measurement line
	output:
	- flow
	"""
	if not isinstance(data, np.ndarray):
		exit("ERROR: flow accepts data of type <ndarray>")
	peds = np.unique(data[:,0]).astype(int)
	times = []
	for ped in peds:
		d = data[ data[:,0] == ped ]
		first = min( d[ d[:,2] >= x0 ][:,1] )
		times.append( first )
		#print "ped", ped
		#print "first", first
	if len(times) < 2:
		print "WARNING: times=", times, " return 0"
		return 0	
	flow = fps * float(N-1) / ( max(times) - min(times) )
	return flow


if __name__ == "__main__":
	widths = [0.8, 0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0]
	flows = []

	for w in widths:
		geofile = GEODIR + str(w) + "_" + "bottleneck.xml"
		trajfile = TRAJDIR + str(w) + "_" + "TrajBottleneck.xml"
		inifile =  TRUNK + str(w) + "_ini-Bottleneck.xml"
		if SIM:
			if path.exists(trajfile): #delete old simulations
				remove = "rm %s"%trajfile
				sh(remove)

		if not path.exists(geofile):
			exit("ERROR: geofile <%s> does not exist"%geofile)
		if not path.exists(inifile):
			exit("ERROR: inifile <%s> does not exist"%inifile)

		if SIM:
			cmd = "./rebuild.exe --inifile=%s"%inifile
			os.chdir(TRUNK) #cd to the simulation directory
			sh(cmd) #make simulation
			os.chdir(CWD) # cd back to the working directory

		if not path.exists(trajfile):
			exit("ERROR: trajfile <%s> does not exit"%trajfile)
		print "parsing ", trajfile
		fps, N, traj = parse_file(trajfile)
		J = flow(fps, N, traj, 6100)
		flows.append(J)

	print "flows: ", flows
	if len(flows) > 1:
		ms = 10 #marker size
		plot(widths, flows, "o-", lw = 2, ms = ms, label = "simulation")
		sey = np.loadtxt("bck-b-scaling/seyfried-j-b.dat")
		kretz = np.loadtxt("bck-b-scaling/kretz-j-b.dat")
		muel32 = np.loadtxt("bck-b-scaling/mueller-bg-32-No.dat")
		muel26 = np.loadtxt("bck-b-scaling/mueller-bg-26-No.dat")
		plot(sey[:,0], sey[:,3], "s", ms = ms, label = "Seyfried")
		plot(kretz[:,0], kretz[:,1], "D", ms = ms, label = "Kretz")
		plot(muel32[:,0], muel32[:,1]/muel32[:,2], "*", ms = ms, label = "Mueller b_g=3.2m")
		plot(muel26[:,0], muel26[:,1]/muel26[:,2], "x", ms = ms, label = "Mueller b_g=2.6m")
		legend(loc='best')
		grid()
		figname = "flow_pngs/flow_" + str( int (time.time() ) ) + ".png"
		savefig(figname)
		show()
