# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim.native as py_jps
import pytest


def test_simulation_rejects_none_model():
    """Passing None as model should raise, not segfault."""
    surface = py_jps.WalkableSurface()
    surface.add_region(boundary=[(0, 0), (100, 0), (100, 100), (0, 100)])
    geometry = surface.create_geometry()

    with pytest.raises(Exception):
        py_jps.Simulation(model=None, geometry=geometry, dt=0.01)
