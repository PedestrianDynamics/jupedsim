# SPDX-License-Identifier: LGPL-3.0-or-later
"""Smoke checks for the portal boundary index on a real multi-level mesh.

The portal index only answers correctly while every region of the split
projects injectively onto (x, y), which split_into_regions() promises --
the region-count guard protects that promise on a mesh that once violated
it, and the invariant sweep holds Query()'s contract over a grid of real
queries.
"""

import math
from pathlib import Path

import pytest
from jupedsim.internal.routing import (
    Geometry,
    make_portal_boundary_index,
)

MESH = (
    Path(__file__).parent.parent
    / "examples"
    / "geometry"
    / "multi_level_u_stair.obj"
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


def canonical(segment) -> tuple:
    a = tuple(round(c, 9) for c in segment.p1)
    b = tuple(round(c, 9) for c in segment.p2)
    return (a, b) if a <= b else (b, a)


@pytest.fixture(scope="module")
def geometry() -> Geometry:
    return Geometry.from_obj(str(MESH))


def test_mesh_splits_into_multiple_regions(geometry) -> None:
    # The folded U-stair cannot share a region with the floors it connects. The
    # split certifies every region's projection as injective (strictly simple
    # boundary), so a low count no longer signals folded mega-regions -- only a
    # single region would mean the overlap went undetected.
    assert geometry.region_count() >= 2


def test_query_invariants_hold_over_a_grid(geometry) -> None:
    index = make_portal_boundary_index(geometry)
    queries = 0
    for x in range(0, 26, 2):
        for y in range(0, 26, 2):
            for z_hint in (0.0, 1.5, 3.0, 4.5):
                location = geometry.get_location(x, y, z_hint)
                if location is None:
                    continue
                for radius in (1.0, 3.0, 100.0):
                    seen = index.query(location, radius)
                    keys = {canonical(s) for s in seen}
                    at = f"({x}, {y}, z_hint={z_hint}), radius={radius}"
                    assert len(keys) == len(seen), f"duplicate piece at {at}"
                    for segment in seen:
                        reach = distance_to_segment(
                            (x, y), segment.p1[:2], segment.p2[:2]
                        )
                        assert reach <= radius + 1e-6, (
                            f"piece beyond the radius at {at}: {reach}"
                        )
                    queries += 1
    # A silent no-op grid would pass vacuously; make sure we actually queried.
    assert queries > 250


def test_fixed_sample_points_see_walls(geometry) -> None:
    index = make_portal_boundary_index(geometry)
    for x, y, z_hint in ((1.0, 1.0, 0.0), (20.0, 14.5, 0.0)):
        location = geometry.get_location(x, y, z_hint)
        assert location is not None
        assert len(index.query(location, 5.0)) > 0
