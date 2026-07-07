# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest

_UNIT_SQUARE_10 = [(0, 0), (10, 0), (10, 10), (0, 10)]

# Each built-in model is constructed as a configured instance. Extra state
# kwargs cover cases where the raw C++ struct defaults do not satisfy the
# model's add_agent constraints (GCFM requires a unit-length orientation, V3
# requires non-zero repulsion ranges).
MODEL_INSTANCE_STATE = [
    (jps.CollisionFreeSpeedModel(), jps.CollisionFreeSpeedModelState, {}),
    (
        jps.CollisionFreeSpeedModelV2(),
        jps.CollisionFreeSpeedModelV2State,
        {},
    ),
    (
        jps.CollisionFreeSpeedModelV3(),
        jps.CollisionFreeSpeedModelV3State,
        {
            "strength_neighbor_repulsion": 8.0,
            "range_neighbor_repulsion": 0.1,
            "strength_geometry_repulsion": 5.0,
            "range_geometry_repulsion": 0.02,
        },
    ),
    (
        jps.GeneralizedCentrifugalForceModel(),
        jps.GeneralizedCentrifugalForceModelState,
        {"orientation": (1.0, 0.0)},
    ),
    (jps.SocialForceModel(), jps.SocialForceModelState, {}),
]


@pytest.mark.parametrize(
    "model, state_cls, state_kwargs",
    MODEL_INSTANCE_STATE,
    ids=[type(model).__name__ for model, _, _ in MODEL_INSTANCE_STATE],
)
def test_simulation_from_model_instance(model, state_cls, state_kwargs):
    """Built-in models are constructed from a configured model instance."""
    sim = jps.Simulation(model=model, geometry=_UNIT_SQUARE_10)
    wp = sim.add_waypoint_stage((9, 9), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=state_cls(position=(1, 1), **state_kwargs),
    )

    for _ in range(10):
        sim.iterate()
    assert sim.agent(agent_id).position != (1, 1)


def test_simulation_rejects_wrong_model_argument():
    with pytest.raises(TypeError):
        jps.Simulation(model=42, geometry=_UNIT_SQUARE_10)


@pytest.fixture
def corridor():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=[(0, 0), (10, 0), (10, 2), (0, 2)],
    )


def test_set_desired_speed(corridor):
    sim = corridor
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.CollisionFreeSpeedModelV2State(
            position=(1, 1), desired_speed=1
        ),
    )
    assert math.isclose(sim.agent(agent_id).position[0], 1)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(sim.agent(agent_id).position[0], 2)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(sim.agent(agent_id).position[0], 3)
    sim.agent(agent_id).model.desired_speed = 2
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
        geometry=_UNIT_SQUARE_10,
    )


def test_initial_parameters_collision_free_speed_model_v2(
    simulation_with_collision_free_speed_model_v2,
):
    sim = simulation_with_collision_free_speed_model_v2
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    # Create an agent with distinct non-default values for each parameter.
    state = jps.CollisionFreeSpeedModelV2State(
        position=(1, 1),
        time_gap=0.1,
        desired_speed=0.12,
        radius=0.13,
        strength_neighbor_repulsion=0.14,
        range_neighbor_repulsion=0.15,
        strength_geometry_repulsion=0.16,
        range_geometry_repulsion=0.17,
    )
    agent_id = sim.add_agent(journey_id=journey_id, stage_id=wp, state=state)

    agent_model = sim.agent(agent_id).model
    assert agent_model.time_gap == 0.1, (
        f"Expected time_gap to be 0.1, got {agent_model.time_gap}"
    )
    assert agent_model.desired_speed == 0.12, (
        f"Expected desired_speed to be 0.12, got {agent_model.desired_speed}"
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

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.CollisionFreeSpeedModelV2State(position=(1, 1)),
    )

    sim.agent(agent_id).model.desired_speed = 2.0
    assert sim.agent(agent_id).model.desired_speed == 2.0

    sim.agent(agent_id).model.desired_speed = 2.1
    assert sim.agent(agent_id).model.desired_speed == 2.1

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
        model=jps.AnticipationVelocityModel(rng_seed=1234),
        geometry=_UNIT_SQUARE_10,
    )


def test_set_model_parameters_anticipation_velocity_model(
    simulation_with_anticipation_velocity_model,
):
    sim = simulation_with_anticipation_velocity_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.AnticipationVelocityModelState(position=(1, 1)),
    )

    sim.agent(agent_id).model.desired_speed = 2.0
    assert sim.agent(agent_id).model.desired_speed == 2.0

    sim.agent(agent_id).model.desired_speed = 2.1
    assert sim.agent(agent_id).model.desired_speed == 2.1

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
        geometry=_UNIT_SQUARE_10,
    )


def test_set_model_parameters_collision_free_speed_model(
    simulation_with_collision_free_speed_model,
):
    sim = simulation_with_collision_free_speed_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.CollisionFreeSpeedModelState(position=(1, 1)),
    )

    sim.agent(agent_id).model.desired_speed = 2.0
    assert sim.agent(agent_id).model.desired_speed == 2.0

    sim.agent(agent_id).model.desired_speed = 2.1
    assert sim.agent(agent_id).model.desired_speed == 2.1

    sim.agent(agent_id).model.time_gap = 3.0
    assert sim.agent(agent_id).model.time_gap == 3.0

    sim.agent(agent_id).model.radius = 4.0
    assert sim.agent(agent_id).model.radius == 4.0


def test_collision_free_speed_model_repulsion_parameters_are_model_level():
    """The repulsion parameters are configured on the model instance."""
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(
            strength_neighbor_repulsion=7.5,
            range_neighbor_repulsion=0.2,
            strength_geometry_repulsion=4.5,
            range_geometry_repulsion=0.03,
        ),
        geometry=_UNIT_SQUARE_10,
    )
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.CollisionFreeSpeedModelState(position=(1, 1)),
    )

    # The repulsion parameters no longer appear on the per-agent state.
    agent_model = sim.agent(agent_id).model
    for field in (
        "strength_neighbor_repulsion",
        "range_neighbor_repulsion",
        "strength_geometry_repulsion",
        "range_geometry_repulsion",
    ):
        assert not hasattr(agent_model, field)

    # The configured model drives the simulation.
    for _ in range(10):
        sim.iterate()
    assert sim.agent(agent_id).position != (1, 1)


@pytest.fixture
def simulation_with_generalized_centrifugal_force_model():
    return jps.Simulation(
        model=jps.GeneralizedCentrifugalForceModel(),
        geometry=_UNIT_SQUARE_10,
    )


def test_set_model_parameters_generalized_centrifugal_force_model(
    simulation_with_generalized_centrifugal_force_model,
):
    sim = simulation_with_generalized_centrifugal_force_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.GeneralizedCentrifugalForceModelState(
            position=(1, 1), orientation=(1.0, 0.0)
        ),
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.GeneralizedCentrifugalForceModelState(
            position=(3, 1), orientation=(1.0, 0.0)
        ),
    )

    sim.agent(agent_id).model.speed = 2.0
    assert sim.agent(agent_id).model.speed == 2.0

    sim.agent(agent_id).model.desired_direction = (3.0, -3.0)
    assert sim.agent(agent_id).model.desired_direction == (3.0, -3.0)

    sim.agent(agent_id).model.tau = 4.0
    assert sim.agent(agent_id).model.tau == 4.0

    sim.agent(agent_id).model.desired_speed = 5.0
    assert sim.agent(agent_id).model.desired_speed == 5.0

    sim.agent(agent_id).model.desired_speed = 5.1
    assert sim.agent(agent_id).model.desired_speed == 5.1

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
        geometry=_UNIT_SQUARE_10,
    )


def test_set_model_parameters_social_force_model(
    simulation_with_social_force_model,
):
    sim = simulation_with_social_force_model

    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.SocialForceModelState(position=(1, 1)),
    )

    sim.agent(agent_id).model.velocity = (2.0, -2.0)
    assert sim.agent(agent_id).model.velocity == (2.0, -2.0)

    sim.agent(agent_id).model.mass = 3.0
    assert sim.agent(agent_id).model.mass == 3.0

    sim.agent(agent_id).model.desired_speed = 4.0
    assert sim.agent(agent_id).model.desired_speed == 4.0

    sim.agent(agent_id).model.reaction_time = 5.0
    assert sim.agent(agent_id).model.reaction_time == 5.0

    sim.agent(agent_id).model.agent_scale = 6.0
    assert sim.agent(agent_id).model.agent_scale == 6.0

    sim.agent(agent_id).model.obstacle_scale = 7.0
    assert sim.agent(agent_id).model.obstacle_scale == 7.0

    sim.agent(agent_id).model.force_distance = 8.0
    assert sim.agent(agent_id).model.force_distance == 8.0

    sim.agent(agent_id).model.radius = 9.0
    assert sim.agent(agent_id).model.radius == 9.0


def test_social_force_model_body_force_and_friction_are_model_level():
    """body_force and friction are configured on the model instance."""
    sim = jps.Simulation(
        model=jps.SocialForceModel(body_force=100000.0, friction=200000.0),
        geometry=_UNIT_SQUARE_10,
    )
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.SocialForceModelState(position=(1, 1)),
    )

    # body_force and friction no longer appear on the per-agent state.
    agent_model = sim.agent(agent_id).model
    assert not hasattr(agent_model, "body_force")
    assert not hasattr(agent_model, "friction")

    # The configured model drives the simulation.
    for _ in range(10):
        sim.iterate()
    assert sim.agent(agent_id).position != (1, 1)


def test_agent_handle_raises_after_removal(
    simulation_with_social_force_model,
):
    sim = simulation_with_social_force_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.SocialForceModelState(position=(1, 1)),
    )
    agent = sim.agent(agent_id)
    model = agent.model
    assert agent.position == (1, 1)

    sim.mark_agent_for_removal(agent_id)
    sim.iterate()

    with pytest.raises(jps.SimulationError):
        agent.position
    with pytest.raises(jps.SimulationError):
        agent.model.desired_speed
    with pytest.raises(jps.SimulationError):
        model.desired_speed
    with pytest.raises(jps.SimulationError):
        model.desired_speed = 1.0
    with pytest.raises(jps.SimulationError):
        sim.agent(agent_id)


@pytest.fixture
def simulation_with_collision_free_speed_model_v3():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV3(),
        geometry=_UNIT_SQUARE_10,
    )


def test_initial_parameters_collision_free_speed_model_v3(
    simulation_with_collision_free_speed_model_v3,
):
    sim = simulation_with_collision_free_speed_model_v3
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    state = jps.CollisionFreeSpeedModelV3State(
        position=(1, 1),
        time_gap=0.11,
        desired_speed=0.12,
        radius=0.13,
        strength_neighbor_repulsion=0.14,
        range_neighbor_repulsion=0.15,
        strength_geometry_repulsion=0.16,
        range_geometry_repulsion=0.17,
        range_x_scale=21.0,
        range_y_scale=9.0,
        theta_max_upper_bound=0.8,
        agent_buffer=0.4,
    )
    agent_id = sim.add_agent(journey_id=journey_id, stage_id=wp, state=state)

    agent_model = sim.agent(agent_id).model
    assert agent_model.time_gap == 0.11
    assert agent_model.desired_speed == 0.12
    assert agent_model.radius == 0.13
    assert agent_model.strength_neighbor_repulsion == 0.14
    assert agent_model.range_neighbor_repulsion == 0.15
    assert agent_model.strength_geometry_repulsion == 0.16
    assert agent_model.range_geometry_repulsion == 0.17
    assert agent_model.range_x_scale == 21.0
    assert agent_model.range_y_scale == 9.0
    assert agent_model.theta_max_upper_bound == 0.8
    assert agent_model.agent_buffer == 0.4


def test_set_model_parameters_collision_free_speed_model_v3(
    simulation_with_collision_free_speed_model_v3,
):
    sim = simulation_with_collision_free_speed_model_v3
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=jps.CollisionFreeSpeedModelV3State(
            position=(1, 1),
            strength_neighbor_repulsion=8.0,
            range_neighbor_repulsion=0.1,
            strength_geometry_repulsion=5.0,
            range_geometry_repulsion=0.02,
        ),
    )
    model = sim.agent(agent_id).model

    model.desired_speed = 2.0
    assert model.desired_speed == 2.0

    model.time_gap = 3.0
    assert model.time_gap == 3.0

    model.radius = 0.3
    assert model.radius == 0.3

    model.strength_neighbor_repulsion = 5.0
    assert model.strength_neighbor_repulsion == 5.0

    model.range_neighbor_repulsion = 0.6
    assert model.range_neighbor_repulsion == 0.6

    model.strength_geometry_repulsion = 4.2
    assert model.strength_geometry_repulsion == 4.2

    model.range_geometry_repulsion = 0.8
    assert model.range_geometry_repulsion == 0.8

    model.range_x_scale = 15.0
    assert model.range_x_scale == 15.0

    model.range_y_scale = 6.0
    assert model.range_y_scale == 6.0

    model.theta_max_upper_bound = 0.9
    assert model.theta_max_upper_bound == 0.9

    model.agent_buffer = 0.5
    assert model.agent_buffer == 0.5
