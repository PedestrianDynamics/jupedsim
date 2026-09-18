# SPDX-License-Identifier: LGPL-3.0-or-later

import re
from dataclasses import dataclass

import jupedsim as jps
import pytest
import shapely


########### Helper functions and fixtures ###########
def rectangle(p1: tuple[float, float], p2: tuple[float, float]):
    return [p1, (p1[0], p2[1]), p2, (p2[0], p1[1])]


@dataclass
class TwoFloors:
    surface: jps.WalkableSurface
    ground_floor: int
    upper_floor: int
    ground_conn_edge = ((5, 0), (5, 5))
    upper_conn_edge = ((10, 0), (10, 5))

    def add_stairs(self):
        return self.surface.connect_regions(
            from_region=self.ground_floor,
            from_edge=self.ground_conn_edge,
            to_region=self.upper_floor,
            to_edge=self.upper_conn_edge,
        )


@pytest.fixture
def walkable_surface():
    return jps.WalkableSurface()


@pytest.fixture
def two_floors(walkable_surface):
    ground_floor = walkable_surface.add_region(
        exterior=rectangle((0, 0), (5, 5)), height=0.0
    )
    upper_floor = walkable_surface.add_region(
        exterior=rectangle((10, 0), (15, 5)), height=3.0
    )
    return TwoFloors(walkable_surface, ground_floor, upper_floor)


########### Error cases: Add Region ###########
@pytest.mark.parametrize(
    "exterior, interior",
    [
        pytest.param([(0, 0), (1, 1)], [], id="boundary"),
        pytest.param(rectangle((0, 0), (1, 1)), [[(0, 0), (1, 1)]], id="hole"),
    ],
)
def test_add_region_too_few_points(walkable_surface, exterior, interior):
    with pytest.raises(
        jps.SimulationError, match="needs at least 3 different points"
    ):
        walkable_surface.add_region(exterior=exterior, interior=interior)


@pytest.mark.parametrize(
    "exterior, interior",
    [
        pytest.param([(0, 0), (1, 1), (1, 2), (1, 1)], [], id="no-area"),
        pytest.param([(0, 0), (1, 1), (0, 1), (1, 0)], [], id="bowtie"),
        pytest.param(
            [(0, 0), (0, 0), (1, 0), (1, 1)],
            [],
            id="duplicated-point-in-boundary",
        ),
        pytest.param(
            rectangle((-1, -1), (2, 2)),
            [[(0, 0), (0, 0), (1, 0), (1, 1)]],
            id="duplicated-point-in-hole",
        ),
    ],
)
def test_add_region_not_simple(walkable_surface, exterior, interior):
    with pytest.raises(jps.SimulationError, match="is not simple"):
        walkable_surface.add_region(exterior=exterior, interior=interior)


def test_overlapping_geometry(walkable_surface):
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (10, 10)), height=0.0
    )
    with pytest.raises(
        jps.SimulationError, match=f"New region overlaps with region {id_0}"
    ):
        walkable_surface.add_region(
            exterior=rectangle((5, 5), (15, 15)), height=0.0
        )


def test_touching_polygons(walkable_surface):
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (1, 1)), height=0.0
    )
    with pytest.raises(jps.SimulationError, match=f"touches region {id_0}"):
        walkable_surface.add_region(
            exterior=rectangle((1, 1), (3, 3)), height=0.0
        )


@pytest.mark.parametrize(
    "exterior, interior",
    [
        pytest.param(
            rectangle((0, 0), (3, 3)),
            [[(0, 0), (1, 1), (2, 1)]],
            id="hole-at-boundary-corner",
        ),
        pytest.param(
            rectangle((0, 0), (3, 3)),
            [[(1, 0), (1, 1), (2, 1)]],
            id="hole-on-boundary-edge",
        ),
        pytest.param(
            rectangle((0, 0), (3, 3)),
            [rectangle((0, 0), (3, 3))],
            id="hole-equal-to-boundary",
        ),
        pytest.param(
            rectangle((0, 0), (3, 3)),
            [rectangle((5, 5), (7, 7))],
            id="hole-outside-boundary",
        ),
        pytest.param(
            rectangle((0, 0), (3, 3)),
            [rectangle((2, 1), (5, 2))],
            id="hole-crosses-boundary",
        ),
        pytest.param(
            rectangle((0, 0), (10, 10)),
            [rectangle((1, 1), (9, 9)), rectangle((3, 3), (5, 5))],
            id="hole-inside-hole",
        ),
        pytest.param(
            rectangle((0, 0), (10, 10)),
            [rectangle((1, 1), (3, 3)), rectangle((1, 1), (3, 3))],
            id="identical-holes",
        ),
        pytest.param(
            rectangle((0, 0), (10, 10)),
            [rectangle((1, 1), (3, 3)), rectangle((2, 2), (4, 4))],
            id="holes-cross-each-other",
        ),
        pytest.param(
            rectangle((0, 0), (10, 10)),
            [rectangle((1, 1), (3, 3)), rectangle((3, 3), (4, 4))],
            id="holes-touch-at-corner",
        ),
    ],
)
def test_add_region_invalid_holes(walkable_surface, exterior, interior):
    with pytest.raises(
        jps.SimulationError, match="Holes must lie strictly inside the boundary"
    ):
        walkable_surface.add_region(exterior=exterior, interior=interior)


def test_gets_region_after_error_and_can_create_geometry(walkable_surface):
    polygon = [(0, 0), (1, 1), (0, 1), (1, 0)]
    with pytest.raises(jps.SimulationError, match="is not simple"):
        walkable_surface.add_region(exterior=polygon, height=0.0)
    region_id = walkable_surface.add_region(
        exterior=rectangle((0, 0), (1, 1)), height=0.0
    )
    assert region_id == 0
    walkable_surface.create_geometry()


########### Error cases: Connect Regions ###########
def test_connect_with_unknown_region(two_floors):
    # from region
    with pytest.raises(
        jps.SimulationError, match="Unknown region id used for fromRegion"
    ):
        two_floors.surface.connect_regions(
            from_region=42,
            from_edge=two_floors.ground_conn_edge,
            to_region=two_floors.upper_floor,
            to_edge=two_floors.upper_conn_edge,
        )
    # to region
    with pytest.raises(
        jps.SimulationError, match="Unknown region id used for toRegion"
    ):
        two_floors.surface.connect_regions(
            from_region=two_floors.ground_floor,
            from_edge=two_floors.ground_conn_edge,
            to_region=42,
            to_edge=two_floors.upper_conn_edge,
        )


def test_connect_same_region(two_floors):
    with pytest.raises(jps.SimulationError, match="may not be the same region"):
        two_floors.surface.connect_regions(
            from_region=two_floors.ground_floor,
            from_edge=((0, 0), (0, 5)),
            to_region=two_floors.ground_floor,
            to_edge=two_floors.ground_conn_edge,
        )


def test_connect_with_nonexisting_point(two_floors):
    with pytest.raises(
        jps.SimulationError,
        match=f"is not an edge of region {two_floors.upper_floor}",
    ):
        two_floors.surface.connect_regions(
            from_region=two_floors.ground_floor,
            from_edge=two_floors.ground_conn_edge,
            to_region=two_floors.upper_floor,
            to_edge=((10, 1), (10, 5)),  # (10, 1) does not exist
        )


def test_connect_points_but_no_edge(two_floors):
    with pytest.raises(
        jps.SimulationError,
        match=f"is not an edge of region {two_floors.ground_floor}",
    ):
        two_floors.surface.connect_regions(
            from_region=two_floors.ground_floor,
            from_edge=((0, 0), (5, 5)),  # diagonal
            to_region=two_floors.upper_floor,
            to_edge=two_floors.upper_conn_edge,
        )


def test_connect_too_steep_stairs(walkable_surface):
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (5, 5)), height=0.0
    )
    id_1 = walkable_surface.add_region(
        exterior=rectangle((6, 0), (8, 5)), height=3.0
    )
    with pytest.raises(jps.SimulationError, match="too steep"):
        walkable_surface.connect_regions(
            from_region=id_0,
            from_edge=((5, 0), (5, 5)),
            to_region=id_1,
            to_edge=((6, 0), (6, 5)),
        )


def test_connect_not_planar(walkable_surface):
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (5, 5)), height=0.0
    )
    id_1 = walkable_surface.add_region(
        exterior=rectangle((10, 10), (15, 15)), height=3.0
    )
    # connector edges not parallel in 2D --> not planar
    with pytest.raises(jps.SimulationError, match="not planar"):
        walkable_surface.connect_regions(
            from_region=id_0,
            from_edge=((5, 0), (5, 5)),
            to_region=id_1,
            to_edge=((10, 10), (15, 10)),
        )


def test_connector_not_simple_in_2d(walkable_surface):
    id_0 = walkable_surface.add_region(
        exterior=rectangle((0, 0), (4, 4)), height=0.0
    )
    id_1 = walkable_surface.add_region(
        exterior=rectangle((4, 6), (6, 8)), height=0.0
    )
    expected_error = "Connector is no simple polygon"
    # edges collinear
    with pytest.raises(jps.SimulationError, match=expected_error):
        walkable_surface.connect_regions(
            from_region=id_0,
            from_edge=((4, 0), (4, 4)),
            to_region=id_1,
            to_edge=((4, 6), (4, 8)),
        )
    id_2 = walkable_surface.add_region(
        exterior=rectangle((2, 2), (6, 3)), height=1.0
    )
    id_3 = walkable_surface.add_region(
        exterior=rectangle((4, 1), (6, 3)), height=2.0
    )
    # edges cross in xy
    with pytest.raises(jps.SimulationError, match=expected_error):
        walkable_surface.connect_regions(
            from_region=id_0,
            from_edge=((4, 0), (4, 4)),
            to_region=id_2,
            to_edge=((2, 2), (6, 2)),
        )
    # edges overlap in xy
    with pytest.raises(jps.SimulationError, match=expected_error):
        walkable_surface.connect_regions(
            from_region=id_0,
            from_edge=((4, 0), (4, 4)),
            to_region=id_3,
            to_edge=((4, 1), (4, 3)),
        )


def test_connect_to_connector(two_floors):
    stairs = two_floors.add_stairs()
    # from connector
    with pytest.raises(
        jps.SimulationError, match=f"fromRegion {stairs} is not connectable"
    ):
        two_floors.surface.connect_regions(
            from_region=stairs,
            from_edge=((5, 0), (10, 0)),
            to_region=two_floors.ground_floor,
            to_edge=((0, 0), (0, 5)),
        )
    # to connector
    with pytest.raises(
        jps.SimulationError, match=f"toRegion {stairs} is not connectable"
    ):
        two_floors.surface.connect_regions(
            from_region=two_floors.ground_floor,
            from_edge=((0, 0), (0, 5)),
            to_region=stairs,
            to_edge=((5, 0), (10, 0)),
        )


########### Error cases: Create Geometry ###########
def test_empty_mesh(walkable_surface):
    with pytest.raises(jps.SimulationError, match="No Geometry"):
        walkable_surface.create_geometry()


def test_unconnected_geometry(walkable_surface):
    polygon = rectangle((0, 0), (1, 1))
    walkable_surface.add_region(exterior=polygon, height=0.0)
    walkable_surface.add_region(exterior=polygon, height=3.0)
    with pytest.raises(jps.SimulationError, match="is not connected"):
        walkable_surface.create_geometry()


def test_stairs_through_other_floor(walkable_surface):
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


def test_edge_used_by_several_connectors(two_floors):
    two_floors.add_stairs()
    lower_floor = two_floors.surface.add_region(
        exterior=rectangle((10, 0), (15, 5)), height=-3.0
    )
    connector2 = two_floors.surface.connect_regions(
        from_region=two_floors.ground_floor,
        from_edge=two_floors.ground_conn_edge,
        to_region=lower_floor,
        to_edge=two_floors.upper_conn_edge,
    )
    with pytest.raises(
        jps.SimulationError,
        match=f"Region {connector2} does not fit to a walkable surface",
    ):
        two_floors.surface.create_geometry()


def test_impossible_stairs(two_floors):
    floor3 = two_floors.surface.add_region(
        exterior=rectangle((0, 0), (5, 5)), height=3.0
    )
    # stairs cannot be entered or left along the surface
    stairs = two_floors.surface.connect_regions(
        from_region=two_floors.ground_floor,
        from_edge=two_floors.ground_conn_edge,
        to_region=floor3,
        to_edge=((0, 0), (0, 5)),
    )
    with pytest.raises(
        jps.SimulationError,
        match=f"Region {stairs} does not fit to a walkable surface",
    ):
        two_floors.surface.create_geometry()


########### Correct cases ###########
def test_region_ids(two_floors):
    stairs = two_floors.add_stairs()
    assert [two_floors.ground_floor, two_floors.upper_floor, stairs] == [
        0,
        1,
        2,
    ]
    assert two_floors.surface.create_geometry().region_count() == 3


def test_add_region_from_shapely_polygon(walkable_surface):
    floor = rectangle((0, 0), (10, 10))
    ground_floor = shapely.Polygon(
        floor, holes=[[(2, 2), (6, 2), (6, 2.2), (6, 3.8), (6, 4), (2, 4)]]
    )
    first_floor = shapely.Polygon(
        floor, holes=[[(2, 4), (2, 3.8), (2, 2.2), (2, 2), (6, 4)]]
    )

    id_0 = walkable_surface.add_region(polygon=ground_floor, height=0.0)
    id_1 = walkable_surface.add_region(polygon=first_floor, height=3.0)
    # connecting hole edges only works if the holes were converted
    walkable_surface.connect_regions(
        from_region=id_0,
        from_edge=((6, 2.2), (6, 3.8)),
        to_region=id_1,
        to_edge=((2, 2.2), (2, 3.8)),
    )
    assert walkable_surface.create_geometry().region_count() == 3


def test_striped_floor(walkable_surface):
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
