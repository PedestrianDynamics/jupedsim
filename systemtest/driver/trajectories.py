import pathlib
import re
from dataclasses import dataclass

import pandas as pd
from numpy import count_nonzero, max, ndarray, where


@dataclass()
class Trajectories:
    framerate: float
    count_agents: int
    data: ndarray

    def path(self, id: int) -> ndarray:
        return self.data[where(self.data[:, 0] == id)]

    def runtime(self):
        return max(self.data[:, 1]) / self.framerate

    def frame_count(self):
        return self.data[-1, 1] + 1

    def agent_count_in_frame(self, frame_index: int):
        return count_nonzero(self.data[:, 1] == frame_index - 1)


def load_trajectory(traj_file: pathlib.Path):
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

    return Trajectories(fps, N, data)
