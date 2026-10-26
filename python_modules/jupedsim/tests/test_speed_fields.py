# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import numpy as np
import pytest


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


def time_at(sim, stage, room, x, y):
    grid = sim.travel_times(stage)[room]
    return grid.values[cell_of(grid, x, y)]


def test_default_speed_fields_cover_the_walkable_cells(corridor):
    sim, room, left, _ = corridor
    fields = sim.speed_fields()
    assert list(fields) == [room]
    grid = fields[room]
    times = sim.travel_times(left)[room]

    assert grid.values.shape == (grid.height, grid.width)
    assert (grid.origin, grid.extent) == (times.origin, times.extent)
    assert np.array_equal(grid.values > 0, np.isfinite(times.values))
    assert repr(grid).startswith("SpeedGrid(origin=")

    x, y = grid.cell_centres
    assert x.shape == y.shape == grid.values.shape
    row, col = cell_of(grid, 3.3, 1.7)
    assert abs(x[row, col] - 3.3) <= grid.cell_size / 2
    assert abs(y[row, col] - 1.7) <= grid.cell_size / 2

    # A stage without its own speed field uses the default.
    assert np.array_equal(sim.speed_fields(left)[room].values, grid.values)


def test_slowing_the_default_slows_every_stage(corridor):
    sim, room, left, right = corridor
    before = [time_at(sim, s, room, 5, 2) for s in (left, right)]

    sim.set_speed_field(room, sim.speed_fields()[room].values * 0.5)

    after = [time_at(sim, s, room, 5, 2) for s in (left, right)]
    assert after == pytest.approx([2 * t for t in before])


def test_a_stage_speed_field_applies_to_that_stage_only(corridor):
    sim, room, left, right = corridor
    default = sim.speed_fields()[room].values
    left_before = time_at(sim, left, room, 5, 2)
    right_before = time_at(sim, right, room, 5, 2)

    sim.set_speed_field(room, default * 0.5, stage_id=left)

    assert np.array_equal(sim.speed_fields(left)[room].values, default * 0.5)
    assert np.array_equal(sim.speed_fields(right)[room].values, default)
    assert np.array_equal(sim.speed_fields()[room].values, default)
    assert time_at(sim, left, room, 5, 2) == pytest.approx(2 * left_before)
    assert time_at(sim, right, room, 5, 2) == right_before

    sim.set_speed_field(room, None, stage_id=left)
    assert np.array_equal(sim.speed_fields(left)[room].values, default)
    assert time_at(sim, left, room, 5, 2) == pytest.approx(left_before)


def test_smoke_sends_agents_to_the_other_exit(corridor):
    sim, room, left, right = corridor
    exits = sim.add_exit_stage(
        [
            ([(0, 0), (0.5, 0), (0.5, 4), (0, 4)], room),
            ([(9.5, 0), (10, 0), (10, 4), (9.5, 4)], room),
        ]
    )

    def heading_at(x, y):
        grid = sim.travel_times(exits)[room]
        return grid.directions[cell_of(grid, x, y)]

    assert heading_at(4.1, 2)[0] < -0.9, "the left exit is nearer"

    grid = sim.speed_fields(exits)[room]
    x, _ = grid.cell_centres
    smoke = np.where(x < 4.5, 0.1, 1.0)
    sim.set_speed_field(room, grid.values * smoke, stage_id=exits)

    assert heading_at(4.1, 2)[0] > 0.9, "through clear air to the right exit"


def test_invalid_speed_fields_are_rejected(corridor):
    sim, room, left, _ = corridor
    default = sim.speed_fields()[room].values
    assert default.shape[0] != default.shape[1]
    steering = sim.add_direct_steering_stage()

    with pytest.raises(ValueError, match="shape"):
        sim.set_speed_field(room, default.T)
    with pytest.raises(ValueError, match="shape"):
        sim.set_speed_field(room, default.ravel(), stage_id=left)
    with pytest.raises(ValueError, match="cleared"):
        sim.set_speed_field(room, None)

    row, col = np.argwhere(default > 0)[0]
    for bad in [0.0, -1.0, np.nan, np.inf]:
        values = default.copy()
        values[row, col] = bad
        with pytest.raises(jps.SimulationError, match="positive and finite"):
            sim.set_speed_field(room, values)
        with pytest.raises(jps.SimulationError, match="positive and finite"):
            sim.set_speed_field(room, values, stage_id=left)
    assert np.array_equal(sim.speed_fields()[room].values, default)

    with pytest.raises(jps.SimulationError):
        sim.set_speed_field(room + 1, default)
    assert sim.speed_fields(steering) == {}
    with pytest.raises(jps.SimulationError, match="no area"):
        sim.set_speed_field(room, default, stage_id=steering)
    with pytest.raises(jps.SimulationError, match="no area"):
        sim.set_speed_field(room, None, stage_id=steering)
