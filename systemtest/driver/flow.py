import logging
import os
import re
import xml.etree.cElementTree as ET
from pathlib import Path
from sys import argv
from driver.trajectories import Trajectories

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def check_flow(
    trajectories: Trajectories,
    starting_times_dict: dict,
    traffic_constraints: dict,
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

    for door_id, tc in traffic_constraints.items():
        max_agents = tc.max_agents
        starting_times = starting_times_dict[door_id]
        outflow = tc.outflow

        relativeError = flowError * outflow

        for i, startTime in enumerate(starting_times, start=0):
            firstFrame = startTime * trajectories.framerate
            lastFrame = (
                trajectories.frame_count()
                if i + 1 >= len(starting_times)
                else starting_times[i + 1] * trajectories.framerate
            )
            num_agents = trajectories.agent_count_in_frame(
                firstFrame
            ) - trajectories.agent_count_in_frame(lastFrame)
            last_agent_exit_frame = lastFrame
            while trajectories.agent_count_in_frame(
                last_agent_exit_frame
            ) == trajectories.agent_count_in_frame(lastFrame):
                last_agent_exit_frame -= 1

            time = last_agent_exit_frame / trajectories.framerate - startTime

            # Check if number of agents passing through door exceeds max agents for this door
            assert (
                num_agents
                <= max_agents
                + 2  # We add a tolerance of 2 since currently agents could go through the door in one time step
            ), f"Agents passing door ({num_agents}) are more than max agents ({max_agents}) allowed"
            # Check flow through door
            flow = num_agents / time
            assert (
                np.abs(flow - outflow) <= relativeError
            ), "Door flow is not as expected"
