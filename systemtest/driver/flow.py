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


def check_max_agents(flow_dict, traffic_constraints):
    agents = {}
    agents_error = 1  # Needed if the door is too wide, as multiple pedestrian can walk through in one time step

    for door_id, tc in traffic_constraints.items():
        agent_count = len(flow_dict[door_id].index)
        if tc.max_agents != -1:
            assert (
                np.abs(agent_count - tc.max_agents) <= agents_error
            ), f"agent count ({agent_count}) is higher then max agents ({tc.max_agents})"

        agents[door_id] = agent_count

    return agents


def read_starting_times(events_file: Path):
    tree = ET.parse(events_file)
    root = tree.getroot()
    starting_times = {}
    for event in root.iter("event"):
        if event.attrib["state"] == "open" or event.attrib["state"] == "reset":
            starting_times.setdefault(int(event.attrib["id"]), []).append(
                float(event.attrib["time"])
            )

    assert starting_times, "Could not read starting times from events file"
    return starting_times


def check_flow(
    data_dict: dict, starting_times_dict: dict, traffic_constraints: dict
):
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
