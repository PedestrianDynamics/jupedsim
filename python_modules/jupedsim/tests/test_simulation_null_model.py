# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely


def test_simulation_rejects_none_model():
    """Passing None as model should raise, not segfault."""
    surface = jps.WalkableSurface()
    surface.add_region(
        polygon=shapely.Polygon([(0, 0), (100, 0), (100, 100), (0, 100)])
    )

    with pytest.raises(Exception):
        jps.Simulation(model=None, geometry=surface, dt=0.01)
