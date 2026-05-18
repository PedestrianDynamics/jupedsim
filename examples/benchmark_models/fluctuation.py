"""Calculate and plot the velocity fluctuation of a trajectory."""

import sys

import matplotlib.pyplot as plt
from _metrics import compute_fluctuation


def main(sqlite_file: str, title: str) -> None:
    time_s, fluctuation = compute_fluctuation(sqlite_file)

    phi_v = fluctuation.dropna().mean()
    print(f"Mean velocity fluctuation (phi_v): {phi_v:.4f} m/s^2")

    plt.figure(figsize=(10, 5))
    plt.plot(time_s, fluctuation.values)
    plt.title(title)
    plt.xlim([50, 200])
    plt.ylim([0, 0.25])
    plt.xlabel("Time (s)")
    plt.ylabel(r"Change Magnitude ($m/s^2$)")
    plt.grid(True, linestyle="--", alpha=0.7)
    plt.savefig(f"{title}.png")
    print(f"{title}.png")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
