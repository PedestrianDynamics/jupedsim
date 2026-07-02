# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest
import shapely
from jupedsim_examples.models.pysocial_force import (
    PythonSocialForceModel,
    PythonSocialForceModelState,
)


@pytest.fixture
def corridor_simulation():
    """20x20 room, exit on the right wall."""
    model = PythonSocialForceModel()
    geometry = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage([(19, 9), (19, 11), (20, 11), (20, 9)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = sim.add_journey(journey)
    return sim, exit_id, journey_id


def _add_agent(sim, journey_id, stage_id, position, velocity=(0.0, 0.0)):
    params = jps.CustomModelAgentParameters(
        journey_id=journey_id,
        stage_id=stage_id,
        model=PythonSocialForceModelState(position=position, velocity=velocity),
    )
    return sim.add_agent(params)


def test_simulation_can_be_created_with_python_social_force_model():
    model = PythonSocialForceModel()
    geometry = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    assert sim is not None


def test_agent_can_be_added_with_default_parameters(corridor_simulation):
    sim, exit_id, journey_id = corridor_simulation
    agent_id = _add_agent(sim, journey_id, exit_id, (2.0, 10.0))
    assert sim.agent_count() == 1
    assert sim.agent(agent_id).position == pytest.approx((2.0, 10.0))


def test_agent_moves_toward_exit(corridor_simulation):
    sim, exit_id, journey_id = corridor_simulation
    start = (2.0, 10.0)
    agent_id = _add_agent(sim, journey_id, exit_id, start)

    for _ in range(50):
        sim.iterate()

    pos = sim.agent(agent_id).position
    assert pos[0] > start[0], "Agent should move in +x direction toward exit"


def test_single_agent_reaches_exit(corridor_simulation):
    sim, exit_id, journey_id = corridor_simulation
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0))

    max_steps = 2000
    for _ in range(max_steps):
        if sim.agent_count() == 0:
            break
        sim.iterate()

    assert sim.agent_count() == 0, (
        "Agent should reach the exit within 2000 steps"
    )


def test_multiple_agents_all_reach_exit(corridor_simulation):
    sim, exit_id, journey_id = corridor_simulation
    positions = [(2.0, 10.0), (3.0, 10.5), (2.5, 9.5), (4.0, 10.0)]
    for pos in positions:
        _add_agent(sim, journey_id, exit_id, pos)

    assert sim.agent_count() == len(positions)

    max_steps = 4000
    for _ in range(max_steps):
        if sim.agent_count() == 0:
            break
        sim.iterate()

    assert sim.agent_count() == 0, "All agents should reach the exit"


def test_social_force_prevents_agent_collapse():
    """Agents heading toward the same target should not collapse into each other."""
    model = PythonSocialForceModel()
    geometry = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage([(19, 9), (19, 11), (20, 11), (20, 9)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = sim.add_journey(journey)

    # Two agents starting 1 m apart — within interaction range (2 m) but not overlapping
    id1 = _add_agent(sim, journey_id, exit_id, (5.0, 9.5))
    id2 = _add_agent(sim, journey_id, exit_id, (5.0, 10.5))

    min_dist = float("inf")
    for _ in range(100):
        sim.iterate()
        p1 = sim.agent(id1).position
        p2 = sim.agent(id2).position
        d = ((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2) ** 0.5
        if d < min_dist:
            min_dist = d

    # Social force should keep agents from fully merging; radius sum is 0.6 m
    assert min_dist > 0.4, (
        f"Social force should prevent collapse, min separation was {min_dist:.3f} m"
    )


def test_custom_desired_speed_affects_movement():
    """Agents with higher desired_speed should travel further in the same time."""
    model = PythonSocialForceModel()
    geometry = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage([(19, 9), (19, 11), (20, 11), (20, 9)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = sim.add_journey(journey)

    # Slow agent
    slow = jps.CustomModelAgentParameters(
        journey_id=journey_id,
        stage_id=exit_id,
        model=PythonSocialForceModelState(
            position=(2.0, 8.0), velocity=(0.0, 0.0), desired_speed=0.5
        ),
    )
    slow_id = sim.add_agent(slow)

    # Fast agent
    fast = jps.CustomModelAgentParameters(
        journey_id=journey_id,
        stage_id=exit_id,
        model=PythonSocialForceModelState(
            position=(2.0, 12.0), velocity=(0.0, 0.0), desired_speed=2.0
        ),
    )
    fast_id = sim.add_agent(fast)

    for _ in range(100):
        sim.iterate()

    slow_x = sim.agent(slow_id).position[0]
    fast_x = sim.agent(fast_id).position[0]
    assert fast_x > slow_x, "Fast agent should travel further than slow agent"


def test_obstacle_force_keeps_agents_from_walls():
    """Agents near walls should be repelled and not pass through them."""
    model = PythonSocialForceModel()
    geometry = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage([(19, 9), (19, 11), (20, 11), (20, 9)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = sim.add_journey(journey)

    agent_id = _add_agent(sim, journey_id, exit_id, (2.0, 10.0))

    for _ in range(200):
        if sim.agent_count() == 0:
            break
        sim.iterate()
        if sim.agent_count() > 0:
            pos = sim.agent(agent_id).position
            assert 0 < pos[0] < 20, f"Agent x={pos[0]} went outside geometry"
            assert 0 < pos[1] < 20, f"Agent y={pos[1]} went outside geometry"


def test_per_agent_state_survives_iterations(corridor_simulation):
    """Non-default per-agent parameters must survive the state round-trip.

    Guards against compute_new_position rebuilding the state from defaults
    instead of carrying the current state forward (dataclasses.replace).
    """
    sim, exit_id, journey_id = corridor_simulation
    params = jps.CustomModelAgentParameters(
        journey_id=journey_id,
        stage_id=exit_id,
        model=PythonSocialForceModelState(
            position=(2.0, 10.0),
            velocity=(0.0, 0.0),
            desired_speed=2.0,
            reaction_time=0.25,
        ),
    )
    agent_id = sim.add_agent(params)

    for _ in range(20):
        sim.iterate()

    state = sim.agent(agent_id).model
    assert isinstance(state, PythonSocialForceModelState)
    assert state.desired_speed == 2.0
    assert state.reaction_time == 0.25
    assert state.position == pytest.approx(sim.agent(agent_id).position)
