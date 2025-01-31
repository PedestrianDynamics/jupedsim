# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest


@pytest.fixture
def simulation_with_social_force_model():
    return jps.Simulation(
        model=jps.SocialForceModel(),
        geometry=[(0, 0), (10, 0), (10, 10), (0, 10)],
    )


def test_desired_speed_deprecated(simulation_with_social_force_model):
    sim = simulation_with_social_force_model
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.SocialForceModelAgentParameters(
        journey_id=journey_id,
        stage_id=wp,
        position=(1, 1),
    )
    agent_id = sim.add_agent(agent)
    # Check if the deprecation warning is raised
    with pytest.warns(
        DeprecationWarning, match="deprecated, use 'desired_speed' instead"
    ):
        sim.agent(agent_id).model.desiredSpeed = 1.5
        assert sim.agent(agent_id).model.desiredSpeed == 1.5

    # Verify the new snake_case property reflects the same value
    assert sim.agent(agent_id).model.desired_speed == 1.5
