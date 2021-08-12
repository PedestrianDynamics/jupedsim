# some vtk libs depend on python.
# correct this dependency and make it point to local python.
import glob
import subprocess
import shlex
import os
import sys
RED = '\033[0;31m'
NC = '\033[0m' # No Color

if len(sys.argv) == 1:
    sys.exit("usage: %s path_to_.app_file" % sys.argv[0])

PATH = sys.argv[1] + "/Contents/Frameworks"

CMD = "jpsvis"
# PATH = "./bin/%s.app/Contents/Frameworks"  % CMD
# PATH = "/Volumes/jpsvis\ 0.7.0/jpsvis.app/Contents/Frameworks/"
dependencies = glob.glob("%s/**/*.dylib"%PATH)


#VTK_LIBS
for vtklib in dependencies:
    
    print ("lib: <%s>" % vtklib)

    cmd = "otool -L %s" % vtklib
    f = open("blah.txt", "w")
    res = subprocess.call(shlex.split(cmd), stdout=f)
    f.close()
    f = open("blah.txt", "r")
    lines = f.readlines()
    f.close()

    for line in lines[1:]:
        line = line.strip()
        if not line.startswith("/usr/lib") \
           and not line.startswith("/System") \
           and not line.startswith("@executable_path"):

            print("%s >> <%s> %s" % (RED, line.split()[0], NC))
#           change = "install_name_tool -change %s @executable_path/../Frameworks/Python.framework/Versions/3.5/Python %s"%(line.split()[0], vtklib)
 #           print "<%s>" % change
 #          res = subprocess.call(shlex.split(change))
 #          raw_input("pause ...")
            
 
if os.path.exists ("blah.txt"):
    os.remove("blah.txt")    
