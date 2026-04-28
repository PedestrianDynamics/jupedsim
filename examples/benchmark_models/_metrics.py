"""Shared trajectory analysis helpers for benchmark scripts.

These are used both by ``benchmark_models.py`` and by the standalone
``fluctuation.py`` and ``order_parameter.py`` analysis scripts. They are
not part of the public JuPedSim API.
"""

from pathlib import Path

import numpy as np
import pandas as pd
from pedpy import (
    SpeedCalculation,
    compute_individual_speed,
    load_trajectory_from_jupedsim_sqlite,
)


def _individual_velocity(trajectory_data) -> pd.DataFrame:
    """Compute per-agent vector velocities and merge them into the trajectory."""
    individual_velocity = compute_individual_speed(
        traj_data=trajectory_data,
        frame_step=5,
        compute_velocity=True,
        speed_calculation=SpeedCalculation.BORDER_ADAPTIVE,
    )
    return trajectory_data.data.merge(individual_velocity, on=["id", "frame"])


def compute_fluctuation(sqlite_file):
    """Velocity fluctuation time series ``||v_i - v_{i-1}|| / dt``.

    Returns
    -------
    time_s : pandas.Series
        Frame timestamps in seconds.
    fluctuation : pandas.Series
        Instantaneous fluctuation magnitude in m/s^2.
    """
    traj = load_trajectory_from_jupedsim_sqlite(Path(sqlite_file))
    fps = traj.frame_rate
    dt = 1.0 / fps

    df = _individual_velocity(traj)
    v_avg = df.groupby("frame")[["v_x", "v_y"]].mean()
    diff = v_avg - v_avg.shift(1)
    fluctuation = np.sqrt(diff["v_x"] ** 2 + diff["v_y"] ** 2) / dt
    time_s = fluctuation.index / fps
    return time_s, fluctuation


def compute_order_parameter(sqlite_file):
    """Alignment (order) parameter time series.

    For each agent, the goal is taken as its final position. The order
    parameter at frame ``t`` is the average over all agents of
    ``v_n . u_n / |v_n|``, where ``u_n`` is the unit vector toward the
    goal.

    Returns
    -------
    time_s : numpy.ndarray
        Frame timestamps in seconds.
    alignment : numpy.ndarray
        Alignment parameter per frame.
    """
    traj = load_trajectory_from_jupedsim_sqlite(Path(sqlite_file))
    fps = traj.frame_rate
    df = _individual_velocity(traj)

    goals = (
        df.groupby("id")[["x", "y"]]
        .last()
        .rename(columns={"x": "g_x", "y": "g_y"})
    )
    df = df.merge(goals, on="id")

    dx = df["g_x"] - df["x"]
    dy = df["g_y"] - df["y"]
    dist = np.sqrt(dx**2 + dy**2)
    u_x = dx / dist
    u_y = dy / dist
    speed = np.sqrt(df["v_x"] ** 2 + df["v_y"] ** 2)
    df["alignment_n"] = np.where(
        speed > 0,
        (df["v_x"] * u_x + df["v_y"] * u_y) / speed,
        0.0,
    )
    alignment = df.groupby("frame")["alignment_n"].mean()
    time_s = alignment.index.to_numpy() / fps
    return time_s, alignment.to_numpy()
