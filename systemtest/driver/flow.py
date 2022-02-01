import logging
import os
import re
import xml.etree.cElementTree as ET
from pathlib import Path
from sys import argv

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def read_flow(tmp_path: Path):
    regexString = r"flow_exit_id_\d+_trajectories.txt"
    regex = re.compile(regexString)

    flow_dict = {}
    results_path = tmp_path / "results"
    flow_files = results_path.glob("flow_exit_id_*_trajectories.txt")
    for file in flow_files:
        door_id = int(re.search(r"\d+", file.name).group())
        flow = pd.read_csv(
            file,
            comment="#",
            sep=r"\s+",
            skip_blank_lines=True,
            header=None,
            engine="python",
        )
        flow.columns = ["time", "peds", "pedIDs"]
        flow_dict[door_id] = flow.shape[0]

    assert (
        len(flow_dict.keys()) > 0
    ), "Could not find flow file to expected flow"
    return flow_dict


def read_max_agents(inifile: Path):
    tree = ET.parse(inifile)
    root = tree.getroot()
    max_agents_dict = {}
    for tc in root.iter("traffic_constraints"):
        for door in tc.iter("door"):
            id = int(door.attrib["trans_id"])
            if "max_agents" in door.attrib:
                max_agents = int(door.attrib["max_agents"])
                max_agents_dict[id] = max_agents

    assert (
        len(max_agents_dict.keys()) > 0
    ), "Could not read inifile for max agents"
    return max_agents_dict


def read_num_agents(inifile: Path):
    tree = ET.parse(inifile)
    root = tree.getroot()

    number = 0
    for group in root.iter("group"):
        number += int(group.attrib["number"])

    return number


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
            "Number of agents exiting the simulation ({}) does not match number of agents in the simulation ({})".format(
                exit_usage, num_agents
            )
        )

    # All pedestirans should pass one of the other doors
    other_door_usage = sum(flow_dict.values()) - exit_usage
    if other_door_usage != num_agents:
        success = False
        logging.error(
            "Number of agents using other doors ({}) does not match number of agents in the simulation ({})".format(
                other_door_usage, num_agents
            )
        )

    return success
