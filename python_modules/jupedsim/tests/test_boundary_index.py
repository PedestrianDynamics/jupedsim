# SPDX-License-Identifier: LGPL-3.0-or-later
"""Smoke checks for the portal boundary index on a multi-level geometry: the U-stair.

The invariant sweep holds Query()'s contract over a grid of real queries.
"""

import math

from jupedsim import (
    SimulationError,
    make_portal_boundary_index,
)


def distance_to_segment(p, a, b) -> float:
    """Distance from point p to the segment a-b, all as (x, y) tuples."""
    ax, ay = a
    bx, by = b
    px, py = p
    dx, dy = bx - ax, by - ay
    length2 = dx * dx + dy * dy
    t = 0.0
    if length2 > 0.0:
        t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / length2))
    nx, ny = ax + t * dx, ay + t * dy
    return math.hypot(px - nx, py - ny)


def locations_at(geometry, x, y):
    """Every location over (x, y), one per region that contains it."""
    for region_id in range(geometry.region_count()):
        try:
            yield geometry.get_location(x, y, region_id=region_id)
        except SimulationError:
            pass


def canonical(segment) -> tuple:
    a = tuple(round(c, 9) for c in segment[0])
    b = tuple(round(c, 9) for c in segment[1])
    return (a, b) if a <= b else (b, a)


def test_geometry_has_multiple_regions(u_stair_geometry) -> None:
    # ground floor, upper floor, landing and the two flights
    assert u_stair_geometry.region_count() == 5


def test_query_invariants_hold_over_a_grid(u_stair_geometry) -> None:
    index = make_portal_boundary_index(u_stair_geometry)
    queries = 0
    for x in range(0, 26, 2):
        for y in range(0, 26, 2):
            for location in locations_at(u_stair_geometry, x, y):
                for radius in (1.0, 3.0, 100.0):
                    seen = index.query(location, radius)
                    keys = {canonical(s) for s in seen}
                    at = f"({x}, {y}, region_id={location.region_id}), radius={radius}"
                    assert len(keys) == len(seen), f"duplicate piece at {at}"
                    for segment in seen:
                        reach = distance_to_segment(
                            (x, y), segment[0][:2], segment[1][:2]
                        )
                        assert reach <= radius + 1e-6, (
                            f"piece beyond the radius at {at}: {reach}"
                        )
                    queries += 1
    # A silent no-op grid would pass vacuously; make sure we actually queried.
    assert queries > 250


def test_fixed_sample_points_see_walls(u_stair_geometry) -> None:
    index = make_portal_boundary_index(u_stair_geometry)
    for x, y in ((1.0, 1.0), (19.0, 14.5)):
        ground_floor = min(
            locations_at(u_stair_geometry, x, y), key=lambda loc: loc.z
        )
        assert len(index.query(ground_floor, 5.0)) > 0
