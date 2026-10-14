# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from itertools import pairwise

import jupedsim as jps
import pytest


def test_a_circle_polygon_has_short_edges():
    for radius in (0.05, 0.2, 1.0, 10.0):
        corners = jps.circle_to_polygon(
            center=(3, -2), radius=radius
        ).exterior.coords
        assert len(corners) - 1 >= 4
        assert all(math.dist(a, b) <= 0.4 for a, b in pairwise(corners))
    # Not finer than needed: the edges of a large circle come close to the limit.
    corners = jps.circle_to_polygon(center=(3, -2), radius=10.0).exterior.coords
    assert math.dist(corners[0], corners[1]) > 0.35


@pytest.mark.parametrize("radius", [0.0, -1.0, math.nan])
def test_a_circle_needs_a_positive_radius(radius):
    with pytest.raises(ValueError, match="Circle radius must be positive"):
        jps.circle_to_polygon(center=(0, 0), radius=radius)
