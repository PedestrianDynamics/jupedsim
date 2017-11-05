#!/usr/bin/env python

import subprocess

for i in range(5, 16):
    #script_name = "Python test_" + str(i+1) + "/runtest_rimea_" + str(i+1) + ".py"
    script_name = "Python test_%d/runtest_rimea_%d.py" % (i, i)
    print(">> %s" %script_name)
    subprocess.call(script_name, shell=True)