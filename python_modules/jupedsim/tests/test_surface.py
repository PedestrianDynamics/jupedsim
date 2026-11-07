# SPDX-License-Identifier: LGPL-3.0-or-later
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
    other_surface, other_floor0, _ = two_floor_surface(with_surface=True)
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
