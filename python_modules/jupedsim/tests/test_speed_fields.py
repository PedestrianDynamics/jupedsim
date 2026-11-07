# SPDX-License-Identifier: LGPL-3.0-or-later
#
# The Python side of speed fields: arrays in and out, None, and the errors raised.
# How speeds change routes is tested in libfloorfield, which stage gets which speed
# field in the C++ simulation tests.
import jupedsim as jps
import numpy as np
import pytest
import shapely
from jupedsim.surface import WalkableSurface


@pytest.fixture
def corridor():
    """A 10 x 4 corridor with an exit stage at either end."""
    surface = jps.WalkableSurface()
    room = surface.add_region(boundary=[(0, 0), (10, 0), (10, 4), (0, 4)])
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
    left = sim.add_exit_stage([([(0, 0), (0.5, 0), (0.5, 4), (0, 4)], room)])
    right = sim.add_exit_stage([([(9.5, 0), (10, 0), (10, 4), (9.5, 4)], room)])
    return sim, room, left, right


def cell_of(grid, x, y):
    return (
        int((y - grid.origin[1]) / grid.cell_size),
        int((x - grid.origin[0]) / grid.cell_size),
    )


def test_a_speed_grid_is_laid_out_like_the_travel_times(corridor):
    sim, room, left, _ = corridor
    fields = sim.default_speed_fields()
    assert list(fields) == [room]
    grid = fields[room]
    times = sim.travel_times(left)[room]

    assert grid.values.shape == (grid.height, grid.width)
    assert (grid.origin, grid.extent) == (times.origin, times.extent)
    assert repr(grid).startswith("SpeedGrid(origin=")

    x, y = grid.cell_centers
    assert x.shape == y.shape == grid.values.shape
    row, col = cell_of(grid, 3.3, 1.7)
    assert abs(x[row, col] - 3.3) <= grid.cell_size / 2
    assert abs(y[row, col] - 1.7) <= grid.cell_size / 2


def test_speed_fields_round_trip(corridor):
    sim, room, left, right = corridor
    default = sim.default_speed_fields()[room].values
    assert sim.custom_speed_fields(left) == {room: None}

    # Any array-like with the grid's shape is taken, a nested list too.
    sim.set_default_speed_field(room, (default * 2.0).tolist())
    assert np.array_equal(
        sim.default_speed_fields()[room].values, default * 2.0
    )

    sim.set_custom_speed_field(left, room, default * 4.0)
    assert np.array_equal(
        sim.custom_speed_fields(left)[room].values, default * 4.0
    )
    assert sim.custom_speed_fields(right)[room] is None

    sim.clear_custom_speed_field(left, room)
    assert sim.custom_speed_fields(left)[room] is None


def test_invalid_speed_fields_are_rejected(corridor):
    sim, room, left, _ = corridor
    default = sim.default_speed_fields()[room].values
    assert default.shape[0] != default.shape[1]
    steering = sim.add_direct_steering_stage()

    with pytest.raises(ValueError, match="shape"):
        sim.set_default_speed_field(room, default.T)
    with pytest.raises(ValueError, match="shape"):
        sim.set_custom_speed_field(left, room, default.ravel())

    # Which values are allowed is libfloorfield's to test; one is enough to see
    # its error reach Python.
    zero = default.copy()
    zero[tuple(np.argwhere(default > 0)[0])] = 0.0
    with pytest.raises(jps.SimulationError, match="at least"):
        sim.set_default_speed_field(room, zero)
    with pytest.raises(jps.SimulationError, match="at least"):
        sim.set_custom_speed_field(left, room, zero)

    with pytest.raises(jps.SimulationError):
        sim.set_default_speed_field(room + 1, default)
    assert sim.custom_speed_fields(steering) == {}
    with pytest.raises(jps.SimulationError, match="no area"):
        sim.set_custom_speed_field(steering, room, default)
    with pytest.raises(jps.SimulationError, match="no area"):
        sim.clear_custom_speed_field(steering, room)


def make_hall():
    surface = WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (10, 0), (10, 4), (0, 4)]),
        name="room",
    )
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    left = sim.add_exit([(0, 0), (0.5, 0), (0.5, 4), (0, 4)], region=room)
    right = sim.add_exit([(9.5, 0), (10, 0), (10, 4), (9.5, 4)], region=room)
    return sim, surface, room, left, right


@pytest.fixture
def hall():
    """The corridor built from a WalkableSurface, with an exit at either end."""
    sim, _, room, left, right = make_hall()
    return sim, room, left, right


def test_handle_speed_grids_are_laid_out_like_the_travel_times(hall):
    sim, room, left, _ = hall
    fields = sim.default_speed_fields()
    assert list(fields) == [room]
    times = sim.travel_times(left)
    assert list(times) == [room]
    assert (fields[room].origin, fields[room].extent) == (
        times[room].origin,
        times[room].extent,
    )


def test_speed_fields_round_trip_with_handles(hall):
    sim, room, left, right = hall
    default = sim.default_speed_fields()[room].values
    assert sim.custom_speed_fields(left) == {room: None}

    sim.set_default_speed_field(room, (default * 2.0).tolist())
    assert np.array_equal(
        sim.default_speed_fields()[room].values, default * 2.0
    )

    sim.set_custom_speed_field(left, room, default * 4.0)
    assert np.array_equal(
        sim.custom_speed_fields(left)[room].values, default * 4.0
    )
    assert sim.custom_speed_fields(right)[room] is None

    sim.clear_custom_speed_field(left, room)
    assert sim.custom_speed_fields(left)[room] is None


def test_invalid_speed_fields_are_rejected_with_handles(hall):
    sim, room, left, _ = hall
    default = sim.default_speed_fields()[room].values

    with pytest.raises(ValueError, match="shape"):
        sim.set_default_speed_field(room, default.T)
    with pytest.raises(ValueError, match="shape"):
        sim.set_custom_speed_field(left, room, default.ravel())

    _, _, foreign, other_exit, _ = make_hall()
    with pytest.raises(ValueError, match="belongs to another surface"):
        sim.set_default_speed_field(foreign, default)
    with pytest.raises(TypeError, match="region must be a Region handle"):
        sim.set_custom_speed_field(left, "room", default)

    message = "stage belongs to another simulation"
    with pytest.raises(ValueError, match=message):
        sim.travel_times(other_exit)
    with pytest.raises(ValueError, match=message):
        sim.custom_speed_fields(other_exit)
    with pytest.raises(ValueError, match=message):
        sim.clear_custom_speed_field(other_exit, room)


def test_travel_time_matches_grid(hall):
    sim, room, left, right = hall
    for stage in (left, right):
        grid = sim.travel_times(stage)[room]
        for x, y in [(3.3, 1.7), (7.1, 3.2), (0.2, 0.2), (9.8, 2.0)]:
            row, col = cell_of(grid, x, y)
            assert (
                sim.travel_time(stage, room.at(x, y)) == grid.values[row, col]
            )
    assert sim.travel_time(left, room.at(0.2, 2.0)) == 0.0
    assert sim.travel_time(left, room.at(9, 2)) > sim.travel_time(
        left, room.at(5, 2)
    )


def test_travel_time_rejects_bad_arguments(hall):
    sim, room, left, _ = hall
    with pytest.raises(TypeError, match="location must be a Location"):
        sim.travel_time(left, (3, 2))
    with pytest.raises(TypeError, match="not a stage"):
        sim.travel_time(1, room.at(3, 2))
    _, other_surface, _, other_exit, _ = make_hall()
    with pytest.raises(ValueError, match="stage belongs to another simulation"):
        sim.travel_time(other_exit, room.at(3, 2))
    with pytest.raises(ValueError, match="belongs to another surface"):
        sim.travel_time(left, other_surface.regions[0].at(3, 2))
