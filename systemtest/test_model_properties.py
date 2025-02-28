# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest


@pytest.fixture
def corridor():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=[(0, 0), (10, 0), (10, 2), (0, 2)],
    )


def test_set_v0(corridor):
    sim = corridor
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.CollisionFreeSpeedModelV2AgentParameters(
        journey_id=journey_id, stage_id=wp, position=(1, 1), v0=1
    )
    agent_id = sim.add_agent(agent)
    assert math.isclose(sim.agent(agent_id).position[0], 1)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(sim.agent(agent_id).position[0], 2)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(sim.agent(agent_id).position[0], 3)
    sim.agent(agent_id).model.v0 = 2
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(sim.agent(agent_id).position[0], 5)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(sim.agent(agent_id).position[0], 7)


@pytest.fixture
def simulation_with_collision_free_speed_model_v2():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )


def test_initial_parameters_collision_free_speed_model_v2(
    simulation_with_collision_free_speed_model_v2,
):
    sim = simulation_with_collision_free_speed_model_v2
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    # Create an agent with distinct non-default values for each parameter.
    params = jps.CollisionFreeSpeedModelV2AgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
        time_gap=0.1,
        v0=0.12,
        radius=0.13,
        strength_neighbor_repulsion=0.14,
        range_neighbor_repulsion=0.15,
        strength_geometry_repulsion=0.16,
        range_geometry_repulsion=0.17,
    )
    agent_id = sim.add_agent(params)

    agent_model = sim.agent(agent_id).model
    assert agent_model.time_gap == 0.1, (
        f"Expected time_gap to be 0.1, got {agent_model.time_gap}"
    )
    assert agent_model.v0 == 0.12, (
        f"Expected v0 to be 0.12, got {agent_model.v0}"
    )
    assert agent_model.radius == 0.13, (
        f"Expected radius to be 0.13, got {agent_model.radius}"
    )
    assert agent_model.strength_neighbor_repulsion == 0.14, (
        f"Expected strength_neighbor_repulsion to be 0.14, got {agent_model.strength_neighbor_repulsion}"
    )
    assert agent_model.range_neighbor_repulsion == 0.15, (
        f"Expected range_neighbor_repulsion to be 0.15, got {agent_model.range_neighbor_repulsion}"
    )
    assert agent_model.strength_geometry_repulsion == 0.16, (
        f"Expected strength_geometry_repulsion to be 0.16, got {agent_model.strength_geometry_repulsion}"
    )
    assert agent_model.range_geometry_repulsion == 0.17, (
        f"Expected range_geometry_repulsion to be 0.17, got {agent_model.range_geometry_repulsion}"
    )


def test_set_model_parameters_collision_free_speed_model_v2(
    simulation_with_collision_free_speed_model_v2,
):
    sim = simulation_with_collision_free_speed_model_v2
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.CollisionFreeSpeedModelV2AgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    sim.agent(agent_id).model.v0 = 2.0
    assert sim.agent(agent_id).model.v0 == 2.0

    sim.agent(agent_id).model.time_gap = 3.0
    assert sim.agent(agent_id).model.time_gap == 3.0

    sim.agent(agent_id).model.radius = 4.0
    assert sim.agent(agent_id).model.radius == 4.0

    sim.agent(agent_id).model.strength_neighbor_repulsion = 5.0
    assert sim.agent(agent_id).model.strength_neighbor_repulsion == 5.0

    sim.agent(agent_id).model.range_neighbor_repulsion = 6.0
    assert sim.agent(agent_id).model.range_neighbor_repulsion == 6.0

    sim.agent(agent_id).model.range_geometry_repulsion = 7.0
    assert sim.agent(agent_id).model.range_geometry_repulsion == 7.0

    sim.agent(agent_id).model.range_geometry_repulsion = 8.0
    assert sim.agent(agent_id).model.range_geometry_repulsion == 8.0


@pytest.fixture
def simulation_with_anticipation_velocity_model():
    return jps.Simulation(
        model=jps.AnticipationVelocityModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )


def test_set_model_parameters_anticipation_velocity_model(
    simulation_with_anticipation_velocity_model,
):
    sim = simulation_with_anticipation_velocity_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.AnticipationVelocityModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    sim.agent(agent_id).model.v0 = 2.0
    assert sim.agent(agent_id).model.v0 == 2.0

    sim.agent(agent_id).model.time_gap = 3.0
    assert sim.agent(agent_id).model.time_gap == 3.0

    sim.agent(agent_id).model.radius = 4.0
    assert sim.agent(agent_id).model.radius == 4.0

    sim.agent(agent_id).model.strength_neighbor_repulsion = 5.0
    assert sim.agent(agent_id).model.strength_neighbor_repulsion == 5.0

    sim.agent(agent_id).model.range_neighbor_repulsion = 6.0
    assert sim.agent(agent_id).model.range_neighbor_repulsion == 6.0

    sim.agent(agent_id).model.wall_buffer_distance = 1.1
    assert sim.agent(agent_id).model.wall_buffer_distance == 1.1

    sim.agent(agent_id).model.anticipation_time = 2.1
    assert sim.agent(agent_id).model.anticipation_time == 2.1

    sim.agent(agent_id).model.reaction_time = 0.31
    assert sim.agent(agent_id).model.reaction_time == 0.31


@pytest.fixture
def simulation_with_collision_free_speed_model():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )


def test_set_model_parameters_collision_free_speed_model(
    simulation_with_collision_free_speed_model,
):
    sim = simulation_with_collision_free_speed_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.CollisionFreeSpeedModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    sim.agent(agent_id).model.v0 = 2.0
    assert sim.agent(agent_id).model.v0 == 2.0

    sim.agent(agent_id).model.time_gap = 3.0
    assert sim.agent(agent_id).model.time_gap == 3.0

    sim.agent(agent_id).model.radius = 4.0
    assert sim.agent(agent_id).model.radius == 4.0


@pytest.fixture
def simulation_with_generalized_centrifugal_force_model():
    return jps.Simulation(
        model=jps.GeneralizedCentrifugalForceModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )


def test_set_model_parameters_generalized_centrifugal_force_model(
    simulation_with_generalized_centrifugal_force_model,
):
    sim = simulation_with_generalized_centrifugal_force_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.GeneralizedCentrifugalForceModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent2 = jps.GeneralizedCentrifugalForceModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(3, 1),
    )

    agent_id = sim.add_agent(agent)
    sim.add_agent(agent2)

    sim.agent(agent_id).model.speed = 2.0
    assert sim.agent(agent_id).model.speed == 2.0

    sim.agent(agent_id).model.e0 = (3.0, -3.0)
    assert sim.agent(agent_id).model.e0 == (3.0, -3.0)

    sim.agent(agent_id).model.tau = 4.0
    assert sim.agent(agent_id).model.tau == 4.0

    sim.agent(agent_id).model.v0 = 5.0
    assert sim.agent(agent_id).model.v0 == 5.0

    sim.agent(agent_id).model.a_v = 6.0
    assert sim.agent(agent_id).model.a_v == 6.0

    sim.agent(agent_id).model.a_min = 7.0
    assert sim.agent(agent_id).model.a_min == 7.0

    sim.agent(agent_id).model.b_min = 8.0
    assert sim.agent(agent_id).model.b_min == 8.0

    sim.agent(agent_id).model.b_max = 9.0
    assert sim.agent(agent_id).model.b_max == 9.0


@pytest.fixture
def simulation_with_social_force_model():
    return jps.Simulation(
        model=jps.SocialForceModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )


def test_set_model_parameters_social_force_model(
    simulation_with_social_force_model,
):
    sim = simulation_with_social_force_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.SocialForceModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    sim.agent(agent_id).model.velocity = (2.0, -2.0)
    assert sim.agent(agent_id).model.velocity == (2.0, -2.0)

    sim.agent(agent_id).model.mass = 3.0
    assert sim.agent(agent_id).model.mass == 3.0

    sim.agent(agent_id).model.desiredSpeed = 4.0
    assert sim.agent(agent_id).model.desiredSpeed == 4.0

    sim.agent(agent_id).model.reactionTime = 5.0
    assert sim.agent(agent_id).model.reactionTime == 5.0

    sim.agent(agent_id).model.agentScale = 6.0
    assert sim.agent(agent_id).model.agentScale == 6.0

    sim.agent(agent_id).model.obstacleScale = 7.0
    assert sim.agent(agent_id).model.obstacleScale == 7.0

    sim.agent(agent_id).model.ForceDistance = 8.0
    assert sim.agent(agent_id).model.ForceDistance == 8.0

    sim.agent(agent_id).model.radius = 9.0
    assert sim.agent(agent_id).model.radius == 9.0
