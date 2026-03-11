# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps


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
