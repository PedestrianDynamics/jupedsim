# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
from jupedsim.surface import WalkableSurface

FOOTPRINT = [(0, 0), (20, 0), (20, 10), (0, 10)]
STAIRWELL = [(12, 6), (18, 6), (18, 8), (12, 8)]


def two_floor_surface(with_surface=False):
    surface = WalkableSurface()
    floor0 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[STAIRWELL]),
        height=0.0,
        name="floor0",
    )
    floor1 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[STAIRWELL]),
        height=3.0,
        name="floor1",
    )
    surface.connect_regions(
        from_region=floor0,
        from_edge=((12, 6), (12, 8)),
        to_region=floor1,
        to_edge=((18, 6), (18, 8)),
        name="staircase",
    )
    if with_surface:
        return surface, floor0, floor1
    return floor0, floor1


def test_location_equality():
    floor0, floor1 = two_floor_surface()
    assert floor0.at(5, 2) == floor0.at(5, 2)
    assert floor0.at(5, 2) != floor1.at(5, 2)
    assert hash(floor0.at(5, 2)) == hash(floor0.at(5, 2))


def test_location_accessors():
    floor0, _ = two_floor_surface()
    location = floor0.at(5, 2)
    assert location.xy == (5.0, 2.0)
    assert location.region == floor0


def test_regions_at_stacked_floors():
    surface, floor0, floor1 = two_floor_surface(with_surface=True)
    assert surface.regions_at((10, 5)) == (floor0, floor1)


def test_location_checked_against_surface():
    floor0, _ = two_floor_surface()
    with pytest.raises(ValueError, match=r"\(100, 100\) is not in floor0"):
        floor0.at(100, 100)


def test_location_outside_region_but_in_other():
    _, floor1 = two_floor_surface()
    with pytest.raises(ValueError):
        floor1.at(15, 7)  # stairwell hole


def test_region_lookup():
    surface, floor0, _ = two_floor_surface(with_surface=True)
    assert surface.region("floor0") == floor0
    assert surface.region(id=floor0.id) == floor0
    with pytest.raises(KeyError, match="regions are floor0, floor1, staircase"):
        surface.region("floor2")


def test_regions_and_connector():
    surface, floor0, floor1 = two_floor_surface(with_surface=True)
    staircase = surface.region("staircase")
    assert surface.regions == (floor0, floor1, staircase)
    assert staircase.height is None
    assert floor1.height == 3.0
    assert surface.regions_at((15, 7)) == (staircase,)


def test_region_identity_is_per_surface():
    _, floor0, _ = two_floor_surface(with_surface=True)
    _, other_floor0, _ = two_floor_surface(with_surface=True)
    assert floor0 != other_floor0


def test_lookup_after_adding_region():
    surface = WalkableSurface()
    a = surface.add_region(polygon=shapely.box(0, 0, 1, 1))
    assert a.at(0.5, 0.5).xy == (0.5, 0.5)
    b = surface.add_region(polygon=shapely.box(3, 0, 4, 1))
    connector = surface.connect_regions(
        from_region=a,
        from_edge=((1, 0), (1, 1)),
        to_region=b,
        to_edge=((3, 0), (3, 1)),
    )
    assert b.at(3.5, 0.5).xy == (3.5, 0.5)
    assert connector.at(2.0, 0.5).xy == (2.0, 0.5)


def test_region_requires_exactly_one_key():
    surface, floor0, _ = two_floor_surface(with_surface=True)
    with pytest.raises(TypeError, match="name or an id"):
        surface.region()
    with pytest.raises(TypeError, match="name or an id"):
        surface.region("floor0", id=floor0.id)


def test_region_id_not_found():
    surface, _, _ = two_floor_surface(with_surface=True)
    with pytest.raises(KeyError, match="no region with id 7"):
        surface.region(id=7)


def test_lookup_on_unconnected_surface():
    surface = WalkableSurface()
    floor0 = surface.add_region(polygon=shapely.box(0, 0, 1, 1), name="floor0")
    floor1 = surface.add_region(polygon=shapely.box(3, 0, 4, 1), name="floor1")
    assert floor0.at(0.5, 0.5).region == floor0
    assert floor1.at(3.5, 0.5).region == floor1
    assert surface.regions_at((0.5, 0.5)) == (floor0,)
    with pytest.raises(ValueError, match=r"\(3.5, 0.5\) is not in floor0"):
        floor0.at(3.5, 0.5)


def test_lookup_respects_holes_and_boundary():
    floor0, _ = two_floor_surface()
    assert floor0.at(0, 0).xy == (0.0, 0.0)
    assert floor0.at(12, 7).xy == (12.0, 7.0)
    with pytest.raises(ValueError, match=r"\(15, 7\) is not in floor0"):
        floor0.at(15, 7)


def test_lookup_survives_an_unconnected_region_added_later():
    surface, floor0, floor1 = two_floor_surface(with_surface=True)
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    surface.add_region(polygon=shapely.box(30, 0, 40, 10), name="later")
    assert floor0.at(3, 5).region == floor0
    assert floor1.at(3, 5).region == floor1
    assert sim.add_agent(floor0.at(3, 5), target=floor0.at(5, 5)).alive


def test_connector_lookup_survives_an_unconnected_region_added_later():
    surface, floor0, floor1 = two_floor_surface(with_surface=True)
    staircase = surface.region("staircase")
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    later = surface.add_region(polygon=shapely.box(30, 0, 40, 10), name="later")
    assert staircase.at(15, 7).region == staircase
    assert staircase.at(12, 6).xy == (12.0, 6.0)
    with pytest.raises(ValueError, match=r"\(11, 7\) is not in staircase"):
        staircase.at(11, 7)
    assert surface.regions_at((15, 7)) == (staircase,)
    assert surface.regions_at((3, 5)) == (floor0, floor1)
    assert surface.regions_at((35, 5)) == (later,)
    # The existing simulation takes locations of its own regions only.
    for location in (staircase.at(15, 7), floor1.at(3, 5)):
        assert sim.add_agent(location, target=floor0.at(5, 5)).alive
    with pytest.raises(
        ValueError, match="later is not part of this simulation's geometry"
    ):
        sim.add_agent(later.at(35, 5), target=floor0.at(5, 5))


def test_connect_rejects_foreign_region():
    surface, floor0, _ = two_floor_surface(with_surface=True)
    _, other0, other1 = two_floor_surface(with_surface=True)
    with pytest.raises(ValueError, match="belongs to another surface"):
        surface.connect_regions(
            from_region=floor0,
            from_edge=((12, 6), (12, 8)),
            to_region=other1,
            to_edge=((18, 6), (18, 8)),
        )


def test_connect_rejects_edge_off_the_boundary():
    surface, floor0, floor1 = two_floor_surface(with_surface=True)
    with pytest.raises(
        ValueError,
        match="^from_edge is not an edge of floor0's boundary$",
    ):
        surface.connect_regions(
            from_region=floor0,
            from_edge=((5, 10), (5, 0)),
            to_region=floor1,
            to_edge=((18, 6), (18, 8)),
        )
    with pytest.raises(
        ValueError, match="^to_edge is not an edge of floor1's boundary$"
    ):
        surface.connect_regions(
            from_region=floor0,
            from_edge=((12, 6), (12, 8)),
            to_region=floor1,
            to_edge=((5, 10), (5, 0)),
        )


def test_connect_rejects_connector():
    surface, floor0, _ = two_floor_surface(with_surface=True)
    floor2 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[STAIRWELL]),
        height=6.0,
        name="floor2",
    )
    staircase = surface.region("staircase")
    with pytest.raises(ValueError) as info:
        surface.connect_regions(
            from_region=staircase,
            from_edge=((12, 6), (12, 8)),
            to_region=floor2,
            to_edge=((18, 6), (18, 8)),
        )
    assert str(info.value) == (
        "staircase is a connector and cannot be connected again; add a "
        "landing region between two connectors"
    )


@pytest.mark.parametrize("offset", [1e-6, -1e-6])
def test_connector_lookup_uses_the_canonical_vertices(offset):
    surface = WalkableSurface()
    floor0 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[STAIRWELL]), height=0.0
    )
    floor1 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[STAIRWELL]), height=3.0
    )
    # The native surface takes the corners from the regions' vertices, so
    # the seam lies at x = 12 for both offsets.
    staircase = surface.connect_regions(
        from_region=floor0,
        from_edge=((12 + offset, 6), (12 + offset, 8)),
        to_region=floor1,
        to_edge=((18, 6), (18, 8)),
        name="staircase",
    )
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    seam = staircase.at(12, 7)
    assert sim.add_agent(floor0.at(5, 5), target=seam).alive
    assert staircase in surface.regions_at((12, 7))
    with pytest.raises(ValueError, match="is not in staircase"):
        staircase.at(11.9999995, 7)
    assert staircase not in surface.regions_at((11.9999995, 7))


def test_duplicate_region_name_is_rejected():
    surface = WalkableSurface()
    surface.add_region(polygon=shapely.box(0, 0, 1, 1), name="floor")
    with pytest.raises(ValueError, match="region name 'floor' is already"):
        surface.add_region(polygon=shapely.box(2, 0, 3, 1), name="floor")
    assert len(surface.regions) == 1


def test_duplicate_connector_name_is_rejected():
    surface, floor0, floor1 = two_floor_surface(with_surface=True)
    with pytest.raises(ValueError, match="region name 'staircase' is already"):
        surface.connect_regions(
            from_region=floor0,
            from_edge=((18, 6), (18, 8)),
            to_region=floor1,
            to_edge=((12, 6), (12, 8)),
            name="staircase",
        )
    assert len(surface.regions) == 3
