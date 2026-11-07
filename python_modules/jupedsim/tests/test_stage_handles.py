# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
from jupedsim.surface import WalkableSurface

FOOTPRINT = [(0, 0), (20, 0), (20, 10), (0, 10)]
DOOR = [(1, 1), (2, 1), (2, 2), (1, 2)]
DOOR_FAR = [(13, 6.5), (14, 6.5), (14, 7.5), (13, 7.5)]


def make_sim():
    surface = WalkableSurface()
    hole = [(12, 6), (18, 6), (18, 8), (12, 8)]
    floor0 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[hole]),
        height=0.0,
        name="floor0",
    )
    floor1 = surface.add_region(
        polygon=shapely.Polygon(FOOTPRINT, holes=[hole]),
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
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
    return sim, surface, floor0, floor1


def test_region_is_required():
    sim, *_ = make_sim()
    with pytest.raises(
        TypeError,
        match=r"region= is required, e.g. "
        r"sim.add_waypoint\(area, region=floor0\)",
    ):
        sim.add_waypoint(DOOR)
    with pytest.raises(
        TypeError,
        match=r"region= is required, e.g. sim.add_exit\(area, region=floor0\)",
    ):
        sim.add_exit(DOOR)


def test_region_must_be_a_handle():
    sim, *_ = make_sim()
    with pytest.raises(TypeError, match=r"region must be a Region handle"):
        sim.add_waypoint(DOOR, region=1)


def test_radius_applies_to_points_only():
    sim, _, floor0, _ = make_sim()
    with pytest.raises(
        ValueError, match=r"radius applies to point waypoints only"
    ):
        sim.add_waypoint(DOOR, radius=0.5, region=floor0)


def test_radius_must_be_positive():
    sim, _, floor0, _ = make_sim()
    with pytest.raises(ValueError, match=r"radius must be > 0"):
        sim.add_waypoint((5, 5), radius=0, region=floor0)


def test_polygon_with_hole_rejected():
    sim, _, floor0, _ = make_sim()
    holed = shapely.Polygon(
        [(1, 1), (6, 1), (6, 6), (1, 6)],
        holes=[[(2, 2), (3, 2), (3, 3), (2, 3)]],
    )
    with pytest.raises(
        ValueError, match=r"waypoint polygons must be simple and without holes"
    ):
        sim.add_waypoint(holed, region=floor0)


def test_area_must_overlap_region():
    sim, _, _, floor1 = make_sim()
    with pytest.raises(ValueError, match=r"area does not overlap floor1"):
        sim.add_waypoint(DOOR_FAR, region=floor1, name="far")


def test_region_of_another_surface_rejected():
    sim, *_ = make_sim()
    _, _, other0, _ = make_sim()
    with pytest.raises(ValueError, match=r"belongs to another surface"):
        sim.add_waypoint(DOOR, region=other0)


def test_region_added_after_simulation_rejected():
    sim, surface, *_ = make_sim()
    late = surface.add_region(
        polygon=shapely.Polygon([(30, 0), (31, 0), (31, 1), (30, 1)]),
        name="late",
    )
    with pytest.raises(
        ValueError, match=r"late is not part of this simulation's geometry"
    ):
        sim.add_waypoint([(30, 0), (31, 0), (31, 1), (30, 1)], region=late)


def test_handle_identity_and_repr():
    sim, _, floor0, _ = make_sim()
    exit1 = sim.add_exit(DOOR, region=floor0, name="exit1")
    wp = sim.add_waypoint((5, 5), radius=0.5, region=floor0)
    assert repr(exit1) == 'Exit(id=%d, name="exit1")' % exit1.id
    assert repr(wp) == "Waypoint(id=%d)" % wp.id
    assert wp.name is None
    assert exit1 != wp
    assert exit1 == sim.stage("exit1")
    assert hash(exit1) == hash(sim.stage(id=exit1.id))
    other, _, other0, _ = make_sim()
    other_exit = other.add_exit(DOOR, region=other0)
    assert other_exit != exit1


def test_stage_lookup():
    sim, _, floor0, _ = make_sim()
    wp = sim.add_waypoint((5, 5), radius=0.5, region=floor0, name="wp")
    exit1 = sim.add_exit(DOOR, region=floor0, name="exit1")
    assert sim.stage("exit1") == exit1
    assert sim.stage(id=exit1.id) == exit1
    assert sim.stages == (wp, exit1)
    with pytest.raises(KeyError, match=r"no stage named nope; stages are wp"):
        sim.stage("nope")
    with pytest.raises(KeyError, match=r"no stage with id 99"):
        sim.stage(id=99)
    with pytest.raises(TypeError):
        sim.stage()


class _Recorder:
    def __init__(self, obj):
        self._obj = obj
        self.calls = []

    def add_waypoint_stage(self, areas):
        self.calls.append(areas)
        return self._obj.add_waypoint_stage(areas)

    def add_exit_stage(self, areas):
        self.calls.append(areas)
        return self._obj.add_exit_stage(areas)

    def __getattr__(self, name):
        return getattr(self._obj, name)


def test_multi_area_stage():
    sim, _, floor0, _ = make_sim()
    recorder = sim._obj = _Recorder(sim._obj)
    door_b = [(10, 1), (11, 1), (11, 2), (10, 2)]
    doors = sim.add_exit([(DOOR, floor0), (door_b, floor0)], name="doors")
    assert sim.stage("doors") == doors
    assert len(sim.stages) == 1
    (areas,) = recorder.calls
    assert [r for _, r in areas] == [floor0.id, floor0.id]
    assert [p.bounds for p, _ in areas] == [
        shapely.Polygon(DOOR).bounds,
        shapely.Polygon(door_b).bounds,
    ]


def test_pairs_with_stage_across_regions():
    sim, _, floor0, floor1 = make_sim()
    recorder = sim._obj = _Recorder(sim._obj)
    doors = sim.add_waypoint([(DOOR, floor0), (DOOR, floor1)], name="doors")
    assert doors.name == "doors"
    (areas,) = recorder.calls
    assert [r for _, r in areas] == [floor0.id, floor1.id]


def test_point_waypoint_is_a_disc_crossing_the_boundary():
    sim, _, floor0, _ = make_sim()
    recorder = sim._obj = _Recorder(sim._obj)
    sim.add_waypoint((0.1, 5), radius=0.5, region=floor0)
    ((polygon, region_id),) = recorder.calls[0]
    assert region_id == floor0.id
    assert not shapely.Polygon(FOOTPRINT).contains(polygon)
    assert polygon.centroid.x == pytest.approx(0.1, abs=1e-6)
    assert polygon.centroid.y == pytest.approx(5, abs=1e-6)


def test_waypoint_on_connector():
    sim, surface, *_ = make_sim()
    staircase = surface.region("staircase")
    wp = sim.add_waypoint((15, 7), radius=0.3, region=staircase, name="mid")
    assert sim.stage("mid") == wp


def test_area_outside_connector_rejected():
    sim, surface, *_ = make_sim()
    staircase = surface.region("staircase")
    with pytest.raises(ValueError, match=r"area does not overlap staircase"):
        sim.add_waypoint((2, 2), radius=0.3, region=staircase)
    assert sim.stages == ()


def test_region_rejected_without_python_surface():
    surface = WalkableSurface()
    region = surface.add_region(polygon=shapely.Polygon(FOOTPRINT))
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface._obj
    )
    with pytest.raises(ValueError, match=r"belongs to another surface"):
        sim.add_waypoint(DOOR, region=region)


def test_exit_rejects_point():
    sim, _, floor0, _ = make_sim()
    with pytest.raises(
        ValueError, match=r"add_exit takes an area, not a point"
    ):
        sim.add_exit((5, 5), region=floor0)


def test_empty_pair_list_rejected():
    sim, *_ = make_sim()
    with pytest.raises(ValueError, match=r"no areas given"):
        sim.add_waypoint([])
