#!/usr/bin/env python
import os
import os.path
import subprocess

passed_tests = []

def runtest_execute(tests=list(range(1,16)),delete_logs=False):

    for i in tests:
        script_name = "test_{}/runtest_rimea_{}.py".format(i,i)
        log_name = 'test_{}/log_test_{}.txt'.format(i,i)
        print('\nrun test_{}\n'.format(i))
        if os.path.isfile(log_name):
            os.remove(log_name)
        if os.path.isfile(script_name):
            subprocess.call('python3 {}'.format(script_name),shell = True)
            passed_tests.append('Test '+str(i))
        else:
            print('can not find {} in this direc directory'.format(script_name))

    if len(passed_tests) == 0:
        print('\nNo test ran')
    else:
        print('\nThe following tests ran without interruption:\n',passed_tests)

def test():
    print('runtest_execute successfully imported')

runtest_execute()
