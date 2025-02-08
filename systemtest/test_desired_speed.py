# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim as jps
import pytest


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
    "model_class, agent_class, deprecated_attr",
    [
        (
            jps.SocialForceModel,
            jps.SocialForceModelAgentParameters,
            "desiredSpeed",
        ),
        (
            jps.CollisionFreeSpeedModel,
            jps.CollisionFreeSpeedModelAgentParameters,
            "v0",
        ),
        (
            jps.CollisionFreeSpeedModelV2,
            jps.CollisionFreeSpeedModelV2AgentParameters,
            "v0",
        ),
        (
            jps.GeneralizedCentrifugalForceModel,
            jps.GeneralizedCentrifugalForceModelAgentParameters,
            "v0",
        ),
    ],
)
def test_deprecated_constructor_parameter(
    create_simulation,
    model_class,
    agent_class,
    deprecated_attr,
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
        DeprecationWarning, match="deprecated, use 'desired_speed' instead"
    ):
        agent = agent_class(**kwargs)

    agent_id = sim.add_agent(agent)

    assert sim.agent(agent_id).model.desired_speed == 1.5


def test_changing_desired_speed_deprecated(
    create_simulation, model_class, agent_class, deprecated_attr
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
        DeprecationWarning, match="deprecated, use 'desired_speed' instead"
    ):
        setattr(sim.agent(agent_id).model, deprecated_attr, 1.5)
        assert getattr(sim.agent(agent_id).model, deprecated_attr) == 1.5

    assert sim.agent(agent_id).model.desired_speed == 1.5
