# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim.native as py_jps
import pytest


def test_simulation_rejects_none_model():
    """Passing None as model should raise, not segfault."""
    geo_builder = py_jps.GeometryBuilder()
    geo_builder.add_accessible_area([(0, 0), (100, 0), (100, 100), (0, 100)])
    geometry = geo_builder.build()

    with pytest.raises(Exception):
        py_jps.Simulation(model=None, geometry=geometry, dt=0.01)
