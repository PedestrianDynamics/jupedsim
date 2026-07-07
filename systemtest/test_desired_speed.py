# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim as jps
import pytest

model_params = [
    (
        jps.SocialForceModel,
        jps.SocialForceModelState,
        {},
        ["desiredSpeed", "v0"],
    ),
    (
        jps.CollisionFreeSpeedModel,
        jps.CollisionFreeSpeedModelState,
        {},
        ["desiredSpeed", "v0"],
    ),
    (
        jps.CollisionFreeSpeedModelV2,
        jps.CollisionFreeSpeedModelV2State,
        {},
        ["desiredSpeed", "v0"],
    ),
    (
        jps.GeneralizedCentrifugalForceModel,
        jps.GeneralizedCentrifugalForceModelState,
        {"orientation": (1, 0)},
        ["desiredSpeed", "v0"],
    ),
]


@pytest.fixture
def create_simulation():
    """Creates a generic simulation fixture for different models."""

    def _create(model_class):
        return jps.Simulation(
            model=model_class(),
            geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
        )

    return _create


@pytest.mark.parametrize(
    "model_class, state_class, extra_state_kwargs, removed_attrs", model_params
)
def test_desired_speed_can_be_set_via_state(
    create_simulation,
    model_class,
    state_class,
    extra_state_kwargs,
    removed_attrs,
):
    """Test that desired_speed passed to the state constructor is applied."""
    sim = create_simulation(model_class)
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=state_class(
            position=(1, 1), desired_speed=1.5, **extra_state_kwargs
        ),
    )

    assert sim.agent(agent_id).model.desired_speed == 1.5


@pytest.mark.parametrize(
    "model_class, state_class, extra_state_kwargs, removed_attrs", model_params
)
def test_desired_speed_can_be_mutated_via_agent_handle(
    create_simulation,
    model_class,
    state_class,
    extra_state_kwargs,
    removed_attrs,
):
    """Test that desired_speed can be changed through the agent handle."""
    sim = create_simulation(model_class)
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=state_class(position=(1, 1), **extra_state_kwargs),
    )

    sim.agent(agent_id).model.desired_speed = 1.5

    assert sim.agent(agent_id).model.desired_speed == 1.5


@pytest.mark.parametrize(
    "model_class, state_class, extra_state_kwargs, removed_attrs", model_params
)
def test_removed_parameter_names_raise(
    create_simulation,
    model_class,
    state_class,
    extra_state_kwargs,
    removed_attrs,
):
    """Test that pre-2.0 parameter names are gone without replacement."""
    sim = create_simulation(model_class)
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    for removed_attr in removed_attrs:
        with pytest.raises(TypeError):
            state_class(
                position=(1, 1),
                **{removed_attr: 1.5},
                **extra_state_kwargs,
            )

    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=wp,
        state=state_class(position=(1, 1), **extra_state_kwargs),
    )

    for removed_attr in removed_attrs:
        with pytest.raises(AttributeError):
            getattr(sim.agent(agent_id).model, removed_attr)
