# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps


def _path_distance(points: list[tuple[float, float]]) -> float:
    """Calculate total Euclidean length of a waypoint list."""
    return sum(
        math.hypot(x2 - x1, y2 - y1)
        for (x1, y1), (x2, y2) in zip(points[:-1], points[1:])
    )


def test_simple_path():
    geometry = [(0, 0), (0, 10), (10, 10), (10, 0)]
    navi = jps.RoutingEngine(geometry)

    # Check is_routable for a point outside the geometry
    assert not navi.is_routable((-1, -1))

    # Now check a simple path. Ensure it does not cross the triangles so that the path length is 2
    start = (1, 1)
    end = (4, 1)
    path = navi.compute_waypoints(start, end)
    assert all([navi.is_routable(p) for p in (start, end)])
    assert len(path) == 2
    # distance is simple as we only move along the x-axis
    assert math.isclose(_path_distance(path), 3)
