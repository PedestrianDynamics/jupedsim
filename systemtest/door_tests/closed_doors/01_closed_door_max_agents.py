#!/usr/bin/env python3
######################################################################################
# Check if the door closed after max_agents is reached and peds use the other door.  #
######################################################################################
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


def read_flow():
    regexString = 'flow_exit_id_\d+_traj.txt'
    regex = re.compile(regexString)

    flow_dict = {}

    for root, dirs, files in os.walk('results'):
        for file in files:
            if regex.search(file):
                door_id = int(re.search(r'\d+', file).group())
                flow = pd.read_csv(os.path.join('results', file),
                                   comment='#',
                                   sep='\s+',
                                   skip_blank_lines=True,
                                   header=None,
                                   engine='python')
                flow.columns = ['time', 'peds']
                flow_dict[door_id] = flow.shape[0]

    if len(flow_dict.keys()) > 0:
        return flow_dict

    logging.error('Could not find flow file to expected flow')
    exit(FAILURE)


def read_max_agents(inifile):
    tree = ET.parse(inifile)
    root = tree.getroot()
    max_agents_dict = {}
    for tc in root.iter("traffic_constraints"):
        for door in tc.iter("door"):
            id = int(door.attrib["trans_id"])
            if 'max_agents' in door.attrib:
                max_agents = int(door.attrib["max_agents"])
                max_agents_dict[id] = max_agents

    if len(max_agents_dict.keys()) > 0:
        return max_agents_dict

    logging.error('Could not read inifile for max agents: {}.'.format(inifile))
    exit(FAILURE)


def read_num_agents(inifile):
    tree = ET.parse(inifile)
    root = tree.getroot()

    for group in root.iter("group"):
        number = int(group.attrib["number"])
        return number

    logging.error('Could not read inifile for number of agents: {}.'.format(inifile))
    exit(FAILURE)


def check_max_agents(flow_dict, max_agents_dict):
    success = True

    agents = {}
    agents_error = 1  # Needed if the door is too wide, as multiple pedestrian can walk through in one time step

    for door_id, max_agents in max_agents_dict.items():
        agent_count = flow_dict[door_id]
        if np.abs(agent_count - max_agents) > agents_error:
            success = False

        agents[door_id] = agent_count

    return success, agents


def check_door_usage(flow_dict, num_agents):
    success = True

    # All pedestrian should leave through the main exit
    exit_usage = max(flow_dict.values())  # maximum value
    if exit_usage != num_agents:
        success = False
        logging.error(
            'Number of agents exiting the simulation ({}) does not match number of agents in the simulation ({})'
                .format(exit_usage, num_agents))

    # All pedestirans should pass one of the other doors
    other_door_usage = sum(flow_dict.values()) - exit_usage
    if other_door_usage != num_agents:
        success = False
        logging.error(
            'Number of agents using other doors ({}) does not match number of agents in the simulation ({})'
                .format(other_door_usage, num_agents))

    return success


def runtest(inifile, trajfile):
    success = True

    max_agents_dict = read_max_agents(inifile)
    flow_dict = read_flow()
    num_agents = read_num_agents(inifile)

    [max_agents_correct, measured_agents] = check_max_agents(flow_dict, max_agents_dict)
    if not max_agents_correct:
        success = False
        logging.error(
            'Wrong number of pedestrians passing the door. Should be {}, but are {}.'
                .format(max_agents_dict, measured_agents))

    door_usage_correct = check_door_usage(flow_dict, num_agents)
    if not door_usage_correct:
        success = False
        logging.error(
            'Wrong door usage {}.'.format(flow_dict))

    if not success:
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(2, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpscore=argv[1])
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
