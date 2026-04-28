"""Calculate and plot the alignment (order) parameter of a trajectory."""

import sys

import matplotlib.pyplot as plt
from _metrics import compute_order_parameter


def main(sqlite_file: str, title: str) -> None:
    time_s, alignment = compute_order_parameter(sqlite_file)

    plt.figure(figsize=(10, 6))
    plt.plot(time_s, alignment, label=r"Alignment Parameter $\phi_d$")
    plt.axhline(
        y=1, color="r", linestyle="--", alpha=0.5, label="Perfect Alignment"
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Alignment Parameter")
    plt.ylim([0, 1.1])
    plt.title(title)
    plt.legend()
    plt.grid(True)

    plt.savefig(f"{title}.png")
    print(f"{title}.png")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
