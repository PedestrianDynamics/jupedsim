# make some new ini-files based on the file <ini-Bottleneck.xml>
# so first make changes in <ini-Bottleneck.xml> and then spread them with this script to the  world         

import re
from os import path

HOME = path.expanduser("~")
TRUNK =  HOME + "/Workspace/peddynamics/JuPedSim/JPSgcfm/trunk/"
GEODIR = TRUNK + "inputfiles/Bottleneck/"
TRAJDIR = TRUNK + "outputfiles/"
inifile =  TRUNK + "ini-Bottleneck.xml"

f = open(inifile)
read_data = f.read()
f.close()
W = [0.8, 0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0]
for w in W:
	ofile = TRUNK + str(w) + "_ini-Bottleneck.xml"
	geofile = GEODIR + str(w) + "_" + "bottleneck.xml" 
	trajfile = TRAJDIR + str(w) + "_" + "TrajBottleneck.xml"
	
	print "inifile ---> ", ofile
	print "geofile ---> ", geofile
	print "trajfile ---> ", trajfile	
	print"-----------------------------"
	g = open(ofile, "w")

	newLine = "<geometry>%s</geometry>"%geofile
	write_data = re.sub(r'<geometry>(.*)</geometry>', newLine, read_data)
	newLine = r'<trajectories format="xml-plain" version="0.4">%s</trajectories>'%trajfile
	write_data = re.sub(r'<trajectories (.*)</trajectories>', newLine, write_data)
	g.write(write_data) 
	g.close() 
