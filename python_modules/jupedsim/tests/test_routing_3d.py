# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import pytest
import shapely
from jupedsim.geometry_utils import build_geometry_3d
from jupedsim.internal.routing_3d import (
    Geometry3D,
    SurfaceMeshShortestPathRoutingEngine,
)

OBJ = Path(__file__).parents[3] / "examples/geometry/multi_level_u_stair.obj"


@pytest.fixture
def engine():
    geo = Geometry3D.from_obj(str(OBJ))
    return SurfaceMeshShortestPathRoutingEngine(geo)


def test_valid_and_invalid_locations(engine):
    # points whose -z ray hits the surface (z given above it) are valid
    assert engine.is_valid_location((3, 4, 2))
    assert engine.is_valid_location((10, 10, 5))
    # outside the footprint the -z ray misses the surface entirely
    assert not engine.is_valid_location((100, 100, 50))
    assert not engine.is_valid_location((-5, -5, 10))


def test_shortest_path_across_stair(engine):
    source = (3, 4, 2)
    target = (13, 14, 15)
    path, cost = engine.get_shortest_path(source, target)

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


def test_geometry_regions_and_render_data():
    geo = Geometry3D.from_obj(str(OBJ))
    verts = geo.vertices()
    tris = geo.triangles()
    ids = geo.region_id_per_face()
    assert len(verts) > 0
    assert all(len(v) == 3 for v in verts)
    assert all(len(t) == 3 for t in tris)
    # one region id per triangle, in triangle order
    assert len(ids) == len(tris)
    # lower floor + whole stair merge into one region,
    # only the overlapping upper floor splits off -> exactly 2 regions.
    assert geo.region_count() == 2
    assert max(ids) == geo.region_count() - 1


def test_orientation_is_unit_vector(engine):
    d = engine.get_orientation((3, 4, 2), (13, 14, 15))
    assert len(d) == 2
    assert math.hypot(*d) == pytest.approx(1.0)


def test_build_geometry_3d_lifts_shapely_polygon():
    poly = shapely.Polygon(
        [(0, 0), (10, 0), (10, 10), (0, 10)],
        holes=[[(4, 4), (6, 4), (6, 6), (4, 6)]],
    )
    geo = build_geometry_3d(poly)

    assert geo.region_count() == 1
    assert geo.is_valid_location((1, 1, 1))
    assert not geo.is_valid_location((5, 5, 1))  # inside the hole
    assert not geo.is_valid_location((20, 20, 1))  # outside geometry
    # flat lift: every vertex at z=0
    assert all(v[2] == 0.0 for v in geo.vertices())

    # routing on the lifted geometry has to bend around the hole:
    # (2,5) -> (8,5) via the hole corners (4,4) and (6,4) (or the symmetric
    # pair above), length 2 + 2*sqrt(5).
    # NOTE: This test will fail once we wall clearance implemented in 3D.
    engine = SurfaceMeshShortestPathRoutingEngine(geo)
    path, cost = engine.get_shortest_path((2, 5, 1), (8, 5, 1))
    assert cost == pytest.approx(2 + 2 * math.sqrt(5))
    assert all(p[2] == pytest.approx(0.0) for p in path)


def test_build_geometry_3d_accepts_wkt():
    geo = build_geometry_3d("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))")
    assert geo.region_count() == 1
    assert geo.is_valid_location((5, 5, 1))


def test_fixed_target_several_sources(engine):
    target = (13, 14, 15)
    # repeated query from the same source yields same result
    _, c1 = engine.get_shortest_path((3, 4, 2), target)
    _, c2 = engine.get_shortest_path((3, 4, 2), target)
    assert c1 == pytest.approx(c2)
    # different source reuses the same target with its own cost
    _, c3 = engine.get_shortest_path((13, 13, 15), target)
    assert c3 != pytest.approx(2.0)  # just modified y
