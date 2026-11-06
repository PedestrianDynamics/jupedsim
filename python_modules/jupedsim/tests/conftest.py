# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest


def _rect(x0, y0, x1, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


@pytest.fixture(scope="module")
def u_stair_geometry() -> jps.Geometry:
    """Two 20 x 20 floors (z 0 and 3) joined by a U-stair with a landing at z 1.5.

    Regions: ground 0, upper 1, landing 2, and the two flights.
    """
    # (8, 9) and (13, 9) split the stairwell sides so each flight has an edge of its own.
    stairwell = [(8, 6), (14, 6), (14, 12), (8, 12), (8, 9)]
    builder = jps.GeometryBuilder()
    ground = builder.add_region(
        boundary=_rect(0, 0, 20, 20), holes=[stairwell], height=0.0
    )
    upper = builder.add_region(
        boundary=_rect(0, 0, 20, 20), holes=[stairwell], height=3.0
    )
    landing = builder.add_region(
        boundary=[(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)], height=1.5
    )
    builder.connect_regions(
        from_region=ground,
        from_edge=((8, 6), (8, 9)),
        to_region=landing,
        to_edge=((13, 6), (13, 9)),
    )
    builder.connect_regions(
        from_region=landing,
        from_edge=((13, 9), (13, 12)),
        to_region=upper,
        to_edge=((8, 9), (8, 12)),
    )
    return builder.build()
