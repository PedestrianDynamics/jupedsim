# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import pytest
from jupedsim.internal.routing_3d import SurfaceMeshShortestPathRoutingEngine

OBJ = Path(__file__).parents[3] / "examples/geometry/multi_level_u_stair.obj"


@pytest.fixture
def engine():
    e = SurfaceMeshShortestPathRoutingEngine()
    e.set_geometry_from_obj(str(OBJ))
    return e


def test_query_without_geometry_raises():
    e = SurfaceMeshShortestPathRoutingEngine()
    with pytest.raises(RuntimeError):
        e.is_valid_location((3, 4, 2))


def test_valid_and_invalid_locations(engine):
    # points whose -z ray hits the surface (z given above it) are valid
    assert engine.is_valid_location((3, 4, 2))
    assert engine.is_valid_location((10, 10, 5))
    # outside the footprint the -z ray misses the surface entirely
    assert not engine.is_valid_location((100, 100, 50))
    assert not engine.is_valid_location((-5, -5, 10))


def test_shortest_path_without_target_raises(engine):
    with pytest.raises(RuntimeError):
        engine.get_shortest_path((3, 4, 2))


def test_shortest_path_across_stair(engine):
    source = (3, 4, 2)
    target = (13, 14, 15)
    engine.set_target(target)
    path, cost = engine.get_shortest_path(source)

    assert isinstance(path, list)
    assert all(len(p) == 3 for p in path)
    # endpoints keep the query x/y and are projected onto the surface
    assert path[0][:2] == pytest.approx(source[:2])
    assert path[-1][:2] == pytest.approx(target[:2])
    assert path[0][2] == pytest.approx(0.0)
    assert path[-1][2] == pytest.approx(3.0)
    # the geodesic distance (no wall clearance)
    assert cost == pytest.approx(23.381, abs=1e-3)
    # Minimally it has to be >= direct euclidian distance
    # assert cost >= math.dist(path[0], path[-1])


def test_orientation_is_unit_vector(engine):
    engine.set_target((13, 14, 15))
    d = engine.get_orientation((3, 4, 2))
    assert len(d) == 2
    assert math.hypot(*d) == pytest.approx(1.0)


def test_fixed_target_several_sources(engine):
    engine.set_target((13, 14, 15))
    # repeated query from the same source yields same result
    _, c1 = engine.get_shortest_path((3, 4, 2))
    _, c2 = engine.get_shortest_path((3, 4, 2))
    assert c1 == pytest.approx(c2)
    # different source reuses the same target tree with its own cost
    _, c3 = engine.get_shortest_path((13, 13, 15))
    assert c3 != pytest.approx(2.0)  # just modified y
