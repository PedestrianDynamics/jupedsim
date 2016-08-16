#!/usr/bin/env python3
import re

UP = 103
DOWN = 101

infile = "bottleneck.xml"

f = open(infile)
read_data = f.read()
f.close()


W = [0.8, 0.9, 1.0, 1.1, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.5, 3.0, 4.0, 5.0]

for w in W:
	ofile = str(w) + "_" +infile
	print "---> ", ofile
	g = open(ofile, "w")
	shift = 0.5*(UP - DOWN - w)
	newDOWN = r'py="%.3f"'%(DOWN + shift)
	newUP = r'py="%.3f"'%(UP - shift)
        print "UP=%f, newUP=%s, Down=%f newDown=%s"%(UP, newUP, DOWN, newDOWN)
	write_data = re.sub(r'py="%.6f"'%UP, newUP, read_data)
	write_data = re.sub(r'py="%.6f"'%DOWN, newDOWN, write_data)
	g.write(write_data)
	g.close()

    
