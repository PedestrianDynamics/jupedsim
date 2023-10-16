# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import pytest

import jupedsim as jps


def test_collision_free_speed_model_constraints():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )

    exit_id = simulation.add_exit_stage(
        [(99, 45), (99, 55), (100, 55), (100, 45)]
    )

    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)

    agent_position = (50, 50)

    # Radius
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius 0 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=0,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius -1 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=-1,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius 2.1 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=2.1,
            )
        )

    # v0
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 0 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=0,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 -1 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=-1,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=10.1,
            )
        )

    # time gap
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: timeGap 0.09 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                time_gap=0.09,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: timeGap 10.1 .*"
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                time_gap=10.1,
            )
        )

    # too close to wall
    radius = 0.2
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance <= \d+\.?\d*",
    ):
        assert simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=(0 + 0.5 * radius, 10),
                journey_id=journey_id,
                stage_id=exit_id,
                radius=radius,
            )
        )


def test_generalized_centrifugal_force_constraints():
    messages = []

    def log_msg_handler(msg):
        messages.append(msg)

    jps.set_info_callback(log_msg_handler)
    jps.set_warning_callback(log_msg_handler)
    jps.set_error_callback(log_msg_handler)

    simulation = jps.Simulation(
        model=jps.GeneralizedCentrifugalForceModel(),
        geometry=[(0, 0), (100, 0), (100, 100), (0, 100)],
    )

    exit_id = simulation.add_exit_stage(
        [(99, 45), (99, 55), (100, 55), (100, 45)]
    )

    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)

    agent_position = (50, 50)

    # mass
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: mass [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                mass=0.99,
            )
        )

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: mass [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                mass=100.1,
            )
        )

    # tau
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: tau [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                tau=0.09,
            )
        )

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: tau [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                tau=10.1,
            )
        )

    # v0
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 [+-]?\d+\.?\d* .+"
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=0,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 [+-]?\d+\.?\d* .+"
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=-1,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 [+-]?\d+\.?\d* .+"
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=10.1,
            )
        )

    # Av
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: Av [+-]?\d+\.?\d* .+"
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_v=-0.001,
            )
        )

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: Av [+-]?\d+\.?\d* .+"
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_v=10.1,
            )
        )

    # AMin
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: AMin [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_min=0.099,
            )
        )

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: AMin [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_min=1.1,
            )
        )

    # BMin
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: BMin [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_min=0.099,
            )
        )

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: BMin [+-]?\d+\.?\d* .+",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_min=1.1,
            )
        )

    # BMax
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: BMax [+-]?\d+\.?\d* .+",
    ):
        b_min = 0.3

        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_min=b_min,
                b_max=b_min - 0.001,
            )
        )

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: BMax [+-]?\d+\.?\d* .+",
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_max=2.01,
            )
        )

    # too close to wall
    a_min = 0.3
    b_max = a_min
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance <= \d+\.?\d*",
    ):
        assert simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=(0 + 0.5 * a_min, 10),
                journey_id=journey_id,
                stage_id=exit_id,
                a_min=a_min,
                b_max=b_max,
            )
        )
