# SPDX-License-Identifier: LGPL-3.0-or-later
import gc

import jupedsim as jps
import pytest

SQUARE = [(0, 0), (10, 0), (10, 10), (0, 10)]


def _builder():
    builder = jps.GeometryBuilder()
    room = builder.add_region(boundary=SQUARE, height=0.0, name="room")
    return builder, room


def test_a_region_has_no_locations_before_build():
    _, room = _builder()
    with pytest.raises(
        jps.SimulationError,
        match=r"Region 0 \(room\) is not built yet; call build\(\) first",
    ):
        room.at(5, 5)


def test_a_location_outside_names_the_region():
    builder, room = _builder()
    builder.build()
    with pytest.raises(
        jps.SimulationError,
        match=r"Point \(20, 20\) is not in region 0 \(room\)",
    ):
        room.at(20, 20)


def test_handles_of_one_region_are_equal():
    builder, room = _builder()
    geometry = builder.build()
    location = room.at(5, 5)
    for other in (geometry.region(0), geometry.region("room"), location.region):
        assert other == room
        assert hash(other) == hash(room)


def test_regions_of_different_geometries_differ():
    builder, room = _builder()
    builder.build()
    other_builder, other_room = _builder()
    other_builder.build()
    assert room != other_room


def test_an_unknown_region_is_a_key_error():
    builder, _ = _builder()
    geometry = builder.build()
    with pytest.raises(KeyError, match="no region named 'hall'"):
        geometry.region("hall")
    with pytest.raises(KeyError, match="no region with id 1"):
        geometry.region(1)
    with pytest.raises(TypeError):
        geometry.region(True)


def test_a_location_outlives_its_builder_and_geometry():
    def location():
        builder, room = _builder()
        builder.build()
        return room.at(5, 5)

    where = location()
    gc.collect()
    assert where.xy == (5, 5)
    assert where.region.at(1, 1).xy == (1, 1)


def test_a_region_of_another_geometry_is_rejected():
    builder, _ = _builder()
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelTordeux2016(), geometry=builder.build()
    )
    other_builder, other_room = _builder()
    other_builder.build()
    with pytest.raises(
        ValueError, match=r"Region 0 \(room\) belongs to another geometry"
    ):
        sim.add_target(jps.Exit([(SQUARE, other_room)]))


def test_travel_times_are_given_by_region():
    builder, room = _builder()
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelTordeux2016(), geometry=builder.build()
    )
    exit_ = sim.add_target(
        jps.Exit([([(9, 0), (10, 0), (10, 10), (9, 10)], room)])
    )
    assert list(sim.travel_times(exit_)) == [room]
