# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest


@pytest.fixture
def corridor():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (10, 0), (10, 2), (0, 2)],
    )


def test_set_v0(corridor):
    sim = corridor
    wp = sim.add_waypoint_stage((10, 1), 0.5)
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))

    agent = jps.CollisionFreeSpeedModelAgentParameters(
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
