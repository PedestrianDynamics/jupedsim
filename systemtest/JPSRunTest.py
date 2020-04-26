# coding:utf-8
"""
Class for benchmarking of jpscore
See template_test/How_to_make_new_test.txt
"""

import fnmatch
import glob
import logging
import numpy as np
import os
import subprocess
import sys
from os import path
from stat import S_ISREG, ST_MODE, ST_MTIME
import makeini
import shutil
__author__ = 'Oliver Schmidts'
__email__ = 'dev@jupedsim.org'
__credits__ = ['Oliver Schmidts', 'Mohcine Chraibi']
__copyright__ = '<2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.'
__license__ = 'GNU Lesser General Public License'
__version__ = '0.1'
__status__ = 'Production'

def getScriptPath():
    return os.path.dirname(os.path.realpath(sys.argv[0]))

class JPSRunTestDriver(object):

    def __init__(self, testnumber, argv0, testdir, utestdir="..", jpsreport="", jpscore=""):
        self.SUCCESS = 0
        self.FAILURE = 1
        # check if testnumber is digit
        assert isinstance(testnumber, float) or isinstance(testnumber, int), "argument <testnumber> is not digit"
        # only allow path and strings as path directory name
        assert isinstance(argv0, str), "argument <testdir> is not string"
        assert path.exists(testdir), "%s does not exist" % testdir
        assert path.exists(utestdir), "%s does not exist" % utestdir
        assert isinstance(argv0, str), "argument <argv0> is not string"
        assert path.exists(argv0), "%s is does not exist" % argv0
        assert jpscore or jpsreport, "no executable given (jpscore or jpsreport)"
        self.testno = testnumber

        # touch file if not already there
        logging.basicConfig(stream=sys.stdout, level=logging.DEBUG,
                            format='%(asctime)s - %(levelname)s - %(message)s')
        self.HOME = path.expanduser("~")
        self.DIR = testdir
        self.jpsreport = jpsreport
        self.jpscore = jpscore
        # Where to find the measured data from the simulations. We will use Voronoi diagrams
        # if self.testno == 101: # fix for 1dfd, since jpsreport can not be used in 1D
        self.simDataDir = os.path.join(self.DIR,
                                       "Output",
                                       "Fundamental_Diagram",
                                       "Classical_Voronoi")
        # Assume that this directory is always data/
        self.expDataDir = os.path.join(self.DIR, "data")
        self.UTEST = utestdir
        self.CWD = os.getcwd()
        self.FILE = os.path.join(self.DIR, "master_ini.xml")

    def run_analysis(self, testfunction, trajfile="", *args):
        assert hasattr(testfunction, '__call__'), "run_test: testfunction has no __call__ function"
        self.__configure()
        assert path.exists(self.jpsreport), "executable {} does not exist".format(self.jpsreport)

        if not path.exists(self.jpsreport_ini):
            logging.critical("jpsreport_ini <{}> does not exist".format(self.jpsreport_ini))
            exit(self.FAILURE)

        res = self.__execute_test(self.jpsreport, self.jpsreport_ini, testfunction, trajfile, *args)
        return res

    def run_test(self, testfunction, fd=0, *args): #fd==1: make fundamental diagram
        assert hasattr(testfunction, '__call__'), "run_test: testfunction has no __call__ function"
        self.__configure()
        assert path.exists(self.jpscore), "executable {} does not exist.".format(self.jpscore)

        results = []
        for inifile in self.inifiles:
            res = self.__execute_test(self.jpscore, inifile, testfunction, *args)
            results.append(res)

        return results

    def __get_FD_data(self, data_dir):
        """
        collect density-velocity data experiments or simulations
        and return an array
        """
        files = glob.glob(os.path.join(data_dir, "*.dat"))
        if len(files) == 0: # maybe we have txt files?
            files = glob.glob(os.path.join(data_dir, "*.txt"))
            if len(files) == 0:
                logging.critical("get_FD_data: no data in %s", data_dir)
                exit(self.FAILURE)

        once = 1
        for f in files:
            if once:
                fd_data = np.loadtxt(f)
                if fd_data.size == 0 :
                    logging.warning("get_FD_data: simulation file %s is empty!", f)
                    continue
                once = 0
            else:
                d = np.loadtxt(f)
                fd_data = np.vstack((fd_data, d))

        return fd_data

    def __configure(self):
        if self.CWD != self.DIR:
            logging.info("working dir is %s. Change to %s", os.getcwd(), self.DIR)
            os.chdir(self.DIR)
        logging.info("change directory to utest=%s", self.UTEST)
        os.chdir(self.UTEST)
        # -------- get directory of the code TRUNK
        # *** Note: assume that UTEST is always a direct subdirectory of TRUNK ***
        self.trunk = os.path.dirname(os.getcwd())
        self.jpsreport_ini = os.path.join(self.DIR, "jpsreport_ini.xml")
        if path.exists(self.FILE):
            makeini.main(self.FILE)
        elif not path.exists(self.jpsreport_ini):
            logging.critical("Did not find master_ini nor jpsreport_ini.")

        # os.chdir(self.DIR)
        logging.info("change directory back to %s", self.DIR)
        os.chdir(self.DIR)
        # remove output directory
        shutil.rmtree('Output', ignore_errors=True)
        if self.UTEST == "..":
            lib_path = os.path.abspath(os.path.join(self.trunk, "systemtest"))
        else:
            lib_path = os.path.abspath(self.UTEST)

        sys.path.append(lib_path)
        # initialise the inputfiles for jpscore
        self.geofile = os.path.join(self.DIR, "geometry.xml") # FIXME: sometimes we have geometries/
        self.inifiles = glob.glob(os.path.join("inifiles", "*.xml"))
        if not path.exists(self.geofile):
            geometries = os.path.join(self.DIR, "geometries/") #maybe we habe a dir with geometries?
            if os.path.exists(geometries) and os.listdir(geometries):
                self.geofile = geometries
            else:
                logging.critical("geofile <%s> does not exist", self.geofile)
                exit(self.FAILURE)

        for inifile in self.inifiles:
            if not path.exists(inifile):
                logging.critical("inifile <%s> does not exist", inifile)
                exit(self.FAILURE)
        return

    def __find_executable(self, executable):
        # executable = os.path.join(self.trunk, "bin", "jpscore")

        # fix for windows
        if not path.exists(executable):
            matches = []
            for root, dirname, filenames in os.walk(os.path.join(self.trunk, 'bin')):
                for filename in fnmatch.filter(filenames, 'jpscore.exe'):
                    matches.append(os.path.join(root, filename))
            if len(matches) == 0:
                logging.critical("executable <%s> or jpscore.exe does not exist yet.", executable)
                exit(self.FAILURE)
            elif len(matches) > 1:
                matches = ((os.stat(file_path), file_path) for file_path in matches)
                matches = ((stat[ST_MTIME], file_path)
                           for stat, file_path in matches if S_ISREG(stat[ST_MODE]))
                matches = sorted(matches)
            executable = matches[0]
        # end fix for windows

        return executable

    def __execute_test(self, executable, inifile, testfunction, trajfile="", *args):
        cmd = "%s %s"%(executable, inifile)
        logging.info('start simulating with exe=<%s>', cmd)
        returncode = subprocess.call([executable, "%s" % inifile])
        if returncode:
            logging.error("Simulation returned with error code: " + str(returncode) + ".")
            exit(self.FAILURE)

        logging.info('end simulation ...\n--------------\n')
        logging.info("inifile <%s>", inifile)
        if not trajfile:
            trajfile = os.path.join("trajectories", "traj" + inifile.split("ini")[2])
            if not path.exists(trajfile):
                trajfile, file_extension = os.path.splitext(trajfile)
                logging.info("trajfile <%s> with ext=<%s> does not exist. Looking for *.txt",
                            trajfile, file_extension)
                trajfile += ".txt"
                if not path.exists(trajfile):
                    logging.critical("trajfile <%s> does not exist", trajfile)
                    exit(self.FAILURE)
                else:
                    logging.info('trajfile = <%s>', trajfile)
            else:
                logging.info('trajfile = <%s>', trajfile)

        res = testfunction(inifile, trajfile, *args)
        return res
