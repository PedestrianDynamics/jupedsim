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
        flow_dict[door_id] = flow

    assert (
        len(flow_dict.keys()) > 0
    ), "Could not find flow file to expected flow"
    return flow_dict


def check_flow(
    data_dict: dict, starting_times_dict: dict, traffic_constraints: dict
):
    """
    This method compares the configured traffic constraints (e.g. max_agents and outflow) against the actual flow and data.
    For this, the number of agents passing the door in a certain timeperiod is computed.
    The computed number is compared with the expected flow and maximum agents.
    This is done using an allowed error since the simulation might exceed the flow as well as the maximum agents.
    It is possible, that more then one agent crosses the door in a single

    :param data_dict:
    :param starting_times_dict:
    :param traffic_constraints:
    """
    flowError = 0.3
    timeError = 0.2  # in seconds, a little time is between opening of door and first ped passing

    for door_id, data in data_dict.items():
        max_agents = traffic_constraints[door_id].max_agents
        starting_times = starting_times_dict[door_id]
        outflow = traffic_constraints[door_id].outflow

        relativeError = flowError * outflow

        for i, startTime in enumerate(starting_times, start=0):
            if i + 1 < len(starting_times):
                endTime = starting_times[i + 1]
                first = data[data.time.between(startTime, endTime)].iloc[0]
                last = data[data.time.between(startTime, endTime)].iloc[-1]
                num_agents = len(data[data.time.between(startTime, endTime)])
            else:
                first = data[startTime < data.time].iloc[0]
                last = data[startTime < data.time].iloc[-1]
                num_agents = len(data[startTime < data.time])

            # Check if number of agents passing through door exceeds max agents for this door
            assert (
                num_agents
                <= max_agents
                + 2  # We add a tolerance of 2 since currently agents could go through the door in one time step
            ), f"Agents passing door ({num_agents}) are more than max agents ({max_agents}) allowed"

            # Check flow through door
            time = last.time - first.time
            flow = num_agents / time
            assert (
                np.abs(flow - outflow) <= relativeError
            ), "Door flow is not as expected"

            # Check starting time
            assert np.abs(first.time - starting_times[i]) < timeError
