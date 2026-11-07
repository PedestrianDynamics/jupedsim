# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest
import shapely


def stay_at(sim, room, centre):
    """A route to a waypoint at ``centre``, where the agent stays."""
    wp = sim.add_waypoint(centre, radius=0.5, region=room)
    return jps.Route([wp, jps.STAY])


@pytest.fixture
def warp_driver_corridor():
    """20m x 4m corridor with WarpDriver model."""
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (20, 0), (20, 4), (0, 4)])
    )
    simulation = jps.Simulation(
        model=jps.WarpDriverModel(), geometry=surface, dt=0.01
    )
    return simulation, room


def test_default_model_construction():
    """Test that the WarpDriver model constructs with its C++ defaults.

    The model-level parameters ``time_horizon`` and ``step_size`` are
    consumed by the constructor and are no longer part of the per-agent
    state; they default to 2.0 and 0.5 respectively.
    """
    jps.WarpDriverModel()
    jps.WarpDriverModel(time_horizon=2.0, step_size=0.5)


def test_default_state_construction():
    """Test that the WarpDriver agent state exposes the C++ defaults."""
    state = jps.WarpDriverModelState()
    assert math.isclose(state.radius, 0.15)
    assert math.isclose(state.desired_speed, 1.2)


def test_simulation_runs(warp_driver_corridor):
    """Test that a basic simulation runs without errors."""
    sim, room = warp_driver_corridor
    exit_area = shapely.Polygon([(19, 0), (20, 0), (20, 4), (19, 4)])
    exit = sim.add_exit([(exit_area, room)])
    route = jps.Route([exit])

    aid = sim.add_agent(
        room.at(2, 2),
        route=route,
        state=jps.WarpDriverModelState(desired_speed=1.2, radius=0.15),
    ).id

    initial_x = sim.agent(aid).position.x
    for _ in range(50):
        sim.iterate()
    assert sim.agent_count() == 1
    assert sim.agent(aid).position.x > initial_x, (
        "Agent should have moved forward"
    )


def test_single_agent_straight_path(warp_driver_corridor):
    """Single agent with no neighbors follows straight path towards target."""
    sim, room = warp_driver_corridor

    aid = sim.add_agent(
        room.at(2, 2),
        route=stay_at(sim, room, (10, 2)),
        state=jps.WarpDriverModelState(desired_speed=1.0, radius=0.15),
    ).id

    for _ in range(100):
        sim.iterate()

    pos = sim.agent(aid).position.xy
    # Agent should have moved towards x=10, staying near y=2
    assert pos[0] > 2.5, f"Agent should have moved forward, got x={pos[0]}"
    assert abs(pos[1] - 2.0) < 0.5, (
        f"Agent should stay near y=2, got y={pos[1]}"
    )


def test_two_agents_head_on_avoid(warp_driver_corridor):
    """Two head-on agents should avoid each other and both reach their goals."""
    sim, room = warp_driver_corridor
    exit_left = shapely.Polygon([(0, 0), (1, 0), (1, 4), (0, 4)])
    exit_right = shapely.Polygon([(19, 0), (20, 0), (20, 4), (19, 4)])

    exit_left = sim.add_exit([(exit_left, room)])
    exit_right = sim.add_exit([(exit_right, room)])

    journey_right = jps.Route([exit_right])
    journey_left = jps.Route([exit_left])

    sim.add_agent(
        room.at(3, 2),
        route=journey_right,
        state=jps.WarpDriverModelState(
            orientation=(1, 0), desired_speed=1.2, radius=0.15
        ),
    )
    sim.add_agent(
        room.at(17, 2),
        route=journey_left,
        state=jps.WarpDriverModelState(
            orientation=(-1, 0), desired_speed=1.2, radius=0.15
        ),
    )

    max_steps = 3000
    for _ in range(max_steps):
        sim.iterate()
        if sim.agent_count() == 0:
            break

    assert sim.agent_count() == 0, (
        f"Both agents should have exited, but {sim.agent_count()} remain"
    )


def test_agent_parameters():
    """Test per-agent parameter access and mutation."""
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (10, 0), (10, 4), (0, 4)])
    )
    sim = jps.Simulation(model=jps.WarpDriverModel(), geometry=surface, dt=0.01)

    aid = sim.add_agent(
        room.at(1, 2),
        route=stay_at(sim, room, (5, 2)),
        state=jps.WarpDriverModelState(desired_speed=1.0, radius=0.2),
    ).id

    state = sim.agent(aid).state
    assert math.isclose(state.radius, 0.2)
    assert math.isclose(state.desired_speed, 1.0)

    # Mutate radius and desired_speed
    state.radius = 0.3
    state.desired_speed = 1.5
    assert math.isclose(sim.agent(aid).state.radius, 0.3)
    assert math.isclose(sim.agent(aid).state.desired_speed, 1.5)


def test_invalid_agent_state():
    """Invalid per-agent state raises errors when adding the agent."""
    surface = jps.WalkableSurface()
    room = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (10, 0), (10, 4), (0, 4)])
    )
    sim = jps.Simulation(model=jps.WarpDriverModel(), geometry=surface, dt=0.01)
    route = stay_at(sim, room, (5, 2))

    with pytest.raises(jps.SimulationError, match="radius"):
        sim.add_agent(
            room.at(1, 2),
            route=route,
            state=jps.WarpDriverModelState(radius=-1.0),
        )

    with pytest.raises(jps.SimulationError, match="v0"):
        sim.add_agent(
            room.at(1, 2),
            route=route,
            state=jps.WarpDriverModelState(desired_speed=-1.0),
        )
