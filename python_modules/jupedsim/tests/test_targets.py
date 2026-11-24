# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import numpy as np
import pytest
import shapely

LEFT_STRIP = [(-2.5, -2.5), (-2, -2.5), (-2, 2.5), (-2.5, 2.5)]


def _square_room_5x5():
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
        height=0.0,
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface.build()
    )
    return simulation, room


@pytest.fixture
def square_room_5x5():
    return _square_room_5x5()


@pytest.fixture
def square_room_5x5_with_obstacle():
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        polygon=shapely.Polygon(
            [(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
            [[(-0.5, -0.5), (-0.5, 0.5), (0.5, 0.5), (0.5, -0.5)]],
        ),
        height=0.0,
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface.build()
    )
    return simulation, room


def test_exception_on_empty_polygon_in_exit(square_room_5x5):
    sim, room = square_room_5x5
    with pytest.raises(Exception, match=r"Polygon must have at least 3 points"):
        sim.add_target(jps.Exit([([], room)]))


def test_exception_on_exit_without_areas(square_room_5x5):
    sim, _ = square_room_5x5
    with pytest.raises(
        jps.SimulationError, match="A target needs at least one area"
    ):
        sim.add_target(jps.Exit([]))


def test_exception_on_exit_polygon_with_hole(square_room_5x5):
    sim, room = square_room_5x5
    exit_with_hole = shapely.Polygon(
        [(-2, -2), (2, -2), (2, 2), (-2, 2)],
        [[(-1, -1), (1, -1), (1, 1), (-1, 1)]],
    )
    with pytest.raises(ValueError, match="Polygon must not have holes"):
        sim.add_target(jps.Exit([(exit_with_hole, room)]))


@pytest.mark.parametrize(
    "waypoint_position",
    [(-2.5, -2.5), (-2, -2), (2, 2), (2.5, 2.5), (-0.5, -0.5), (0.5, 0.5)],
)
def test_can_add_waypoint(square_room_5x5_with_obstacle, waypoint_position):
    simulation, room = square_room_5x5_with_obstacle
    simulation.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=waypoint_position, radius=1), room)]
        )
    )


def test_a_waypoint_given_by_its_area_is_reached(square_room_5x5):
    sim, room = square_room_5x5
    waypoint = sim.add_target(
        jps.Waypoint([([(1, -0.5), (2, -0.5), (2, 0.5), (1, 0.5)], room)])
    )
    exit_ = sim.add_target(
        jps.Exit([([(-2.5, -1), (-2, -1), (-2, 1), (-2.5, 1)], room)])
    )
    route = sim.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, waypoint),
            jps.FixedTransition(waypoint, exit_),
        ]
    )
    agent = sim.add_agent(room.at(1.5, 0), route=route)
    # It starts inside the waypoint, so the first step moves it on to the exit.
    sim.iterate()
    assert agent.current_target == exit_


def test_can_not_add_waypoint_outside_geometry(square_room_5x5):
    simulation, room = square_room_5x5

    with pytest.raises(
        jps.SimulationError, match=f"Area does not intersect region {room.id}"
    ):
        simulation.add_target(
            jps.Waypoint(
                [(jps.circle_to_polygon(center=(10, 10), radius=1), room)]
            )
        )


def test_can_not_add_exit_completely_outside_geometry(square_room_5x5):
    simulation, room = square_room_5x5

    with pytest.raises(
        jps.SimulationError, match=f"Area does not intersect region {room.id}"
    ):
        simulation.add_target(
            jps.Exit([([(-10, -10), (-8, -10), (-8, -8), (-10, -8)], room)])
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
    sim.add_target(jps.Exit([(polygon, room)]))


def test_an_agent_standing_at_the_edge_of_an_exit_leaves():
    # The exit's edge lies off the routing grid.
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(0, 0), (10, 0), (10, 10), (0, 10)], height=0.0
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface.build()
    )
    exit_ = simulation.add_target(
        jps.Exit([([(8.05, 4), (9.5, 4), (9.5, 6), (8.05, 6)], room)])
    )
    route = simulation.add_route(
        [jps.FixedTransition(jps.AGENT_LOCATION, exit_)]
    )
    simulation.add_agent(room.at(8.02, 5), route=route)
    expected_iterations_to_finish = 10
    simulation.iterate(expected_iterations_to_finish)
    assert simulation.agent_count() == 0, (
        f"Expected agent to exit simulation within {expected_iterations_to_finish} iterations."
    )


def test_travel_times_to_an_exit(square_room_5x5):
    sim, room = square_room_5x5
    exit_ = sim.add_target(jps.Exit([(LEFT_STRIP, room)]))

    grid = sim.travel_times(exit_)[room]

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
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(-2.5, -2.5), (2.5, -2.5), (2.5, 2.5), (-2.5, 2.5)],
        height=0.0,
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface.build(),
        floorfield_options=jps.FloorfieldOptions(cell_size=0.1),
    )
    exit_ = sim.add_target(jps.Exit([(LEFT_STRIP, room)]))
    assert sim.travel_times(exit_)[room].cell_size == 0.1


def test_travel_times_of_every_region(square_room_5x5):
    sim, room = square_room_5x5
    exit_ = sim.add_target(jps.Exit([(LEFT_STRIP, room)]))

    grids = sim.travel_times(exit_)

    assert list(grids) == [room]
    assert isinstance(grids[room], jps.TravelTimeGrid)


def test_travel_time_from_a_location(square_room_5x5):
    sim, room = square_room_5x5
    exit_ = sim.add_target(jps.Exit([(LEFT_STRIP, room)]))
    inside = room.at(-2.25, 0)
    far = room.at(2.25, 0)
    assert sim.travel_time(exit_, inside) == 0.0
    assert sim.travel_time(exit_, far) > 0.0


def test_a_target_describes_its_input(square_room_5x5):
    sim, room = square_room_5x5
    description = jps.Exit([(LEFT_STRIP, room)])
    exit_ = sim.add_target(description, name="exit")
    assert exit_.description is description
    assert exit_.description.areas == ((LEFT_STRIP, room),)
    assert repr(exit_) == f"RouteTarget(id={exit_.id}, name='exit')"
    with pytest.raises(AttributeError):
        description.areas = ()


def test_targets_are_looked_up_by_name_or_id(square_room_5x5):
    sim, room = square_room_5x5
    exit_ = sim.add_target(jps.Exit([(LEFT_STRIP, room)]), name="exit")
    waypoint = sim.add_target(jps.Waypoint([(LEFT_STRIP, room)]))
    assert sim.targets() == [exit_, waypoint]
    assert sim.target("exit") == exit_
    assert sim.target(waypoint.id) == waypoint
    with pytest.raises(KeyError, match="no target named 'exit9'"):
        sim.target("exit9")
    with pytest.raises(jps.SimulationError, match="'exit' is already used"):
        sim.add_target(jps.Exit([(LEFT_STRIP, room)]), name="exit")


def test_a_target_of_another_simulation_is_rejected(square_room_5x5):
    sim, room = square_room_5x5
    other, other_room = _square_room_5x5()
    foreign = other.add_target(jps.Exit([(LEFT_STRIP, other_room)]))
    with pytest.raises(ValueError, match="belongs to another simulation"):
        sim.travel_times(foreign)
