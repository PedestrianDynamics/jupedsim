# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import jupedsim as jps
import pytest

####################
# Utility functions
####################


def path_distance(points: list[tuple[float, float]]) -> float:
    """Calculate total Euclidean length of a waypoint list."""
    return sum(
        math.hypot(x2 - x1, y2 - y1)
        for (x1, y1), (x2, y2) in zip(points[:-1], points[1:])
    )


def load_wkt_file(filename: str):
    # Load file relative to repo root
    repo_root = Path(__file__).parents[1]
    wkt_path = repo_root / filename
    return wkt_path.read_text(encoding="utf-8")


########################
# End Utility functions
########################


def test_routing_engine_with_excluded_areas():
    """Verify excluded_areas kwarg is forwarded to build_geometry."""
    outer = [(0, 0), (100, 0), (100, 100), (0, 100)]
    hole = [(40, 40), (60, 40), (60, 60), (40, 60)]

    engine = jps.RoutingEngine(
        geometry=outer,
        excluded_areas=[hole],
    )
    assert engine is not None


def test_routing_engine_without_excluded_areas():
    outer = [(0, 0), (100, 0), (100, 100), (0, 100)]
    engine = jps.RoutingEngine(geometry=outer)
    assert engine is not None


BAD_ASTAR_ROUTINGS = [
    {
        "test_name": "corner_with_shortcut",
        "description": "Same starting point, but end points differ just 0.2 on y axis, but total distance diff was >>0.2",
        "wkt_path": "examples/geometry/corner_with_shortcut.wkt",
        "error_type": "max_diff",
        "path1": [(11.43, 0.44), (27.93, 15.0)],
        "path2": [(11.43, 0.44), (27.93, 15.2)],
        "max_diff": 0.2,
    },
    {
        "test_name": "aknz_evac",
        "description": "Direct path possible, but path was ways longer",
        "error_type": "direct path possible",
        "wkt_path": "examples/geometry/aknz_evac.wkt",
        "path": [(530.15, 1762.46), (530.15, 1774.66)],
    },
    {
        "test_name": "BUW",
        "description": "Direct path possible, but path was ways longer",
        "error_type": "direct path possible",
        "wkt_path": "examples/geometry/BUW.wkt",
        "path": [(12.57, 36.49), (22.0, 36.49)],
    },
    {
        "test_name": "SiB2023_entrance_jupedsim",
        "description": "Direct path possible, but path was ways longer",
        "error_type": "direct path possible",
        "wkt_path": "examples/geometry/SiB2023_entrance_jupedsim.wkt",
        "path": [(-1864.57, -83.91), (-1764.4, -116.18)],
        # Still needs some tolerance here, but it was ways worse beforehand
        "abs_diff_tolerance": 0.25,
    },
]


@pytest.mark.parametrize(
    "test_entry",
    [
        test_entry
        for test_entry in BAD_ASTAR_ROUTINGS
        if test_entry["error_type"] == "max_diff"
    ],
    ids=lambda params: params["test_name"],
)
def test_max_diff_example(test_entry):
    geometry = load_wkt_file(test_entry["wkt_path"])
    navi = jps.RoutingEngine(geometry)

    path1 = navi.compute_waypoints(
        test_entry["path1"][0], test_entry["path1"][1]
    )
    path2 = navi.compute_waypoints(
        test_entry["path2"][0], test_entry["path2"][1]
    )

    distance1 = path_distance(path1)
    distance2 = path_distance(path2)

    distance_diff = math.fabs(distance2 - distance1)
    assert distance_diff <= test_entry["max_diff"]


@pytest.mark.parametrize(
    "test_entry",
    [
        test_entry
        for test_entry in BAD_ASTAR_ROUTINGS
        if test_entry["error_type"] == "direct path possible"
    ],
    ids=lambda params: params["test_name"],
)
def test_direct_path_possible_example(test_entry):
    geometry = load_wkt_file(test_entry["wkt_path"])
    navi = jps.RoutingEngine(geometry)

    path = navi.compute_waypoints(*test_entry["path"])
    print(f"----------- [RL, DEBUG] Path length: {len(path)}: {path}")
    distance = path_distance(path)

    direct_distance = path_distance(test_entry["path"])
    abs_tolerance = test_entry.get("abs_diff_tolerance", 1e-12)
    rel_tolerance = test_entry.get("rel_diff_tolerance", 1e-6)
    assert distance == pytest.approx(
        direct_distance, abs=abs_tolerance, rel=rel_tolerance
    )
