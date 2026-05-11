#! /usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import jupedsim as jps


def path_distance(points: list[tuple[float, float]]) -> float:
    """Calculate total Euclidean length of a waypoint list."""
    return sum(
        math.hypot(x2 - x1, y2 - y1)
        for (x1, y1), (x2, y2) in zip(points[:-1], points[1:])
    )


def load_wkt_file(filename: str):
    # Load file relative to repo root
    repo_root = Path(__file__).parents[2]
    wkt_path = repo_root / filename
    return wkt_path.read_text(encoding="utf-8")


EXAMPLES = {
    "corner_with_shortcut": {
        "description": "Same starting point, but end points differ just 0.2 on y axis, but total distance diff is >>0.2",
        "wkt_path": "examples/geometry/corner_with_shortcut.wkt",
        "error_type": "max_diff",
        "path1": [(11.43, 0.44), (27.93, 15.0)],
        "path2": [(11.43, 0.44), (27.93, 15.2)],
        "max_diff": 0.2,
    },
    "aknz_evac": {
        "description": "Direct path possible, but path ways longer",
        "error_type": "direct path possible",
        "wkt_path": "examples/geometry/aknz_evac.wkt",
        "path": [(530.15, 1762.46), (530.15, 1774.66)],
    },
    "BUW": {
        "description": "Direct path possible, but path ways longer",
        "error_type": "direct path possible",
        "wkt_path": "examples/geometry/BUW.wkt",
        "path": [(12.57, 36.49), (22.0, 36.49)],
    },
    "SiB2023_entrance_jupedsim": {
        "description": "Direct path possible, but path ways longer",
        "error_type": "direct path possible",
        "wkt_path": "examples/geometry/SiB2023_entrance_jupedsim.wkt",
        "path": [(-1864.57, -83.91), (-1764.4, -116.18)],
    },
}


def run_max_diff_example(example):
    # Initialize routing engine from wkt file
    geometry = load_wkt_file(example["wkt_path"])
    navi = jps.RoutingEngine(geometry)

    # Calculate shortest paths
    path1 = navi.compute_waypoints(example["path1"][0], example["path1"][1])
    path2 = navi.compute_waypoints(example["path2"][0], example["path2"][1])

    # Calculate distances
    distance1 = path_distance(path1)
    distance2 = path_distance(path2)

    # Show distance compared to max-distance
    distance_diff = math.fabs(distance2 - distance1)
    print(f"Distance 1: {distance1:.2f}, Distance 2: {distance2:.2f}")
    print(
        f"Distannces should be within <={example['max_diff']} from each other, observed {distance_diff:.2f}"
    )


def run_direct_path_possible_example(example):
    # Initialize routing engine from wkt file
    geometry = load_wkt_file(example["wkt_path"])
    navi = jps.RoutingEngine(geometry)

    # Calculate shortest path and distance
    path = navi.compute_waypoints(*example["path"])
    distance = path_distance(path)

    # Show distance compared to direct distance
    direct_distance = path_distance(example["path"])
    distance_diff = math.fabs(distance - direct_distance)
    print(
        f"Path Distance: {distance:.2f}, Direct Distance: {direct_distance:.2f}"
    )
    print(
        f"Expected path distance to be the direct distance, observed distance difference of {distance_diff:.2f}"
    )


def main():
    jps.set_debug_callback(print)
    jps.set_info_callback(print)
    jps.set_warning_callback(print)
    jps.set_error_callback(print)

    bar = "-" * 140
    for name, example in EXAMPLES.items():
        print(bar)
        print(f"Executing {name!r}: {example['description']}")
        print(bar)
        match error_type := example["error_type"]:
            case "max_diff":
                run_max_diff_example(example)
            case "direct path possible":
                run_direct_path_possible_example(example)
            case _:
                raise ValueError(f"Unknown error type {error_type}")
        print()


if __name__ == "__main__":
    main()
