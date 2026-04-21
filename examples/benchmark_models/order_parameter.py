"""Calculate the order parameter."""

import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from pedpy import (
    SpeedCalculation,
    compute_individual_speed,
    load_trajectory_from_jupedsim_sqlite,
)

filename = Path(sys.argv[1])
title = sys.argv[2]


trajectory_data = load_trajectory_from_jupedsim_sqlite(filename)
df = trajectory_data.data

# 2. Compute Velocity using PedPy
# This adds 'v_x' and 'v_y' columns to the dataframe
individual_velocity = compute_individual_speed(
    traj_data=trajectory_data,
    frame_step=5,
    compute_velocity=True,
    speed_calculation=SpeedCalculation.BORDER_ADAPTIVE,
)

df = df.merge(individual_velocity, on=["id", "frame"])

# 3. Define the Goal (Last position of each agent)
goals = (
    df.groupby("id")[["x", "y"]].last().rename(columns={"x": "g_x", "y": "g_y"})
)
df = df.merge(goals, on="id")

# 4. Calculate Alignment Parameter components
# Vector to goal (direction u)
df["dx_g"] = df["g_x"] - df["x"]
df["dy_g"] = df["g_y"] - df["y"]
df["dist_g"] = np.sqrt(df["dx_g"] ** 2 + df["dy_g"] ** 2)

# Unit vector toward goal (u_n)
df["u_x"] = df["dx_g"] / df["dist_g"]
df["u_y"] = df["dy_g"] / df["dist_g"]

# Magnitude of velocity
df["speed"] = np.sqrt(df["v_x"] ** 2 + df["v_y"] ** 2)

# Calculate dot product (v_n \cdot u_n) / |v_n|
# This is equivalent to the projection of the normalized velocity onto the goal vector
# We handle cases where speed is 0 to avoid division by zero
df["alignment_n"] = np.where(
    df["speed"] > 0,
    (df["v_x"] * df["u_x"] + df["v_y"] * df["u_y"]) / df["speed"],
    0,
)

# 5. Average over all participants per frame (N)
alignment_over_time = df.groupby("frame")["alignment_n"].mean()

# 6. Plotting
plt.figure(figsize=(10, 6))
plt.plot(
    alignment_over_time.index,
    alignment_over_time.values,
    label=r"Alignment Parameter $\phi_d$",
)
plt.axhline(
    y=1, color="r", linestyle="--", alpha=0.5, label="Perfect Alignment"
)
plt.xlabel("Frame")
plt.ylabel("Alignment Parameter")
plt.ylim([0, 1.1])
plt.title(title)
plt.legend()
plt.grid(True)

plt.savefig(f"{title}.png")
print(f"{title}.png")
plt.show()
