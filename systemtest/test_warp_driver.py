# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest
import shapely


@pytest.fixture
def warp_driver_corridor():
    """20m x 4m corridor with WarpDriver model."""
    area = shapely.Polygon([(0, 0), (20, 0), (20, 4), (0, 4)])
    return jps.Simulation(model=jps.WarpDriverModel(), geometry=area, dt=0.01)


def test_default_model_construction():
    """Test that WarpDriver model can be created with defaults."""
    model = jps.WarpDriverModel()
    assert model.time_horizon == 2.0
    assert model.step_size == 0.5
    assert model.sigma == 0.3
    assert model.num_samples == 20


def test_simulation_runs(warp_driver_corridor):
    """Test that a basic simulation runs without errors."""
    sim = warp_driver_corridor
    exit_area = shapely.Polygon([(19, 0), (20, 0), (20, 4), (19, 4)])
    exit_id = sim.add_exit_stage(exit_area)
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    sim.add_agent(
        jps.WarpDriverModelAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
            desired_speed=1.2,
            radius=0.15,
        )
    )

    for _ in range(50):
        sim.iterate()
    assert sim.agent_count() >= 0


def test_single_agent_straight_path(warp_driver_corridor):
    """Single agent with no neighbors follows straight path towards target."""
    sim = warp_driver_corridor
    wp = sim.add_waypoint_stage((10, 2), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    aid = sim.add_agent(
        jps.WarpDriverModelAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=wp,
            desired_speed=1.0,
            radius=0.15,
        )
    )

    for _ in range(100):
        sim.iterate()

    pos = sim.agent(aid).position
    # Agent should have moved towards x=10, staying near y=2
    assert pos[0] > 2.5, f"Agent should have moved forward, got x={pos[0]}"
    assert abs(pos[1] - 2.0) < 0.5, (
        f"Agent should stay near y=2, got y={pos[1]}"
    )


def test_two_agents_head_on_avoid(warp_driver_corridor):
    """Two head-on agents should avoid each other and both reach their goals."""
    sim = warp_driver_corridor
    exit_left = shapely.Polygon([(0, 0), (1, 0), (1, 4), (0, 4)])
    exit_right = shapely.Polygon([(19, 0), (20, 0), (20, 4), (19, 4)])

    exit_left_id = sim.add_exit_stage(exit_left)
    exit_right_id = sim.add_exit_stage(exit_right)

    journey_right = sim.add_journey(jps.JourneyDescription([exit_right_id]))
    journey_left = sim.add_journey(jps.JourneyDescription([exit_left_id]))

    sim.add_agent(
        jps.WarpDriverModelAgentParameters(
            position=(3, 2),
            orientation=(1, 0),
            journey_id=journey_right,
            stage_id=exit_right_id,
            desired_speed=1.2,
            radius=0.15,
        )
    )
    sim.add_agent(
        jps.WarpDriverModelAgentParameters(
            position=(17, 2),
            orientation=(-1, 0),
            journey_id=journey_left,
            stage_id=exit_left_id,
            desired_speed=1.2,
            radius=0.15,
        )
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
    area = shapely.Polygon([(0, 0), (10, 0), (10, 4), (0, 4)])
    sim = jps.Simulation(model=jps.WarpDriverModel(), geometry=area, dt=0.01)

    wp = sim.add_waypoint_stage((5, 2), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    aid = sim.add_agent(
        jps.WarpDriverModelAgentParameters(
            position=(1, 2),
            journey_id=journey_id,
            stage_id=wp,
            desired_speed=1.0,
            radius=0.2,
        )
    )

    state = sim.agent(aid).model
    assert math.isclose(state.radius, 0.2)
    assert math.isclose(state.desired_speed, 1.0)
    assert state.jam_counter == 0

    # Mutate radius and desired_speed
    state.radius = 0.3
    state.desired_speed = 1.5
    assert math.isclose(sim.agent(aid).model.radius, 0.3)
    assert math.isclose(sim.agent(aid).model.desired_speed, 1.5)


def test_invalid_parameters():
    """Invalid model parameters raise errors at simulation creation."""
    area = shapely.Polygon([(0, 0), (10, 0), (10, 4), (0, 4)])

    with pytest.raises(Exception):
        jps.Simulation(
            model=jps.WarpDriverModel(time_horizon=-1.0),
            geometry=area,
            dt=0.01,
        )

    with pytest.raises(Exception):
        jps.Simulation(
            model=jps.WarpDriverModel(sigma=0.0),
            geometry=area,
            dt=0.01,
        )

    with pytest.raises(Exception):
        jps.Simulation(
            model=jps.WarpDriverModel(num_samples=0),
            geometry=area,
            dt=0.01,
        )
