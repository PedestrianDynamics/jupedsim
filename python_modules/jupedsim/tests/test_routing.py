# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import jupedsim as jps
import pytest
import shapely


def _path_distance(points: list[tuple[float, float]]) -> float:
    """Calculate total Euclidean length of a waypoint list."""
    return sum(
        math.hypot(x2 - x1, y2 - y1)
        for (x1, y1), (x2, y2) in zip(points[:-1], points[1:])
    )


def test_simple_path():
    surface = jps.WalkableSurface()
    surface.add_region(boundary=[(0, 0), (0, 10), (10, 10), (10, 0)])
    navi = jps.RoutingEngine(surface.create_geometry())

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


def _load_geometry_from_wkt(filename: str) -> jps.Geometry:
    """Load a WKT file given relative to the repository root."""
    repo_root = Path(__file__).parents[3]
    wkt = shapely.from_wkt((repo_root / filename).read_text(encoding="utf-8"))
    surface = jps.WalkableSurface()
    surface.add_region(polygon=shapely.union_all(shapely.get_parts(wkt)))
    return surface.create_geometry()


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
    geometry = _load_geometry_from_wkt(test_entry["wkt_path"])
    navi = jps.RoutingEngine(geometry)

    path1 = navi.compute_waypoints(
        test_entry["path1"][0], test_entry["path1"][1]
    )
    path2 = navi.compute_waypoints(
        test_entry["path2"][0], test_entry["path2"][1]
    )

    distance1 = _path_distance(path1)
    distance2 = _path_distance(path2)
    distance_diff = math.fabs(distance2 - distance1)
    assert distance_diff <= test_entry["max_diff"]
