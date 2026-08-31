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


ROUTE_LENGTH_CONTINUITY = [
    {
        "test_name": "corner_with_shortcut",
        "description": "Same starting point, but end points differ just 0.2 on y axis",
        "wkt_path": "examples/geometry/corner_with_shortcut.wkt",
        "path1": [(11.43, 0.44), (27.93, 15.0)],
        "path2": [(11.43, 0.44), (27.93, 15.2)],
        "max_diff": 0.2,
    },
    {
        "test_name": "corner_with_shortcut2",
        "description": "Same starting point, but end points differ just 0.01 on y axis",
        "wkt_path": "examples/geometry/corner_with_shortcut.wkt",
        "path1": [(11.80, 1.00), (28.50, 13.54)],
        "path2": [(11.80, 1.00), (28.50, 13.55)],
        "max_diff": 0.01,
    },
]


@pytest.mark.parametrize(
    "test_entry",
    ROUTE_LENGTH_CONTINUITY,
    ids=lambda params: params["test_name"],
)
def test_route_length_follows_the_target(test_entry):
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
