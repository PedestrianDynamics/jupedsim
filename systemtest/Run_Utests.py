import os
os.chdir(os.path.realpath(__file__).split('/Run_Utests.py')[0])
import time
import subprocess
import shutil
import xml.etree.ElementTree as ET
from termcolor import colored


class Utest :
    def __init__(self,testname,testcase,runtime,id):
        self.testname = testname
        self.startdir = os.getcwd()
        self.testnumber = id
        self.testfolder = "{}/{}".format(self.startdir,testname)
        self.testcase = testcase
        self.expc_runtime = runtime
        self.real_runtime = "Test didn't run!"

    def show_info(self,cls='all'):
        if cls == 'all' or cls == self.testcase:
            print("\ntestclass\t\t=\t{}".format(self.testcase))
            print("testnumber\t\t=\t{}".format(self.testnumber))
            print("expected runtime\t=\t{} seconds".format(self.runtime))
            if self.check_testfiles()[0]:
                print("All necessary files are found")
            else:
                print (colored(self.check_testfiles()[1], 'red'))

    def run_test (self,cls='all'):
        if cls == 'all' or cls == self.testcase:
            if self.check_testfiles()[0]:
                self.make_inifiles()
                os.chdir(self.testfolder)
                start_time = time.time()
                for ini in self.inilist:
                    subprocess.call(["{}/{}".format(self.startdir.split('/Utest')[0],jpscore),"{}/inifiles/{}".format(self.testfolder,ini)])
                self.real_runtime = time.time() - start_time
                os.chdir(self.startdir)
                self.move_logfile()

    def check_testfiles(self):
        if os.path.isdir(self.testfolder):
            data_complete = True
            message = None
            os.chdir(self.testfolder)
            if not os.path.isfile('master_ini.xml') or not os.path.isfile('geometry.xml'):
                data_complete = False
                message = "master_ini or geometry is missing in the directory"
        else:
            data_complete = False
            message = "The Folder {} is missing".format(self.testfolder)

        os.chdir(self.startdir)
        return(data_complete,message)

    def make_inifiles(self):
        subprocess.call(["python", "makeini.py", "-f", "{}/master_ini.xml".format(self.testfolder)])
        self.inilist = []
        for ini in os.listdir("{}/inifiles".format(self.testfolder)):
            self.inilist.append(ini)

    def check_logfile(self):
        if os.path.isfile("{}/log_test_{}.txt".format(self.testfolder,self.testnumber)):
            self.logfile = "{}/log_test_{}.txt".format(self.testfolder,self.testnumber)
            return(True)
        elif os.path.isfile("{}/inifiles/log".format(self.testfolder)):
            self.logfile = "{}/inifiles/log".format(self.testfolder)
            return(True)
        elif os.path.isfile("{}/inifiles/log.txt".format(self.testfolder)):
            self.logfile = "{}/inifiles/log.txt".format(self.testfolder)
            return(True)
        else:
            return(False)

    def move_logfile(self):
        log_dir= '{}/Utest_logs'.format(os.getcwd())
        if not os.path.isdir(log_dir):
            os.mkdir(log_dir)
        if self.check_logfile():
            shutil.move(self.logfile,'{}/{}'.format(log_dir,self.logfile.split("/")[-1]))
            os.rename('{}/{}'.format(log_dir,self.logfile.split("/")[-1]),"{}/{}_{}.txt".format(log_dir,self.testcase,self.testnumber))

    def compare_runtime(self):
        print("\n{}\nexpected runtime = {} [sec]\treal runtime = {:.3f} [sec]".format(self.testname,self.expc_runtime,self.real_runtime))
        if self.real_runtime >= 5:
            if not 0.7 < self.real_runtime/self.expc_runtime < 1.3:
                print (colored('The expected runtime does not fix with the real runtime, please check if the Test in run properly!', 'red'))
        if self.real_runtime <= 0.01:
            print (colored('It seems like the test does not run!', 'red'))



testlist = []
testcases = "all"

os.chdir(os.path.realpath(__file__).split('/Run_Utests.py')[0])
tree = ET.parse('Utests_ini.xml')
root = tree.getroot()

# get config
jpscore = root[0][0].get('path')
short = int(root[0][1].get('short'))
middle = int(root[0][1].get('middle'))

# get testdata
for test_case in root[1]:
    id = 1
    testcases = "{}, {}".format(testcases,test_case.tag)
    for test in test_case:
        obj = Utest("{}/{}".format(test_case.get('folder'),test.tag),test_case.tag,int(test.get('runtime')),id)
        testlist.append(obj)
        id += 1


def Utests(testcases):
    inp_message = ("\ncommand test_class runtime\n\ncommands\t:\tinfo, run, quit,\ntest_classes\t:\t" + testcases)
    inp_message = ("{}\nruntime \t:\tshort, middle, long\n\n".format(inp_message))
    command = input(inp_message)
    command = command.split(' ')

    if command[0] == 'info':
        if len(command) >= 2:
            for test in testlist:
                test.show_info(command[1])

    elif command[0] == 'run':
        ran_tests = []
        if len(command) == 2:
            for test in testlist:
                test.run_test(command[1])
                ran_tests.append(test)
        elif len(command) == 3:
            if command[2] == 'short':
                for test in testlist:
                    if test.expc_runtime < short:
                        test.run_test(command[1])
            elif command[2] == 'mid':
                for test in testlist:
                    if test.expc_runtime < middle:
                        test.run_test(command[1])
            else:
                for test in testlist:
                    test.run_test(command[1])

        for test in testlist:
            if type(test.real_runtime) == float:
                    test.compare_runtime()


    else:
        print("The command don't fit with run or info!")


if __name__ == "__main__":
    Utests(testcases)
