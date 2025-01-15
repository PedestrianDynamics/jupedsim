# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim as jps
import numpy as np
import pytest


@pytest.fixture
def square_room_100x100():
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )

    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])

    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)
    agent_position = (0, 0)

    return simulation, journey_id, exit_id, agent_position


@pytest.fixture
def square_room_100x100_gcfm():
    simulation = jps.Simulation(
        model=jps.GeneralizedCentrifugalForceModel(),
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )

    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])

    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)
    agent_position = (0, 0)

    return simulation, journey_id, exit_id, agent_position


@pytest.mark.parametrize(
    "radius",
    np.arange(0.01, 2, 0.5),
)
def test_collision_free_speed_model_can_set_radius(square_room_100x100, radius):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            radius=radius,
        )
    )


def test_collision_free_speed_model_can_not_set_radius_too_small(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius -1 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=-1,
            )
        )


def test_collision_free_speed_model_can_not_set_radius_zero(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius 0 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=0,
            )
        )


def test_collision_free_speed_model_can_not_set_radius_too_large(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius 2.1 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=2.1,
            )
        )


@pytest.mark.parametrize("v0", np.arange(0.0, 10, 1))
def test_collision_free_speed_model_can_set_v0(square_room_100x100, v0):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            v0=v0,
        )
    )


def test_collision_free_speed_model_can_not_set_v0_too_small(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 -1 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=-1,
            )
        )


def test_collision_free_speed_model_can_not_set_v0_too_large(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=10.1,
            )
        )


@pytest.mark.parametrize("time_gap", [*np.arange(0.1, 10, 1), 10])
def test_collision_free_speed_model_can_set_time_gap(
    square_room_100x100, time_gap
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            time_gap=time_gap,
        )
    )


def test_collision_free_speed_model_can_not_set_time_gap_too_small(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: timeGap -1 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                time_gap=-1,
            )
        )


def test_collision_free_speed_model_can_not_set_time_too_large(
    square_room_100x100,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: timeGap 10.1 .*"
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                time_gap=10.1,
            )
        )


@pytest.mark.parametrize("radius", np.arange(0.1, 0.5, 0.1))
def test_collision_free_speed_model_can_not_add_agent_too_close_to_wall(
    square_room_100x100,
    radius,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance .*",
    ):
        simulation.add_agent(
            jps.CollisionFreeSpeedModelAgentParameters(
                position=(50 - (0.99 * radius), 0),
                journey_id=journey_id,
                stage_id=exit_id,
                radius=radius,
            )
        )


@pytest.mark.parametrize("mass", [*np.arange(1, 100, 20), 100])
def test_generalized_centrifugal_force_model_can_set_mass(
    square_room_100x100_gcfm, mass
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            mass=mass,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_mass_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: mass 0 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                mass=0,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_mass_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: mass 100.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                mass=100.1,
            )
        )


@pytest.mark.parametrize("tau", [*np.arange(0.1, 10, 1), 10])
def test_generalized_centrifugal_force_model_can_set_tau(
    square_room_100x100_gcfm, tau
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            tau=tau,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_tau_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: tau 0 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                tau=0,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_tau_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: tau 10.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                tau=10.1,
            )
        )


@pytest.mark.parametrize("v0", [*np.arange(0, 10, 1), 10])
def test_generalized_centrifugal_force_model_can_set_v0(
    square_room_100x100_gcfm, v0
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            v0=v0,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_v0_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 -0.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=-0.1,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_v0_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=10.1,
            )
        )


@pytest.mark.parametrize("a_v", [*np.arange(0.1, 10, 1), 10])
def test_generalized_centrifugal_force_model_can_set_a_v(
    square_room_100x100_gcfm, a_v
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            a_v=a_v,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_a_v_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: Av -0.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_v=-0.1,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_a_v_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: Av 10.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_v=10.1,
            )
        )


@pytest.mark.parametrize("a_min", [*np.arange(0.1, 1, 0.1), 1])
def test_generalized_centrifugal_force_model_can_set_a_min(
    square_room_100x100_gcfm, a_min
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            a_min=a_min,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_a_min_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: AMin 0.099 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_min=0.099,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_a_min_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: AMin 1.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                a_min=1.1,
            )
        )


@pytest.mark.parametrize("b_min", [*np.arange(0.1, 1, 0.1), 1])
def test_generalized_centrifugal_force_model_can_set_b_min(
    square_room_100x100_gcfm, b_min
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            b_min=b_min,
            b_max=b_min,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_b_min_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: BMin 0.099 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_min=0.099,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_b_min_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: BMin 1.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_min=1.1,
            )
        )


@pytest.mark.parametrize("b_max", [*np.arange(0.2, 2, 0.3), 2])
def test_generalized_centrifugal_force_model_can_set_b_max(
    square_room_100x100_gcfm, b_max
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            b_max=b_max,
        )
    )


def test_generalized_centrifugal_force_model_can_not_set_b_max_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: BMax 0.39 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_min=0.4,
                b_max=0.39,
            )
        )


def test_generalized_centrifugal_force_model_can_not_set_b_max_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: BMax 2.1 .*"
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                b_max=2.1,
            )
        )


@pytest.mark.parametrize("radius", np.arange(0.1, 0.5, 0.1))
def test_generalized_centrifugal_force_model_can_not_add_agent_too_close_to_wall(
    square_room_100x100_gcfm,
    radius,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    a_min = 0.3
    b_max = a_min
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance <= \d+\.?\d*",
    ):
        simulation.add_agent(
            jps.GeneralizedCentrifugalForceModelAgentParameters(
                position=(-50 + 0.5 * a_min, 10),
                journey_id=journey_id,
                stage_id=exit_id,
                a_min=a_min,
                b_max=b_max,
            )
        )


@pytest.fixture
def square_room_100x100_avm():
    simulation = jps.Simulation(
        model=jps.AnticipationVelocityModel(),
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )

    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])

    journey = jps.JourneyDescription([exit_id])
    journey_id = simulation.add_journey(journey)
    agent_position = (0, 0)

    return simulation, journey_id, exit_id, agent_position


@pytest.mark.parametrize("strength", [*np.arange(0.1, 10, 1)])
def test_anticipation_velocity_model_can_set_strength_neighbor_repulsion(
    square_room_100x100_avm, strength
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            strength_neighbor_repulsion=strength,
        )
    )


def test_anticipation_velocity_model_can_not_set_strength_neighbor_repulsion_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: strengthNeighborRepulsion -0.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                strength_neighbor_repulsion=-0.1,
            )
        )


def test_anticipation_velocity_model_can_not_set_strength_neighbor_repulsion_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: strengthNeighborRepulsion 20.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                strength_neighbor_repulsion=20.1,
            )
        )


@pytest.mark.parametrize("range_repulsion", [*np.arange(0.1, 5, 1)])
def test_anticipation_velocity_model_can_set_range_neighbor_repulsion(
    square_room_100x100_avm, range_repulsion
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            range_neighbor_repulsion=range_repulsion,
        )
    )


def test_anticipation_velocity_model_can_not_set_range_neighbor_repulsion_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: rangeNeighborRepulsion 0 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                range_neighbor_repulsion=0,
            )
        )


def test_anticipation_velocity_model_can_not_set_range_neighbor_repulsion_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: rangeNeighborRepulsion 5.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                range_neighbor_repulsion=5.1,
            )
        )


@pytest.mark.parametrize("wall_buffer", [*np.arange(0.1, 1, 0.2)])
def test_anticipation_velocity_model_can_set_wall_buffer_distance(
    square_room_100x100_avm, wall_buffer
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            wall_buffer_distance=wall_buffer,
        )
    )


def test_anticipation_velocity_model_can_not_set_wall_buffer_distance_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: wallBufferDistance -0.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                wall_buffer_distance=-0.1,
            )
        )


def test_anticipation_velocity_model_can_not_set_wall_buffer_distance_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: wallBufferDistance 2.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                wall_buffer_distance=2.1,
            )
        )


@pytest.mark.parametrize("anticipation_time", [*np.arange(0.1, 5, 0.5)])
def test_anticipation_velocity_model_can_set_anticipation_time(
    square_room_100x100_avm, anticipation_time
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            anticipation_time=anticipation_time,
        )
    )


def test_anticipation_velocity_model_can_not_set_anticipation_time_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: anticipationTime -0.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                anticipation_time=-0.1,
            )
        )


def test_anticipation_velocity_model_can_not_set_anticipation_time_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: anticipationTime 5.1 .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                anticipation_time=5.1,
            )
        )


@pytest.mark.parametrize("reaction_time", [*np.arange(0.1, 1, 0.2)])
def test_anticipation_velocity_model_can_set_reaction_time(
    square_room_100x100_avm, reaction_time
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            reaction_time=reaction_time,
        )
    )


def test_anticipation_velocity_model_can_not_set_reaction_time_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: reactionTime 0 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                reaction_time=0,
            )
        )


def test_anticipation_velocity_model_can_not_set_reaction_time_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: reactionTime 2.1 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                reaction_time=2.1,
            )
        )


@pytest.mark.parametrize("time_gap", [*np.arange(0.1, 10, 1)])
def test_anticipation_velocity_model_can_set_time_gap(
    square_room_100x100_avm, time_gap
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            time_gap=time_gap,
        )
    )


def test_anticipation_velocity_model_can_not_set_time_gap_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: timeGap 0 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                time_gap=0,
            )
        )


def test_anticipation_velocity_model_can_not_set_time_gap_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: timeGap 10.1 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                time_gap=10.1,
            )
        )


@pytest.mark.parametrize("v0", [*np.arange(0.1, 10, 1)])
def test_anticipation_velocity_model_can_set_v0(square_room_100x100_avm, v0):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            v0=v0,
        )
    )


def test_anticipation_velocity_model_can_not_set_v0_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 -0.1 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=-0.1,
            )
        )


def test_anticipation_velocity_model_can_not_set_v0_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                v0=10.1,
            )
        )


@pytest.mark.parametrize("radius", [*np.arange(0.1, 2, 0.2)])
def test_anticipation_velocity_model_can_set_radius(
    square_room_100x100_avm, radius
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        jps.AnticipationVelocityModelAgentParameters(
            position=agent_position,
            journey_id=journey_id,
            stage_id=exit_id,
            radius=radius,
        )
    )


def test_anticipation_velocity_model_can_not_set_radius_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius 0 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=0,
            )
        )


def test_anticipation_velocity_model_can_not_set_radius_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        RuntimeError, match=r"Model constraint violation: radius 2.1 .*"
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=agent_position,
                journey_id=journey_id,
                stage_id=exit_id,
                radius=2.1,
            )
        )


@pytest.mark.parametrize("radius", [*np.arange(0.1, 0.5, 0.1)])
def test_anticipation_velocity_model_can_not_add_agent_too_close_to_wall(
    square_room_100x100_avm,
    radius,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    with pytest.raises(
        RuntimeError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance .*",
    ):
        simulation.add_agent(
            jps.AnticipationVelocityModelAgentParameters(
                position=(50 - (0.99 * radius), 0),
                journey_id=journey_id,
                stage_id=exit_id,
                radius=radius,
            )
        )
