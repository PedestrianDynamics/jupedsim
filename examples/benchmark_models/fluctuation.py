"""Calculate fluctuations."""

import numpy as np
import matplotlib.pyplot as plt
from pedpy import load_trajectory_from_jupedsim_sqlite, compute_individual_speed
from pedpy import SpeedCalculation

import sys


from pathlib import Path

filename = Path(sys.argv[1])
title = sys.argv[2]

# 1. Load the trajectory data
trajectory_data = load_trajectory_from_jupedsim_sqlite(filename)
df = trajectory_data.data
fps = trajectory_data.frame_rate
dt = 1 / fps  # Time difference between frames

# 2. Compute Individual Velocities
# We need individual velocities first to find the average crowd velocity per frame
individual_velocity = compute_individual_speed(
    traj_data=trajectory_data,
    frame_step=5,
    compute_velocity=True,
    speed_calculation=SpeedCalculation.BORDER_ADAPTIVE,
)

df = df.merge(individual_velocity, on=["id", "frame"])

# 3. Calculate Average Vector Velocity per frame (v_i)
# This represents the "average vector velocity inside the considered area"
v_i = df.groupby("frame")[["v_x", "v_y"]].mean()

# 4. Calculate the Fluctuations: || v_i - v_{i-1} ||
# Shift the dataframe to get v_{i-1}
v_prev = v_i.shift(1)

# Calculate the Euclidean norm of the difference between consecutive frames
diff_x = v_i["v_x"] - v_prev["v_x"]
diff_y = v_i["v_y"] - v_prev["v_y"]
fluctuation_magnitudes = np.sqrt(diff_x**2 + diff_y**2)

# 5. Calculate Velocity Fluctuation (phi_v)
# The formula: (1 / (I * dt)) * sum(||v_i - v_{i-1}||)
total_sum = fluctuation_magnitudes.sum()
I = len(v_i)
phi_v = (1 / (I * dt)) * total_sum

print(f"Total Velocity Fluctuation (phi_v): {phi_v:.4f} m/s^2")

# 6. Optional: Plot the instantaneous fluctuation over time
plt.figure(figsize=(10, 5))
plt.plot(fluctuation_magnitudes.index / fps, fluctuation_magnitudes.values / dt)
plt.title(title)
plt.xlim([50, 200])
plt.ylim([0, 0.25])
plt.xlabel("Time (s)")
plt.ylabel("Change Magnitude ($m/s^2$)")
plt.grid(True, linestyle="--", alpha=0.7)
plt.savefig(f"{title}.png")
print(f"{title}.png")
