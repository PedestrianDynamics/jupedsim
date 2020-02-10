#!/usr/bin/env python3
#################################################################################
# Check if the number of pedestrians using the exit is the same as the number of
# pedestrians in the simulation. Hence checking if the door statistics are correct
#################################################################################
import os
from sys import argv, path
import logging
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)

from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

import xml.etree.cElementTree as ET


def readFlow(numPeds):
    filename = 'flow_exit_id_0_traj_group-number_{:d}.txt'.format(int(numPeds))
    flowfile = os.path.join('results', filename)
    logging.error('try to read: {}'.format(flowfile))
    flow = pd.read_csv(flowfile,
                       comment='#',
                       sep='\s+',
                       skip_blank_lines=True,
                       header=None,
                       engine='python')
    flow.columns = ['time', 'peds']
    return flow


def readNumPeds(inifile):
    tree = ET.parse(inifile)
    root = tree.getroot()
    for tc in root.iter("agents"):
        for door in tc.iter("group"):
            return float(door.attrib["number"])

    logging.error('Could not read inifile for number of pedestrians: {}.'.format(inifile))
    exit(FAILURE)


def runtest(inifile, trajfile):
    numPeds = readNumPeds(inifile)

    data = readFlow(numPeds)
    passingPeds = data.shape[0]

    if np.abs(passingPeds - numPeds) > 1e-5:
        logging.error('{} peds used the transition, but {} peds are in the simulation.'.format(passingPeds, numPeds))
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
