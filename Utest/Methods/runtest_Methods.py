#!/usr/bin/env python

import os



import logging



logging.basicConfig(
    filename = "Utest_Methods.log",
    level    = logging.DEBUG,
    style    = "{",
    format   = "{levelname:8} \t {message} \t")

logging.info("start")

# cmd = 'jpsreport ini_Methods.xml'

cmd = 'pwd'

logging.info("{0}".format(cmd))

os.system(cmd + '> output.txt')



logging.info("end")

logging.shutdown()