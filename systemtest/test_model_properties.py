# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import pytest

import jupedsim as jps


@pytest.fixture
def corridor():
    geometry = jps.geometry_from_coordinates(
        [(0, 0), (10, 0), (10, 2), (0, 2)]
    )
    return jps.Simulation(
        model=jps.VelocityModelParameters(), geometry=geometry, dt=0.01
    )


def test_set_v0(corridor):
    sim = corridor
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.VelocityModelAgentParameters()
    agent.journey_id = journey_id
    agent.stage_id = wp
    agent.position = (1, 1)
    agent.orientation = (1, 0)
    agent.time_gap = 1
    agent.tau = 0.5
    agent.v0 = 1
    # This should not be required
    agent.e0 = (1, 0)
    agent.radius = 0.15
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
