#!/usr/bin/env python3
#################################################################################
# Check if the door is opened only opened after the times specified in events
# file. Also check if the flow fits the outflow specified in the ini file.
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
import re

def readFlow(outflow):
    ioutflow = int(outflow)
    regexString = 'rate_{:.2f}'.format(outflow)
    regex = re.compile(regexString)

    for root, dirs, files in os.walk('results'):
        for file in files:
            if regex.search(file):
                flow = pd.read_csv(os.path.join('results', file),
                                   comment='#',
                                   sep='\s+',
                                   skip_blank_lines=True,
                                   header=None,
                                   engine='python')
                flow.columns = ['time', 'peds']
                return flow

    logging.error('Could not find flow file to expected flow {}.'.format(outflow))
    exit(FAILURE)


def readOutflow(inifile):
    tree = ET.parse(inifile)
    root = tree.getroot()
    for tc in root.iter("traffic_constraints"):
        for door in tc.iter("door"):
            return float(door.attrib["outflow"])

    logging.error('Could not read inifile for outflow: {}.'.format(inifile))
    exit(FAILURE)


def readMaxAgents(inifile):
    tree = ET.parse(inifile)
    root = tree.getroot()
    for tc in root.iter("traffic_constraints"):
        for door in tc.iter("door"):
            return int(door.attrib["max_agents"])

    logging.error('Could not read inifile for max agents: {}.'.format(inifile))
    exit(FAILURE)


def readStartingTimes():
    times = []
    tree = ET.parse('results/events.xml')
    root = tree.getroot()
    for event in root.iter("event"):
        if event.attrib["state"] == "open" or event.attrib["state"] == "reset":
            times.append(float(event.attrib["time"]))

    if times:
        return times
    else:
        logging.error('Could not read starting times from events file: {}.'.format('results/events.xml'))
        exit(FAILURE)


def checkFlow(data, maxAgents, startingTimes, expectedFlow):
    success = True

    flowError = 0.3
    relativeError = flowError * expectedFlow
    flows = []
    for i, startTime in enumerate(startingTimes, start=0):
        if i + 1 < len(startingTimes):
            endTime = startingTimes[i + 1]
            first = data[data.time.between(startTime, endTime)].iloc[0]
            last = data[data.time.between(startTime, endTime)].iloc[-1]
        else:
            first = data[startTime < data.time].iloc[0]
            last = data[startTime < data.time].iloc[-1]

        time = last.time - first.time
        flow = maxAgents / time

        if (np.abs(flow - expectedFlow) > relativeError):
            success = False

        flows.append(flow)

    return success, flows


def checkMaxAgents(data, maxAgents, startingTimes):
    success = True

    agentsError = 2

    agents = []
    for i, startTime in enumerate(startingTimes, start=0):
        if i + 1 < len(startingTimes):
            endTime = startingTimes[i + 1]
            agentCount = data[data.time.between(startTime, endTime)].shape[0]
        else:
            agentCount = data[startTime < data.time].shape[0]

        if (np.abs(agentCount - maxAgents) > agentsError):
            success = False

        agents.append(agentCount)

    return success, agents

def checkStart(data, maxAgents, startingTimes):
    success = True

    timeError = 0.2  # in seconds, a little time is between opening of door and first ped passing
    times = []
    i = 0

    gaps = enumerate([data.time[i + 1] - data.time[i] for i in range(len(data.time) - 1)], start=0)
    maxGaps = sorted(gaps, reverse=True, key=lambda x: x[1])[:len(startingTimes) - 1]

    for gap in maxGaps:
        splitIndex = gap[0]
        times.append(data.time.iloc[:splitIndex][0])

    splitIndex = maxGaps[-1][0]
    times.append(data.time.iloc[splitIndex + 1])

    if len(times) != len(startingTimes):
        logging.error("Lengths of starting time does not match defined number of events")
        return False, times

    for measured, expected in zip(times, startingTimes):
        if np.abs(measured - expected) > timeError:
            success = False

    return success, times

def runtest(inifile, trajfile):
    success = True

    maxAgents = readMaxAgents(inifile)
    outflow = readOutflow(inifile)
    startingTimes = readStartingTimes()
    data = readFlow(outflow)

    [flowCorrect, measuredFlow] = checkFlow(data, maxAgents, startingTimes, outflow)
    if not flowCorrect:
        success = False
        logging.error('Outflow rate is not as expected. Expected {}, but are {}.'.format(outflow, measuredFlow))

    [startCorrect, measuredStart] = checkStart(data, maxAgents, startingTimes)
    if not startCorrect:
        success = False
        logging.error(
            'Wrong times for opening the doors. Should be {}, but are {}.'.format(startingTimes, measuredStart))

    [maxAgentsCorrect, measuredAgents] = checkMaxAgents(data, maxAgents, startingTimes)
    if not maxAgentsCorrect:
        success = False
        logging.error(
            'Wrong number of pedestrians passing the door. Should be max {}, but are {}.'.format(maxAgents,
                                                                                                 measuredAgents))

    if not success:
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
