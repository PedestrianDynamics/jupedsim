# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest
import shapely

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
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    sim = jps.Simulation(model=model, geometry=surface.build())
    wp = sim.add_target(
        jps.Waypoint([(jps.circle_to_polygon(center=(9, 9), radius=0.5), room)])
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=state_cls(**state_kwargs),
    )

    for _ in range(10):
        sim.iterate()
    assert agent.location.xy != (1, 1)


def test_simulation_rejects_wrong_model_argument():
    with pytest.raises(TypeError):
        jps.Simulation(model=42, geometry=jps.GeometryBuilder())


@pytest.fixture
def corridor():
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(0, 0), (10, 0), (10, 2), (0, 2)], height=0.0
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=surface.build(),
    )
    return simulation, room


def test_set_desired_speed(corridor):
    sim, room = corridor
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.CollisionFreeSpeedModelV2State(desired_speed=1),
    )
    assert math.isclose(agent.location.xy[0], 1)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(agent.location.xy[0], 2)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(agent.location.xy[0], 3)
    agent.state.desired_speed = 2
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(agent.location.xy[0], 5)
    for _ in range(0, 100):
        sim.iterate()
    assert math.isclose(agent.location.xy[0], 7)


@pytest.fixture
def simulation_with_collision_free_speed_model_v2():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=surface.build(),
    )
    return simulation, room


def test_initial_parameters_collision_free_speed_model_v2(
    simulation_with_collision_free_speed_model_v2,
):
    sim, room = simulation_with_collision_free_speed_model_v2
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    # Create an agent with distinct non-default values for each parameter.
    state = jps.CollisionFreeSpeedModelV2State(
        time_gap=0.1,
        desired_speed=0.12,
        radius=0.13,
        strength_neighbor_repulsion=0.14,
        range_neighbor_repulsion=0.15,
        strength_geometry_repulsion=0.16,
        range_geometry_repulsion=0.17,
    )
    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=state,
    )

    agent_state = agent.state
    assert agent_state.time_gap == 0.1, (
        f"Expected time_gap to be 0.1, got {agent_state.time_gap}"
    )
    assert agent_state.desired_speed == 0.12, (
        f"Expected desired_speed to be 0.12, got {agent_state.desired_speed}"
    )
    assert agent_state.radius == 0.13, (
        f"Expected radius to be 0.13, got {agent_state.radius}"
    )
    assert agent_state.strength_neighbor_repulsion == 0.14, (
        f"Expected strength_neighbor_repulsion to be 0.14, got {agent_state.strength_neighbor_repulsion}"
    )
    assert agent_state.range_neighbor_repulsion == 0.15, (
        f"Expected range_neighbor_repulsion to be 0.15, got {agent_state.range_neighbor_repulsion}"
    )
    assert agent_state.strength_geometry_repulsion == 0.16, (
        f"Expected strength_geometry_repulsion to be 0.16, got {agent_state.strength_geometry_repulsion}"
    )
    assert agent_state.range_geometry_repulsion == 0.17, (
        f"Expected range_geometry_repulsion to be 0.17, got {agent_state.range_geometry_repulsion}"
    )


def test_set_model_parameters_collision_free_speed_model_v2(
    simulation_with_collision_free_speed_model_v2,
):
    sim, room = simulation_with_collision_free_speed_model_v2
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.CollisionFreeSpeedModelV2State(),
    )

    agent.state.desired_speed = 2.0
    assert agent.state.desired_speed == 2.0

    agent.state.desired_speed = 2.1
    assert agent.state.desired_speed == 2.1

    agent.state.time_gap = 3.0
    assert agent.state.time_gap == 3.0

    agent.state.radius = 4.0
    assert agent.state.radius == 4.0

    agent.state.strength_neighbor_repulsion = 5.0
    assert agent.state.strength_neighbor_repulsion == 5.0

    agent.state.range_neighbor_repulsion = 6.0
    assert agent.state.range_neighbor_repulsion == 6.0

    agent.state.range_geometry_repulsion = 7.0
    assert agent.state.range_geometry_repulsion == 7.0

    agent.state.range_geometry_repulsion = 8.0
    assert agent.state.range_geometry_repulsion == 8.0


@pytest.fixture
def simulation_with_anticipation_velocity_model():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    simulation = jps.Simulation(
        model=jps.AnticipationVelocityModel(rng_seed=1234),
        geometry=surface.build(),
    )
    return simulation, room


def test_set_model_parameters_anticipation_velocity_model(
    simulation_with_anticipation_velocity_model,
):
    sim, room = simulation_with_anticipation_velocity_model
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.AnticipationVelocityModelState(),
    )

    agent.state.desired_speed = 2.0
    assert agent.state.desired_speed == 2.0

    agent.state.desired_speed = 2.1
    assert agent.state.desired_speed == 2.1

    agent.state.time_gap = 3.0
    assert agent.state.time_gap == 3.0

    agent.state.radius = 4.0
    assert agent.state.radius == 4.0

    agent.state.strength_neighbor_repulsion = 5.0
    assert agent.state.strength_neighbor_repulsion == 5.0

    agent.state.range_neighbor_repulsion = 6.0
    assert agent.state.range_neighbor_repulsion == 6.0

    agent.state.wall_buffer_distance = 1.1
    assert agent.state.wall_buffer_distance == 1.1

    agent.state.anticipation_time = 2.1
    assert agent.state.anticipation_time == 2.1

    agent.state.reaction_time = 0.31
    assert agent.state.reaction_time == 0.31


@pytest.fixture
def simulation_with_collision_free_speed_model():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=surface.build(),
    )
    return simulation, room


def test_set_model_parameters_collision_free_speed_model(
    simulation_with_collision_free_speed_model,
):
    sim, room = simulation_with_collision_free_speed_model
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )

    agent.state.desired_speed = 2.0
    assert agent.state.desired_speed == 2.0

    agent.state.desired_speed = 2.1
    assert agent.state.desired_speed == 2.1

    agent.state.time_gap = 3.0
    assert agent.state.time_gap == 3.0

    agent.state.radius = 4.0
    assert agent.state.radius == 4.0


def test_collision_free_speed_model_repulsion_parameters_are_model_level():
    """The repulsion parameters are configured on the model instance."""
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(
            strength_neighbor_repulsion=7.5,
            range_neighbor_repulsion=0.2,
            strength_geometry_repulsion=4.5,
            range_geometry_repulsion=0.03,
        ),
        geometry=surface.build(),
    )
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.CollisionFreeSpeedModelState(),
    )

    # The repulsion parameters no longer appear on the per-agent state.
    agent_state = agent.state
    for field in (
        "strength_neighbor_repulsion",
        "range_neighbor_repulsion",
        "strength_geometry_repulsion",
        "range_geometry_repulsion",
    ):
        assert not hasattr(agent_state, field)

    # The configured model drives the simulation.
    for _ in range(10):
        sim.iterate()
    assert agent.location.xy != (1, 1)


@pytest.fixture
def simulation_with_generalized_centrifugal_force_model():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    simulation = jps.Simulation(
        model=jps.GeneralizedCentrifugalForceModel(),
        geometry=surface.build(),
    )
    return simulation, room


def test_set_model_parameters_generalized_centrifugal_force_model(
    simulation_with_generalized_centrifugal_force_model,
):
    sim, room = simulation_with_generalized_centrifugal_force_model
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.GeneralizedCentrifugalForceModelState(orientation=(1.0, 0.0)),
    )
    sim.add_agent(
        room.at(3, 1),
        route=route,
        state=jps.GeneralizedCentrifugalForceModelState(orientation=(1.0, 0.0)),
    )

    agent.state.speed = 2.0
    assert agent.state.speed == 2.0

    agent.state.desired_direction = (3.0, -3.0)
    assert agent.state.desired_direction == (3.0, -3.0)

    agent.state.tau = 4.0
    assert agent.state.tau == 4.0

    agent.state.desired_speed = 5.0
    assert agent.state.desired_speed == 5.0

    agent.state.desired_speed = 5.1
    assert agent.state.desired_speed == 5.1

    agent.state.a_v = 6.0
    assert agent.state.a_v == 6.0

    agent.state.a_min = 7.0
    assert agent.state.a_min == 7.0

    agent.state.b_min = 8.0
    assert agent.state.b_min == 8.0

    agent.state.b_max = 9.0
    assert agent.state.b_max == 9.0


@pytest.fixture
def simulation_with_social_force_model():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    simulation = jps.Simulation(
        model=jps.SocialForceModel(),
        geometry=surface.build(),
    )
    return simulation, room


def test_set_model_parameters_social_force_model(
    simulation_with_social_force_model,
):
    sim, room = simulation_with_social_force_model

    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.SocialForceModelState(),
    )

    agent.state.velocity = (2.0, -2.0)
    assert agent.state.velocity == (2.0, -2.0)

    agent.state.mass = 3.0
    assert agent.state.mass == 3.0

    agent.state.desired_speed = 4.0
    assert agent.state.desired_speed == 4.0

    agent.state.reaction_time = 5.0
    assert agent.state.reaction_time == 5.0

    agent.state.agent_scale = 6.0
    assert agent.state.agent_scale == 6.0

    agent.state.obstacle_scale = 7.0
    assert agent.state.obstacle_scale == 7.0

    agent.state.force_distance = 8.0
    assert agent.state.force_distance == 8.0

    agent.state.radius = 9.0
    assert agent.state.radius == 9.0


def test_social_force_model_body_force_and_friction_are_model_level():
    """body_force and friction are configured on the model instance."""
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    sim = jps.Simulation(
        model=jps.SocialForceModel(body_force=100000.0, friction=200000.0),
        geometry=surface.build(),
    )
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.SocialForceModelState(),
    )

    # body_force and friction no longer appear on the per-agent state.
    agent_state = agent.state
    assert not hasattr(agent_state, "body_force")
    assert not hasattr(agent_state, "friction")

    # The configured model drives the simulation.
    for _ in range(10):
        sim.iterate()
    assert agent.location.xy != (1, 1)


def _velocity_along_wall(friction, y):
    """return x-velocity after 20 ms of an agent (radius 0.3 m) at height y in a
    corridor whose wall runs along y = 0. It starts at its desired speed of
    1 m/s along +x, so without other forces its speed stays constant.
    Use DirectSteering as FF-engine would push it away from the wall."""
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(0, 0), (60, 0), (60, 4), (0, 4)], height=0.0
    )
    sim = jps.Simulation(
        model=jps.SocialForceModel(body_force=120000.0, friction=friction),
        geometry=surface.build(),
        dt=0.001,
    )
    agent = sim.add_agent(
        room.at(1, y),
        steer_to=room.at(59, y),
        state=jps.SocialForceModelState(
            velocity=(1.0, 0.0),
            desired_speed=1.0,
            reaction_time=0.5,
            radius=0.3,
        ),
    )
    for _ in range(20):
        sim.iterate()
    return agent.state.velocity[0]


def test_social_force_model_wall_friction_slows_down_an_agent_sliding_along():
    """An agent overlapping a wall by 5 cm slides along it. Friction acts
    against the motion: it slows the agent down, but does not reverse it.
    With the wrong sign it would push the agent forward instead (to about
    10 m/s within these 20 ms)."""
    frictionless = _velocity_along_wall(friction=0.0, y=0.25)
    with_friction = _velocity_along_wall(friction=240000.0, y=0.25)
    assert frictionless == pytest.approx(1.0, abs=0.01)
    assert 0.0 <= with_friction < 0.5 * frictionless


def test_social_force_model_wall_friction_needs_contact():
    """Away from the wall (no overlap) friction has no effect at all."""
    assert _velocity_along_wall(friction=240000.0, y=1.0) == pytest.approx(
        _velocity_along_wall(friction=0.0, y=1.0)
    )


def test_agent_handle_raises_after_removal(
    simulation_with_social_force_model,
):
    sim, room = simulation_with_social_force_model
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.SocialForceModelState(),
    )
    state = agent.state
    assert agent.location.xy == (1, 1)

    sim.mark_agent_for_removal(agent)
    sim.iterate()

    with pytest.raises(jps.SimulationError):
        agent.location.xy
    with pytest.raises(jps.SimulationError):
        agent.state.desired_speed
    with pytest.raises(jps.SimulationError):
        state.desired_speed
    with pytest.raises(jps.SimulationError):
        state.desired_speed = 1.0
    with pytest.raises(KeyError):
        sim.agent(agent.id)


@pytest.fixture
def simulation_with_collision_free_speed_model_v3():
    surface = jps.GeometryBuilder()
    room = surface.add_region(boundary=_UNIT_SQUARE_10, height=0.0)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV3(),
        geometry=surface.build(),
    )
    return simulation, room


def test_initial_parameters_collision_free_speed_model_v3(
    simulation_with_collision_free_speed_model_v3,
):
    sim, room = simulation_with_collision_free_speed_model_v3
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    state = jps.CollisionFreeSpeedModelV3State(
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
    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=state,
    )

    agent_state = agent.state
    assert agent_state.time_gap == 0.11
    assert agent_state.desired_speed == 0.12
    assert agent_state.radius == 0.13
    assert agent_state.strength_neighbor_repulsion == 0.14
    assert agent_state.range_neighbor_repulsion == 0.15
    assert agent_state.strength_geometry_repulsion == 0.16
    assert agent_state.range_geometry_repulsion == 0.17
    assert agent_state.range_x_scale == 21.0
    assert agent_state.range_y_scale == 9.0
    assert agent_state.theta_max_upper_bound == 0.8
    assert agent_state.agent_buffer == 0.4


def test_set_model_parameters_collision_free_speed_model_v3(
    simulation_with_collision_free_speed_model_v3,
):
    sim, room = simulation_with_collision_free_speed_model_v3
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(10, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])

    agent = sim.add_agent(
        room.at(1, 1),
        route=route,
        state=jps.CollisionFreeSpeedModelV3State(
            strength_neighbor_repulsion=8.0,
            range_neighbor_repulsion=0.1,
            strength_geometry_repulsion=5.0,
            range_geometry_repulsion=0.02,
        ),
    )
    state = agent.state

    state.desired_speed = 2.0
    assert state.desired_speed == 2.0

    state.time_gap = 3.0
    assert state.time_gap == 3.0

    state.radius = 0.3
    assert state.radius == 0.3
    state.strength_neighbor_repulsion = 5.0
    assert state.strength_neighbor_repulsion == 5.0

    state.range_neighbor_repulsion = 0.6
    assert state.range_neighbor_repulsion == 0.6

    state.strength_geometry_repulsion = 4.2
    assert state.strength_geometry_repulsion == 4.2

    state.range_geometry_repulsion = 0.8
    assert state.range_geometry_repulsion == 0.8

    state.range_x_scale = 15.0
    assert state.range_x_scale == 15.0
    state.range_y_scale = 6.0
    assert state.range_y_scale == 6.0

    state.theta_max_upper_bound = 0.9
    assert state.theta_max_upper_bound == 0.9

    state.agent_buffer = 0.5
    assert state.agent_buffer == 0.5


def test_collision_free_speed_model_v3_does_not_back_into_a_wall():
    """An agent whose spacing to the neighbor ahead is below its
    agent_buffer backs up, but stops at the wall behind it instead of
    walking into it."""
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        polygon=shapely.Polygon([(-0.5, -2), (20, -2), (20, 2), (-0.5, 2)]),
        height=0.0,
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV3(),
        geometry=surface.build(),
        dt=0.01,
    )
    exit_ = sim.add_target(
        jps.Exit(
            [(shapely.Polygon([(19, -1), (20, -1), (20, 1), (19, 1)]), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)])

    def add(position, desired_speed, agent_buffer):
        return sim.add_agent(
            room.at(*position),
            route=route,
            state=jps.CollisionFreeSpeedModelV3State(
                radius=0.15,
                time_gap=1.0,
                desired_speed=desired_speed,
                agent_buffer=agent_buffer,
            ),
        )

    backing = add((-0.3, 0), desired_speed=0.5, agent_buffer=1.0)
    add((0.1, 0), desired_speed=0.0, agent_buffer=0.0)
    for _ in range(3000):
        sim.iterate()
    assert -0.5 + 0.15 <= backing.location.x < -0.3


def test_anticipation_velocity_model_does_not_back_into_a_wall():
    """An agent closer to the neighbor ahead than their radiuses plus buffer
    backs up, but stops at the wall behind it instead of walking into it."""
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        polygon=shapely.Polygon([(-0.5, -2), (20, -2), (20, 2), (-0.5, 2)]),
        height=0.0,
    )
    sim = jps.Simulation(
        model=jps.AnticipationVelocityModel(),
        geometry=surface.build(),
        dt=0.01,
    )
    exit_ = sim.add_target(
        jps.Exit(
            [(shapely.Polygon([(19, -1), (20, -1), (20, 1), (19, 1)]), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)])

    def add(position, desired_speed):
        return sim.add_agent(
            room.at(*position),
            route=route,
            # Neighbor repulsion would turn the agent sideways.
            state=jps.AnticipationVelocityModelState(
                radius=0.15,
                time_gap=1.0,
                desired_speed=desired_speed,
                strength_neighbor_repulsion=0.0,
            ),
        )

    # 5 mm in front of the wall; the spacing to the neighbor is -0.015 m.
    backing = add((-0.345, 0), desired_speed=1.0)
    add((-0.04, 0), desired_speed=0.0)
    for _ in range(3000):
        sim.iterate()
    assert -0.5 + 0.15 <= backing.location.x < -0.345


# (model, state class, state kwargs, closest the centre may come to the wall)
_WALL_APPROACH = [
    (jps.CollisionFreeSpeedModel, jps.CollisionFreeSpeedModelState, {}, 0.2),
    (
        jps.CollisionFreeSpeedModelV2,
        jps.CollisionFreeSpeedModelV2State,
        {},
        0.2,
    ),
    (
        jps.CollisionFreeSpeedModelV3,
        jps.CollisionFreeSpeedModelV3State,
        {
            "strength_neighbor_repulsion": 8.0,
            "range_neighbor_repulsion": 0.1,
            "strength_geometry_repulsion": 5.0,
            "range_geometry_repulsion": 0.02,
        },
        0.2,
    ),
    (
        jps.AnticipationVelocityModel,
        jps.AnticipationVelocityModelState,
        {},
        0.2,
    ),
    (jps.SocialForceModel, jps.SocialForceModelState, {}, 0.3),
]


@pytest.mark.parametrize(
    "start_y, target_x", [(3.0, 8.0), (1.0, 19.0)], ids=["steep", "shallow"]
)
@pytest.mark.parametrize(
    "model, state_cls, state_kwargs, body",
    _WALL_APPROACH,
    ids=[model.__name__ for model, _, _, _ in _WALL_APPROACH],
)
def test_agent_steered_at_a_wall_stays_out_of_it(
    model, state_cls, state_kwargs, body, start_y, target_x
):
    """An agent steered at a point 2 cm in front of the wall y = 0 walks
    obliquely towards the wall. The wall keeps its body out: its centre
    never comes closer than `body`."""
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(0, 0), (20, 0), (20, 4), (0, 4)], height=0.0
    )
    sim = jps.Simulation(model=model(), geometry=surface.build())
    agent = sim.add_agent(
        room.at(1, start_y),
        steer_to=room.at(target_x, 0.02),
        state=state_cls(**state_kwargs),
    )
    closest = start_y
    for _ in range(3000):
        sim.iterate()
        closest = min(closest, agent.location.y)
    assert closest >= body
