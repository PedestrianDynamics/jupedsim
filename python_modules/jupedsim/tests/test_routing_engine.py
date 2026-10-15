# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from pathlib import Path

import jupedsim as jps
import pytest
import shapely

OBJ = Path(__file__).parents[3] / "examples/geometry/multi_level_u_stair.obj"


@pytest.fixture
def engine():
    geo = jps.Geometry.from_obj(str(OBJ))
    return jps.SurfaceMeshShortestPathRoutingEngine(geo)


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
    path = engine.get_shortest_path(source, target)

    assert isinstance(path, list)
    assert all(len(p) == 3 for p in path)
    # endpoints keep the query x/y and are projected onto the surface
    assert path[0][:2] == pytest.approx(source[:2])
    assert path[-1][:2] == pytest.approx(target[:2])
    assert path[0][2] == pytest.approx(0.0)
    assert path[-1][2] == pytest.approx(3.0)
    # the way leads over the stair, so it is longer than the straight line through the air
    length = sum(math.dist(a, b) for a, b in zip(path, path[1:]))
    assert length > math.dist(path[0], path[-1])


def test_geometry_regions_and_render_data():
    geo = jps.Geometry.from_obj(str(OBJ))
    verts = geo.vertices()
    tris = geo.triangles()
    ids = geo.region_id_per_face()
    assert len(verts) > 0
    assert all(len(v) == 3 for v in verts)
    assert all(len(t) == 3 for t in tris)
    # one region id per triangle, in triangle order
    assert len(ids) == len(tris)
    # Four regions: lower floor, upper floor, first flight + mid landing, and
    # the second flight. Fewer is not possible: each flight's border coincides
    # in (x, y) with stairwell walls it shares no mesh edge with (see
    # RegionSplit.AFlightCoincidingWithTheStairwellWallsSplitsOff), and the two
    # flights overlap each other in plan across the U-turn.
    assert geo.region_count() == 4
    assert max(ids) == geo.region_count() - 1


def test_orientation_is_unit_vector():
    geo = jps.Geometry.from_obj(str(OBJ))
    engine = jps.SurfaceMeshShortestPathRoutingEngine(geo)
    d = engine.get_orientation(
        # from the ground floor (region 0) to the upper floor (region 1)
        geo.get_location(3, 4, region_id=0),
        geo.get_location(13, 14, region_id=1),
    )
    assert len(d) == 2
    assert math.hypot(*d) == pytest.approx(1.0)


def test_a_route_on_a_flat_region_keeps_clear_of_a_hole():
    surface = jps.WalkableSurface()
    surface.add_region(
        polygon=shapely.Polygon(
            [(0, 0), (10, 0), (10, 10), (0, 10)],
            holes=[[(4, 4), (6, 4), (6, 6), (4, 6)]],
        )
    )
    geo = surface.create_geometry()

    assert geo.region_count() == 1
    assert geo.is_valid_location((1, 1, 1))
    assert not geo.is_valid_location((5, 5, 1))  # inside the hole
    assert not geo.is_valid_location((20, 20, 1))  # outside geometry
    # flat lift: every vertex at z=0
    assert all(v[2] == 0.0 for v in geo.vertices())

    # the route has to bend around the hole: (2,5) -> (8,5) turns on two of
    # its corners, and the route is held off each of them by the wall clearance -- so it is
    # longer than the geodesic through them, which measures 2 + 2*sqrt(5).
    engine = jps.SurfaceMeshShortestPathRoutingEngine(geo)
    path = engine.get_shortest_path((2, 5, 1), (8, 5, 1))
    corners = [(4, 4), (6, 4), (4, 6), (6, 6)]
    to_nearest_corner = [
        min(math.dist(p[:2], c) for c in corners) for p in path
    ]
    # nothing on the route comes closer to a corner than the clearance
    assert min(to_nearest_corner) >= engine.wall_clearance() - 1e-9
    # and the two corners it turns on are held off by exactly that much
    held_off = [
        d
        for d in to_nearest_corner
        if d == pytest.approx(engine.wall_clearance())
    ]
    assert len(held_off) == 2
    length = sum(math.dist(a, b) for a, b in zip(path, path[1:]))
    assert length > 2 + 2 * math.sqrt(5)
    assert all(p[2] == pytest.approx(0.0) for p in path)


def test_fixed_target_several_sources(engine):
    target = (13, 14, 15)
    # repeated query from the same source yields same result
    first = engine.get_shortest_path((3, 4, 2), target)
    again = engine.get_shortest_path((3, 4, 2), target)
    assert first == again
    # a different source reuses the same target and gets its own way there
    other = engine.get_shortest_path((13, 13, 15), target)
    assert other != first


def _path_length(path):
    return sum(math.dist(a, b) for a, b in zip(path, path[1:]))


def test_simple_path():
    surface = jps.WalkableSurface()
    surface.add_region(boundary=[(0, 0), (0, 10), (10, 10), (10, 0)])
    engine = jps.SurfaceMeshShortestPathRoutingEngine(surface.create_geometry())

    # Ensure it does not cross the triangles so that the path length is 2
    path = engine.get_shortest_path((1, 1, 0), (4, 1, 0))
    assert len(path) == 2
    # distance is simple as we only move along the x-axis
    assert math.isclose(_path_length(path), 3)


def _load_geometry_from_wkt(filename: str) -> jps.Geometry:
    """Load a WKT file given relative to the repository root."""
    repo_root = Path(__file__).parents[3]
    wkt = shapely.from_wkt((repo_root / filename).read_text(encoding="utf-8"))
    surface = jps.WalkableSurface()
    surface.add_region(polygon=shapely.union_all(shapely.get_parts(wkt)))
    return surface.create_geometry()


ROUTE_LENGTH_CONTINUITY = [
    {
        "test_name": "corner_with_shortcut",
        "description": "Same starting point, but end points differ just 0.2 on y axis",
        "wkt_path": "examples/geometry/corner_with_shortcut.wkt",
        "path1": [(11.43, 0.44, 0), (27.93, 15.0, 0)],
        "path2": [(11.43, 0.44, 0), (27.93, 15.2, 0)],
        "max_diff": 0.2,
    },
    {
        "test_name": "corner_with_shortcut2",
        "description": "Same starting point, but end points differ just 0.01 on y axis",
        "wkt_path": "examples/geometry/corner_with_shortcut.wkt",
        "path1": [(11.80, 1.00, 0), (28.50, 13.54, 0)],
        "path2": [(11.80, 1.00, 0), (28.50, 13.55, 0)],
        "max_diff": 0.01,
    },
]


@pytest.mark.parametrize(
    "test_entry",
    ROUTE_LENGTH_CONTINUITY,
    ids=lambda params: params["test_name"],
)
def test_route_length_follows_the_target(test_entry):
    geometry = _load_geometry_from_wkt(test_entry["wkt_path"])
    engine = jps.SurfaceMeshShortestPathRoutingEngine(geometry)

    path1 = engine.get_shortest_path(*test_entry["path1"])
    path2 = engine.get_shortest_path(*test_entry["path2"])

    distance_diff = math.fabs(_path_length(path2) - _path_length(path1))
    assert distance_diff <= test_entry["max_diff"]
