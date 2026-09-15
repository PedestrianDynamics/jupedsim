# SPDX-License-Identifier: LGPL-3.0-or-later

import re
from dataclasses import dataclass

import jupedsim as jps
import pytest


def rectangle(p1: tuple[float, float], p2: tuple[float, float]):
    return [p1, (p1[0], p2[1]), p2, (p2[0], p1[1])]


########### Error cases: Add Region ###########
def test_add_region_too_few_points():
    walkable_surface = jps.WalkableSurface()
    points = [(0, 0), (1, 1)]
    with pytest.raises(
        jps.SimulationError, match="needs at least 3 different points"
    ):  # as boundary
        walkable_surface.add_region(exterior=points, height=0.0)
    with pytest.raises(
        jps.SimulationError, match="needs at least 3 different points"
    ):  # as hole
        walkable_surface.add_region(
            exterior=rectangle((0, 0), (1, 1)), interior=[points], height=0.0
        )
    with pytest.raises(
        jps.SimulationError, match="needs at least 3 different points"
    ):  # as hole
        walkable_surface.add_region(
            exterior=rectangle((0, 0), (1, 1)), interior=[points], height=0.0
        )


def test_add_region_no_area():
    walkable_surface = jps.WalkableSurface()
    points = [(0, 0), (1, 1), (1, 2), (1, 1)]
    with pytest.raises(
        jps.SimulationError, match="polygon does not form an area"
    ):
        walkable_surface.add_region(exterior=points, height=0.0)


def test_add_region_rejects_bowtie():
    walkable_surface = jps.WalkableSurface()
    polygon = [(0, 0), (1, 1), (0, 1), (1, 0)]
    with pytest.raises(jps.SimulationError, match="crosses itself"):
        walkable_surface.add_region(exterior=polygon, height=0.0)


def test_overlapping_geometry():
    walkable_surface = jps.WalkableSurface()
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (10, 10)), height=0.0
    )
    with pytest.raises(
        jps.SimulationError, match=f"New region overlaps with region {id_0}"
    ):
        walkable_surface.add_region(
            exterior=rectangle((5, 5), (15, 15)), height=0.0
        )


def test_touching_polygons():
    walkable_surface = jps.WalkableSurface()
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (1, 1)), height=0.0
    )
    with pytest.raises(jps.SimulationError, match=f"touches region {id_0}"):
        walkable_surface.add_region(
            exterior=rectangle((1, 1), (3, 3)), height=0.0
        )


def test_gets_region_after_error():
    walkable_surface = jps.WalkableSurface()
    polygon = [(0, 0), (1, 1), (0, 1), (1, 0)]
    with pytest.raises(jps.SimulationError, match="crosses itself"):
        walkable_surface.add_region(exterior=polygon, height=0.0)
    region_id = walkable_surface.add_region(
        exterior=rectangle((0, 0), (1, 1)), height=0.0
    )
    assert region_id == 0


########### Error cases: Connect Regions ###########
@dataclass
class TwoFloors:
    surface: jps.WalkableSurface
    id_0: int
    id_1: int


@pytest.fixture
def two_floors():
    walkable_surface = jps.WalkableSurface()
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (5, 5)), height=0.0
    )
    id_1 = walkable_surface.add_region(
        exterior=rectangle((10, 0), (15, 5)), height=0.0
    )
    return TwoFloors(walkable_surface, id_0, id_1)


def test_connect_same_region(two_floors):
    with pytest.raises(jps.SimulationError, match="may not be the same region"):
        two_floors.surface.connect_regions(
            from_region=two_floors.id_0,
            from_edge=((0, 0), (0, 5)),
            to_region=two_floors.id_0,
            to_edge=((5, 0), (5, 5)),
        )


def test_connect_with_nonexisting_point(two_floors):
    with pytest.raises(
        jps.SimulationError,
        match=f"is not an edge of region {two_floors.id_1}",
    ):
        two_floors.surface.connect_regions(
            from_region=two_floors.id_0,
            from_edge=((5, 0), (5, 5)),
            to_region=two_floors.id_1,
            to_edge=((10, 1), (10, 5)),  # (10, 1) does not exist
        )


def test_connect_points_but_no_edge(two_floors):
    with pytest.raises(
        jps.SimulationError,
        match=f"is not an edge of region {two_floors.id_0}",
    ):
        two_floors.surface.connect_regions(
            from_region=two_floors.id_0,
            from_edge=((0, 0), (5, 5)),  # diagonal
            to_region=two_floors.id_1,
            to_edge=((10, 0), (10, 5)),
        )


########### Error cases: Create Geometry ###########
def test_unconnected_geometry():
    walkable_surface = jps.WalkableSurface()
    polygon = rectangle((0, 0), (1, 1))
    walkable_surface.add_region(exterior=polygon, height=0.0)
    walkable_surface.add_region(exterior=polygon, height=3.0)
    with pytest.raises(jps.SimulationError, match="is not connected"):
        walkable_surface.create_geometry()


def test_stairs_through_other_floor():
    walkable_surface = jps.WalkableSurface()
    floor = rectangle((0, 0), (100, 100))
    ground_floor = walkable_surface.add_region(
        exterior=floor, interior=[rectangle((20, 20), (24, 22))], height=0.0
    )
    first_floor = walkable_surface.add_region(exterior=floor, height=3.0)
    second_floor = walkable_surface.add_region(
        exterior=floor, interior=[rectangle((36, 40), (40, 42))], height=6.0
    )
    stairs = walkable_surface.connect_regions(
        from_region=ground_floor,
        from_edge=((20, 20), (20, 22)),
        to_region=second_floor,
        to_edge=((40, 40), (40, 42)),
    )
    with pytest.raises(jps.SimulationError) as error:
        walkable_surface.create_geometry()
    message = str(error.value)
    assert "pass through each other" in message
    assert re.search(rf"[Rr]egion {first_floor}\b", message)
    assert re.search(rf"[Rr]egion {stairs}\b", message)


########### Correct cases ###########
def test_new_geometry_definition_v1():
    ground_floor = {
        "exterior": rectangle((0, 0), (10, 10)),
        "interior": [[(2, 2), (6, 2), (6, 2.2), (6, 3.8), (6, 4), (2, 4)]],
        "height": 0.0,
    }
    first_floor = {
        "exterior": rectangle((0, 0), (10, 10)),
        "interior": [[(2, 4), (2, 3.8), (2, 2.2), (2, 2), (6, 4), (2, 4)]],
        "height": 3.0,
    }

    walkable_surface = jps.WalkableSurface()

    id_0 = walkable_surface.add_region(**ground_floor)
    id_1 = walkable_surface.add_region(**first_floor)
    id_0_to_1 = walkable_surface.connect_regions(
        from_region=id_0,
        from_edge=((6, 2.2), (6, 3.8)),
        to_region=id_1,
        to_edge=((2, 2.2), (2, 3.8)),
    )
    assert [id_0, id_1, id_0_to_1] == [0, 1, 2]


def test_striped_floor():
    walkable_surface = jps.WalkableSurface()
    num = 10
    prev_id = -1
    for x in range(num):
        stripe = rectangle((2 * x, 0), (2 * x + 1, 1))
        new_id = walkable_surface.add_region(exterior=stripe)
        if x > 0:
            walkable_surface.connect_regions(
                from_region=prev_id,
                from_edge=((2 * x - 1, 0), (2 * x - 1, 1)),
                to_region=new_id,
                to_edge=((2 * x, 0), (2 * x, 1)),
            )
        prev_id = new_id

    geo = walkable_surface.create_geometry()
    assert geo.region_count() == 2 * num - 1
