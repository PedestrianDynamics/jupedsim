# SPDX-License-Identifier: LGPL-3.0-or-later
import gc
import re
import weakref

import jupedsim as jps
import numpy as np
import pytest
import shapely


@pytest.fixture
def square_room_5x5():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)]
        )
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    return simulation, room


@pytest.fixture
def square_room_5x5_with_obstacle():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
            [[(-0.5, -0.5), (-0.5, 0.5), (0.5, 0.5), (0.5, -0.5)]],
        )
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    return simulation, room


def test_exception_on_empty_polygon_in_exit_stage(square_room_5x5):
    sim, room = square_room_5x5
    with pytest.raises(ValueError, match="must be simple and without holes"):
        sim.add_exit([([], room)])


def test_exception_on_exit_without_areas(square_room_5x5):
    sim, _ = square_room_5x5
    with pytest.raises(ValueError, match="no areas given"):
        sim.add_exit([])


def test_exception_on_exit_polygon_with_hole(square_room_5x5):
    sim, room = square_room_5x5
    exit_with_hole = shapely.Polygon(
        [(-2, -2), (2, -2), (2, 2), (-2, 2)],
        [[(-1, -1), (1, -1), (1, 1), (-1, 1)]],
    )
    with pytest.raises(
        ValueError, match="^exit polygons must be simple and without holes$"
    ):
        sim.add_exit([(exit_with_hole, room)])


def test_can_count_the_agents_targeting_a_stage():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    polygon = shapely.union(
        shapely.Polygon([(-10, 2.5), (-10, -2.5), (10, -2.5), (10, 2.5)]),
        shapely.Polygon([(-2.5, 2.5), (-2.5, -10), (2.5, -10), (2.5, 2.5)]),
    )

    surface = jps.WalkableSurface()
    room = surface.add_region(polygon=polygon)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )

    exit = simulation.add_exit(
        [([(-2.5, -9.5), (-2.5, -10), (2.5, -10), (2.5, -9.5)], room)]
    )

    waypoint = simulation.add_waypoint(
        [(jps.circle_to_polygon(center=(9.5, 0), radius=1), room)]
    )

    exit_route = jps.Route([exit])
    entry_route = jps.Route([waypoint, exit])

    assert len(simulation.agents_targeting(exit)) == 0
    assert len(simulation.agents_targeting(waypoint)) == 0

    agent = simulation.add_agent(
        room.at(-9.5, 0),
        route=exit_route,
        state=jps.CollisionFreeSpeedModelState(),
    )

    assert len(simulation.agents_targeting(exit)) == 1
    assert len(simulation.agents_targeting(waypoint)) == 0

    simulation.iterate()

    assert len(simulation.agents_targeting(exit)) == 1
    assert len(simulation.agents_targeting(waypoint)) == 0

    agent.follow(entry_route)

    assert len(simulation.agents_targeting(exit)) == 0
    assert len(simulation.agents_targeting(waypoint)) == 1

    simulation.iterate()

    assert len(simulation.agents_targeting(exit)) == 0
    assert len(simulation.agents_targeting(waypoint)) == 1


@pytest.mark.parametrize(
    "waypoint_position",
    [(-2.5, -2.5), (-2, -2), (2, 2), (2.5, 2.5), (-0.5, -0.5), (0.5, 0.5)],
)
def test_can_add_waypoint(square_room_5x5_with_obstacle, waypoint_position):
    simulation, room = square_room_5x5_with_obstacle
    waypoint = simulation.add_waypoint(
        [(jps.circle_to_polygon(center=waypoint_position, radius=1), room)]
    )

    assert type(waypoint) is jps.Waypoint


def test_a_waypoint_given_by_its_area_is_reached(square_room_5x5):
    sim, room = square_room_5x5
    waypoint = sim.add_waypoint(
        [([(1, -0.5), (2, -0.5), (2, 0.5), (1, 0.5)], room)]
    )
    exit = sim.add_exit([([(-2.5, -1), (-2, -1), (-2, 1), (-2.5, 1)], room)])
    agent = sim.add_agent(
        room.at(1.5, 0),
        route=jps.Route([waypoint, exit]),
        state=jps.CollisionFreeSpeedModelState(),
    )
    # It starts inside the waypoint, so the first step moves it on to the exit.
    sim.iterate()
    assert agent.stage == exit


def test_can_not_add_waypoint_outside_geometry(square_room_5x5):
    simulation, room = square_room_5x5

    with pytest.raises(
        ValueError, match=f"area does not overlap {re.escape(room.label)}"
    ):
        simulation.add_waypoint(
            [(jps.circle_to_polygon(center=(10, 10), radius=1), room)]
        )


def test_can_not_add_exit_completely_outside_geometry(square_room_5x5):
    simulation, room = square_room_5x5

    with pytest.raises(
        ValueError, match=f"area does not overlap {re.escape(room.label)}"
    ):
        simulation.add_exit(
            [([(-10, -10), (-8, -10), (-8, -8), (-10, -8)], room)]
        )


@pytest.mark.parametrize(
    "polygon",
    [
        [(-3, -3), (-3, -1), (-1, -1), (-1, -3)],
        [(-4, -4), (-4, -2), (-2, -2), (-2, -4)],
    ],
)
def test_can_add_exit_partly_outside_geometry(square_room_5x5, polygon):
    sim, room = square_room_5x5
    sim.add_exit([(polygon, room)])


def test_an_agent_standing_at_the_edge_of_an_exit_leaves():
    # The exit's edge lies off the routing grid.
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    exit = simulation.add_exit(
        [([(8.05, 4), (9.5, 4), (9.5, 6), (8.05, 6)], room)]
    )
    route = jps.Route([exit])
    simulation.add_agent(
        room.at(8.02, 5),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )
    expected_iterations_to_finish = 10
    simulation.iterate(expected_iterations_to_finish)
    assert simulation.agent_count() == 0, (
        f"Expected agent to exit simulation within {expected_iterations_to_finish} iterations."
    )


def test_stages_are_handles_of_their_kind(square_room_5x5):
    sim, room = square_room_5x5
    waypoint = sim.add_waypoint(
        [(jps.circle_to_polygon(center=(0, 0), radius=1), room)]
    )
    exit = sim.add_exit([([(2, -1), (2.5, -1), (2.5, 1), (2, 1)], room)])

    assert type(waypoint) is jps.Waypoint
    assert type(exit) is jps.Exit
    assert jps.Waypoint.__module__ == "jupedsim.stages"
    assert len(sim.agents_targeting(waypoint)) == 0
    assert len(sim.agents_targeting(exit)) == 0


def test_a_stage_keeps_its_simulation_alive():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)]
        )
    )
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
    stage = sim.add_waypoint(
        [(jps.circle_to_polygon(center=(0, 0), radius=1), room)]
    )
    native = weakref.ref(sim._obj)
    del sim
    gc.collect()
    assert native() is not None
    assert len(stage._simulation.agents_targeting(stage)) == 0


@pytest.mark.parametrize("bad_id", [-1, 2**64])
def test_stage_lookup_rejects_unknown_ids(square_room_5x5, bad_id):
    sim, _ = square_room_5x5
    with pytest.raises(KeyError):
        sim.stage(id=bad_id)


@pytest.mark.parametrize("bad_id", [1.5, "3", True, 1.0])
def test_stage_lookup_rejects_ids_that_are_not_ints(square_room_5x5, bad_id):
    sim, _ = square_room_5x5
    with pytest.raises(TypeError, match="id must be an int"):
        sim.stage(id=bad_id)


def test_travel_times_to_an_exit(square_room_5x5):
    sim, room = square_room_5x5
    exit = sim.add_exit(
        [([(-2.5, -2.5), (-2, -2.5), (-2, 2.5), (-2.5, 2.5)], room)]
    )

    grid = sim.travel_times(exit)[room]
    assert grid.values.shape == (grid.height, grid.width)
    x_min, x_max, y_min, y_max = grid.extent
    assert (x_min, y_min) == grid.origin
    assert x_max == pytest.approx(x_min + grid.width * grid.cell_size)
    assert y_max == pytest.approx(y_min + grid.height * grid.cell_size)
    assert x_min <= -2.5 and x_max >= 2.5 and y_min <= -2.5 and y_max >= 2.5

    def time_at(x, y):
        col = int((x - x_min) / grid.cell_size)
        row = int((y - y_min) / grid.cell_size)
        return grid.values[row, col]

    assert time_at(-2.25, 0) == 0.0
    assert 0.0 < time_at(0, 0) < time_at(2.25, 0)

    directions = grid.directions
    assert directions.shape == (grid.height, grid.width, 2)
    walkable = np.isfinite(grid.values)
    assert np.isnan(directions[~walkable]).all()
    lengths = np.linalg.norm(directions[walkable], axis=1)
    assert np.all((np.abs(lengths - 1.0) < 1e-9) | (lengths == 0.0))
    row = int((0 - y_min) / grid.cell_size)
    col = int((1.0 - x_min) / grid.cell_size)
    assert directions[row, col, 0] < -0.9, "towards the exit on the left"


def test_travel_times_follow_the_floorfield_options():
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)]
        )
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface,
        floorfield_options=jps.FloorfieldOptions(cell_size=0.1),
    )
    exit = sim.add_exit(
        [([(-2.5, -2.5), (-2, -2.5), (-2, 2.5), (-2.5, 2.5)], room)]
    )
    assert sim.travel_times(exit)[room].cell_size == 0.1


def test_travel_times_of_every_region(square_room_5x5):
    sim, room = square_room_5x5
    exit = sim.add_exit(
        [([(-2.5, -2.5), (-2, -2.5), (-2, 2.5), (-2.5, 2.5)], room)]
    )

    grids = sim.travel_times(exit)

    assert list(grids) == [room]
    assert isinstance(grids[room], jps.TravelTimeGrid)


def test_stages_are_listed_in_creation_order(square_room_5x5):
    sim, room = square_room_5x5
    area = [(-2.5, -2.5), (-2, -2.5), (-2, 2.5), (-2.5, 2.5)]
    exit = sim.add_exit([(area, room)])
    waypoint = sim.add_waypoint([(area, room)])

    assert sim.stages == (exit, waypoint)
    assert [stage.id for stage in sim.stages] == sorted([exit.id, waypoint.id])
