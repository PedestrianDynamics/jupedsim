import pathlib
import re

import pandas as pd


def read_traj_file(traj_file: pathlib.Path):
    """
    Reads in data from trajectory file.

    :param traj_file (Pathlib.path): trajectory file to read in
    :return fps (float), N (int), data (numpy array): frames per second, number of agents, trajectory data

    """
    with open(traj_file, "r") as file:
        for line in file:
            if "#agents" in line:
                # get number of agents
                N = int(re.search(r"\d+", line).group())
            if "#framerate" in line:
                # get fps
                fps = float(re.search(r"\d+.\d+", line).group())
            if "#ID\tFR" in line:
                # get column names
                header = line.rstrip().strip("#").split("\t")
                header.append("dummy after last tab")
                break

    data = pd.read_table(
        traj_file,
        comment="#",
        skip_blank_lines=True,
        names=header,
        usecols=["ID", "FR", "X", "Y", "Z"],
    ).to_numpy()

    return fps, N, data
