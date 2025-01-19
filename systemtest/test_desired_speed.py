# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim as jps
import pytest

deprecated_params = [
    (
        jps.SocialForceModel,
        jps.SocialForceModelAgentParameters,
        "desiredSpeed",
        "desired_speed",
    ),
    (
        jps.SocialForceModel,
        jps.SocialForceModelAgentParameters,
        "reactionTime",
        "reaction_time",
    ),
    (
        jps.SocialForceModel,
        jps.SocialForceModelAgentParameters,
        "agentScale",
        "agent_scale",
    ),
    (
        jps.SocialForceModel,
        jps.SocialForceModelAgentParameters,
        "obstacleScale",
        "obstacle_scale",
    ),
    (
        jps.SocialForceModel,
        jps.SocialForceModelAgentParameters,
        "forceDistance",
        "force_distance",
    ),
    (
        jps.CollisionFreeSpeedModel,
        jps.CollisionFreeSpeedModelAgentParameters,
        "v0",
        "desired_speed",
    ),
    (
        jps.CollisionFreeSpeedModelV2,
        jps.CollisionFreeSpeedModelV2AgentParameters,
        "v0",
        "desired_speed",
    ),
    (
        jps.GeneralizedCentrifugalForceModel,
        jps.GeneralizedCentrifugalForceModelAgentParameters,
        "v0",
        "desired_speed",
    ),
]


@pytest.fixture
def create_simulation():
    """Creates a generic simulation fixture for different models."""

    def _create(model):
        return jps.Simulation(
            model=model,
            geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
        )

    return _create


@pytest.mark.parametrize(
    "model_class, agent_class, deprecated_attr, new_attr", deprecated_params
)
def test_deprecated_constructor_parameter(
    create_simulation,
    model_class,
    agent_class,
    deprecated_attr,
    new_attr,
):
    """Test that passing a deprecated parameter to the agent constructor."""
    sim = create_simulation(model_class())
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    kwargs = {
        "journey_id": journey_id,
        "stage_id": wp,
        "position": (1, 1),
        deprecated_attr: 1.5,
    }

    with pytest.warns(
        DeprecationWarning, match=f"deprecated, use '{new_attr}' instead"
    ):
        agent = agent_class(**kwargs)

    agent_id = sim.add_agent(agent)

    assert getattr(sim.agent(agent_id).model, new_attr) == 1.5


@pytest.mark.parametrize(
    "model_class, agent_class, deprecated_attr, new_attr", deprecated_params
)
def test_get_on_deprecated_params(
    create_simulation, model_class, agent_class, deprecated_attr, new_attr
):
    """
    Test that deprecated 'desiredSpeed' (or 'v0' in some models) raises warnings and is mapped to 'desired_speed'.
    """
    sim = create_simulation(model_class())
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = agent_class(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    with pytest.warns(
        DeprecationWarning, match=f"deprecated, use '{new_attr}' instead"
    ):
        setattr(sim.agent(agent_id).model, new_attr, 1.5)
        assert getattr(sim.agent(agent_id).model, deprecated_attr) == 1.5
        sim.agent(agent_id).model.v0 = 1.5

    assert getattr(sim.agent(agent_id).model, new_attr) == 1.5


@pytest.mark.parametrize(
    "model_class, agent_class, deprecated_attr, new_attr", deprecated_params
)
def test_set_on_deprecated_params(
    create_simulation, model_class, agent_class, deprecated_attr, new_attr
):
    """
    Test that deprecated 'desiredSpeed' (or 'v0' in some models) raises warnings and is mapped to 'desired_speed'.
    """
    sim = create_simulation(model_class())
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = agent_class(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    with pytest.warns(
        DeprecationWarning, match=f"deprecated, use '{new_attr}' instead"
    ):
        setattr(sim.agent(agent_id).model, deprecated_attr, 1.5)
        assert getattr(sim.agent(agent_id).model, new_attr) == 1.5

    assert getattr(sim.agent(agent_id).model, new_attr) == 1.5


def test_foo(create_simulation):
    sim = create_simulation(jps.SocialForceModel())
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.SocialForceModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)

    with pytest.warns(
        DeprecationWarning, match="deprecated, use 'desired_speed' instead"
    ):
        print(sim.agent(agent_id).model)
        print(sim.agent(agent_id).model.desiredSpeed)
