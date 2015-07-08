# coding:utf-8
"""
Class for benchmarking of jpscore
See template_test/How_to_make_new_test.txt
"""

import fnmatch
import glob
import logging
from os import path
import os
from stat import S_ISREG, ST_MODE, ST_MTIME
import subprocess
import sys


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

    def __init__(self, testnumber, argv0, testdir, utestdir=".."):
        self.SUCCESS = 0
        self.FAILURE = 1
        # check if testnumber is digit
        assert isinstance(testnumber, float) or isinstance(testnumber, int), "argument <testnumber> is not digit"
        # only allow path and strings as path directory name
        assert isinstance(argv0, str), "argument <testdir> is not string"
        assert path.exists(testdir), "%s does not exist"%testdir
        assert path.exists(utestdir), "%s does not exist"%utestdir
        assert isinstance(argv0, str), "argument <argv0> is not string"
        assert path.exists(argv0), "%s is does not exist"%argv0
        self.testno = testnumber
        self.logfile = "log_test_%d.txt" % self.testno
        self.logfile = os.path.join(testdir, self.logfile)

        # touch file if not already there
        open(self.logfile, 'a').close()
        logging.basicConfig(filename=self.logfile, level=logging.DEBUG,
                            format='%(asctime)s - %(levelname)s - %(message)s')
        self.HOME = path.expanduser("~")

        self.DIR = testdir
        self.UTEST = utestdir
        self.CWD = os.getcwd()
        self.FILE = os.path.join(self.DIR, "master_ini.xml")

    def run_test(self, testfunction, *args):
        assert hasattr(testfunction, '__call__'), "run_test: testfunction has no __call__ function"
        self.__configure()
        executable = self.__find_executable()
        results = []
        for inifile in self.inifiles:
            res = self.__execute_test(executable, inifile, testfunction, *args)
            results.append(res)
        return results

    def __configure(self):
        if self.CWD != self.DIR:
            logging.info("working dir is %s. Change to %s", os.getcwd(), self.DIR)
            os.chdir(self.DIR)
        logging.info("change directory to utest=%s", self.UTEST)
        os.chdir(self.UTEST)
        # -------- get directory of the code TRUNK
        # *** Note: assume that UTEST is always a direct subdirectory of TRUNK ***
        self.trunk = os.path.dirname(os.getcwd())
        logging.info("call makeini.py with -f %s", self.FILE)
        subprocess.call(["python", "makeini.py", "-f", "%s" % self.FILE])
        # os.chdir(self.DIR)
        logging.info("change directory back to %s", self.DIR)
        os.chdir(self.DIR)
        if self.UTEST == "..":
            lib_path = os.path.abspath(os.path.join(self.trunk, "Utest"))
        else:
            lib_path = os.path.abspath(self.UTEST)

        sys.path.append(lib_path)
        # initialise the inputfiles for jpscore
        self.geofile = os.path.join(self.DIR, "geometry.xml")
        self.inifiles = glob.glob(os.path.join("inifiles", "*.xml"))
        if not path.exists(self.geofile):
            logging.critical("geofile <%s> does not exist", self.geofile)
            exit(self.FAILURE)
        for inifile in self.inifiles:
            if not path.exists(inifile):
                logging.critical("inifile <%s> does not exist", inifile)
                exit(self.FAILURE)
        return

    def __find_executable(self):
        executable = os.path.join(self.trunk, "bin", "jpscore")

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

    def __execute_test(self, executable, inifile, testfunction, *args):
        cmd = "%s --inifile=%s"%(executable, inifile)
        logging.info('start simulating with exe=<%s>', cmd)
        subprocess.call([executable, "--inifile=%s" % inifile])
        logging.info('end simulation ...\n--------------\n')
        trajfile = os.path.join("trajectories", "traj" + inifile.split("ini")[2])
        logging.info('trajfile = <%s>', trajfile)
        if not path.exists(trajfile):
            logging.critical("trajfile <%s> does not exist", trajfile)
            exit(self.FAILURE)
        res = testfunction(inifile, trajfile, *args)
        return res







