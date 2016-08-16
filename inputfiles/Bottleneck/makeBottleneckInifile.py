#!/usr/bin/env python3
# make some new ini-files based on the file <ini-Bottleneck.xml>
# so first make changes in <ini-Bottleneck.xml> and then spread them with this script to the  world         

import re
from os import path
#os.getcwd()
GCFM = 0 # Determine which ini-file should be duplicated
if GCFM: 
    basename = "ini_bottleneck.xml"
else:
    basename = "ini_bottleneck_Gompertz.xml"

HOME = path.expanduser("~")
TRUNK =  HOME + "/Workspace/peddynamics/JuPedSim/jpscore/"
GEODIR = TRUNK + "inputfiles/Bottleneck/"
TRAJDIR = GEODIR#TRUNK + "outputfiles/"
inifile =  GEODIR  + basename

f = open(inifile)
read_data = f.read()
f.close()
W = [0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.5]
for w in W:
    ofile = GEODIR + str(w) + "_" + basename
    #geofile = GEODIR + str(w) + "_" + "bottleneck.xml" 
    geofile = str(w) + "_" +  basename
    #trajfile = TRAJDIR + str(w) + "_" + "TrajBottleneck.xml"
    trajfile = str(w) + "_" + "TrajBottleneck.xml"
    
    print "inifile ---> ", ofile
    print "geofile ---> ", geofile
    print "trajfile ---> ", trajfile    
    print"-----------------------------"
    g = open(ofile, "w")

    newLine = "<geometry>%s</geometry>"%geofile
    write_data = re.sub(r'<geometry>(.*)</geometry>', newLine, read_data)
    #newLine = r'<trajectories format="xml-plain" version="0.4">%s</trajectories>'%trajfile
    newLine = r'<file location="%s"/>'%trajfile
    #write_data = re.sub(r'<trajectories (.*)</trajectories>', newLine, write_data)
    write_data = re.sub(r'<file location="(.*)" />', newLine, write_data)
    #<file location="trajectorien_1.2_bottleneck.xml" />
    g.write(write_data) 
    g.close() 
