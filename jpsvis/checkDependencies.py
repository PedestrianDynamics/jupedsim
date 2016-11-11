# some vtk libs depend on python.
# correct this dependency and make it point to local python.
import glob2
import subprocess
import shlex
import os 
RED = '\033[0;31m'
NC = '\033[0m' # No Color


CMD = "jpsvis"
PATH = "./bin/%s.app/Contents/Frameworks"  % CMD

dependencies = glob2.glob("%s/**/*.dylib"%PATH)





#VTK_LIBS
for vtklib in dependencies:
    
    print "vtklib: <%s>" % vtklib

    cmd = "otool -L %s" % vtklib
    f = open("blah.txt", "w")
    res = subprocess.call(shlex.split(cmd), stdout=f)
    f.close()
    f = open("blah.txt", "r")
    lines = f.readlines()
    for line in lines[1:]:
        line = line.strip()
        if not line.startswith("/usr/lib") \
           and not line.startswith("/System") \
           and not line.startswith("@executable_path"):

            print ">> <%s>" % line.split()[0]
            change = "install_name_tool -change %s @executable_path/../Frameworks/Python.framework/Versions/3.5/Python %s"%(line.split()[0], vtklib)
            print "<%s>" % change
            res = subprocess.call(shlex.split(change))

            
    f.close()

if os.path.exists ("blah.txt"):
    os.remove("blah.txt")    
