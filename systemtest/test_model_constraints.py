# SPDX-License-Identifier: LGPL-3.0-or-later
import dataclasses

import jupedsim as jps
import numpy as np
import pytest


@pytest.fixture
def square_room_100x100_cfsm():
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
def test_collision_free_speed_model_can_set_radius(
    square_room_100x100_cfsm, radius
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.CollisionFreeSpeedModelState(
            position=agent_position, radius=radius
        ),
    )


def test_collision_free_speed_model_can_not_set_radius_too_small(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: radius -1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, radius=-1
            ),
        )


def test_collision_free_speed_model_can_not_set_radius_zero(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: radius 0 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, radius=0
            ),
        )


def test_collision_free_speed_model_can_not_set_radius_too_large(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: radius 2.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, radius=2.1
            ),
        )


@pytest.mark.parametrize("desired_speed", np.arange(0.0, 10, 1))
def test_collision_free_speed_model_can_set_desired_speed(
    square_room_100x100_cfsm, desired_speed
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.CollisionFreeSpeedModelState(
            position=agent_position, desired_speed=desired_speed
        ),
    )


def test_collision_free_speed_model_can_not_set_desired_speed_too_small(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: v0 -1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, desired_speed=-1
            ),
        )


def test_collision_free_speed_model_can_not_set_desired_speed_too_large(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, desired_speed=10.1
            ),
        )


@pytest.mark.parametrize("time_gap", [*np.arange(0.1, 10, 1), 10])
def test_collision_free_speed_model_can_set_time_gap(
    square_room_100x100_cfsm, time_gap
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.CollisionFreeSpeedModelState(
            position=agent_position, time_gap=time_gap
        ),
    )


def test_collision_free_speed_model_can_not_set_time_gap_too_small(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: timeGap -1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, time_gap=-1
            ),
        )


def test_collision_free_speed_model_can_not_set_time_too_large(
    square_room_100x100_cfsm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: timeGap 10.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=agent_position, time_gap=10.1
            ),
        )


@pytest.mark.parametrize("radius", np.arange(0.1, 0.5, 0.1))
def test_collision_free_speed_model_can_not_add_agent_too_close_to_wall(
    square_room_100x100_cfsm,
    radius,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_cfsm

    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.CollisionFreeSpeedModelState(
                position=(50 - (0.99 * radius), 0), radius=radius
            ),
        )


@pytest.mark.parametrize("mass", [*np.arange(1, 100, 20), 100])
def test_generalized_centrifugal_force_model_can_set_mass(
    square_room_100x100_gcfm, mass
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0), position=agent_position, mass=mass
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_mass_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: mass 0 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, mass=0
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_mass_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: mass 100.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, mass=100.1
            ),
        )


@pytest.mark.parametrize("tau", [*np.arange(0.1, 10, 1), 10])
def test_generalized_centrifugal_force_model_can_set_tau(
    square_room_100x100_gcfm, tau
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0), position=agent_position, tau=tau
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_tau_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: tau 0 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, tau=0
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_tau_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: tau 10.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, tau=10.1
            ),
        )


@pytest.mark.parametrize("desired_speed", [*np.arange(0, 10, 1), 10])
def test_generalized_centrifugal_force_model_can_set_desired_speed(
    square_room_100x100_gcfm, desired_speed
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0),
            position=agent_position,
            desired_speed=desired_speed,
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_desired_speed_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: v0 -0.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0),
                position=agent_position,
                desired_speed=-0.1,
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_desired_speed_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0),
                position=agent_position,
                desired_speed=10.1,
            ),
        )


@pytest.mark.parametrize("a_v", [*np.arange(0.1, 10, 1), 10])
def test_generalized_centrifugal_force_model_can_set_a_v(
    square_room_100x100_gcfm, a_v
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0), position=agent_position, a_v=a_v
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_a_v_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: Av -0.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, a_v=-0.1
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_a_v_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: Av 10.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, a_v=10.1
            ),
        )


@pytest.mark.parametrize("a_min", [*np.arange(0.1, 1, 0.1), 1])
def test_generalized_centrifugal_force_model_can_set_a_min(
    square_room_100x100_gcfm, a_min
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0), position=agent_position, a_min=a_min
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_a_min_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: AMin 0.099 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, a_min=0.099
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_a_min_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: AMin 1.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, a_min=1.1
            ),
        )


@pytest.mark.parametrize("b_min", [*np.arange(0.1, 1, 0.1), 1])
def test_generalized_centrifugal_force_model_can_set_b_min(
    square_room_100x100_gcfm, b_min
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0),
            position=agent_position,
            b_min=b_min,
            b_max=b_min,
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_b_min_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: BMin 0.099 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, b_min=0.099
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_b_min_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: BMin 1.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, b_min=1.1
            ),
        )


@pytest.mark.parametrize("b_max", [*np.arange(0.2, 2, 0.3), 2])
def test_generalized_centrifugal_force_model_can_set_b_max(
    square_room_100x100_gcfm, b_max
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.GeneralizedCentrifugalForceModelState(
            orientation=(1.0, 0.0), position=agent_position, b_max=b_max
        ),
    )


def test_generalized_centrifugal_force_model_can_not_set_b_max_too_small(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: BMax 0.39 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0),
                position=agent_position,
                b_min=0.4,
                b_max=0.39,
            ),
        )


def test_generalized_centrifugal_force_model_can_not_set_b_max_too_large(
    square_room_100x100_gcfm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_gcfm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: BMax 2.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0), position=agent_position, b_max=2.1
            ),
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
        jps.SimulationError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance <= \d+\.?\d*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.GeneralizedCentrifugalForceModelState(
                orientation=(1.0, 0.0),
                position=(-50 + 0.5 * a_min, 10),
                a_min=a_min,
                b_max=b_max,
            ),
        )


@pytest.fixture
def square_room_100x100_avm():
    simulation = jps.Simulation(
        model=jps.AnticipationVelocityModel(rng_seed=1234),
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
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, strength_neighbor_repulsion=strength
        ),
    )


def test_anticipation_velocity_model_can_not_set_strength_neighbor_repulsion_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: strengthNeighborRepulsion -0.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, strength_neighbor_repulsion=-0.1
            ),
        )


def test_anticipation_velocity_model_can_not_set_strength_neighbor_repulsion_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: strengthNeighborRepulsion 20.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, strength_neighbor_repulsion=20.1
            ),
        )


@pytest.mark.parametrize("range_repulsion", [*np.arange(0.1, 5, 1)])
def test_anticipation_velocity_model_can_set_range_neighbor_repulsion(
    square_room_100x100_avm, range_repulsion
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, range_neighbor_repulsion=range_repulsion
        ),
    )


def test_anticipation_velocity_model_can_not_set_range_neighbor_repulsion_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: rangeNeighborRepulsion 0 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, range_neighbor_repulsion=0
            ),
        )


def test_anticipation_velocity_model_can_not_set_range_neighbor_repulsion_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: rangeNeighborRepulsion 5.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, range_neighbor_repulsion=5.1
            ),
        )


@pytest.mark.parametrize("wall_buffer", [*np.arange(0.1, 1, 0.2)])
def test_anticipation_velocity_model_can_set_wall_buffer_distance(
    square_room_100x100_avm, wall_buffer
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, wall_buffer_distance=wall_buffer
        ),
    )


def test_anticipation_velocity_model_can_not_set_wall_buffer_distance_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: wallBufferDistance -0.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, wall_buffer_distance=-0.1
            ),
        )


def test_anticipation_velocity_model_can_not_set_wall_buffer_distance_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: wallBufferDistance 2.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, wall_buffer_distance=2.1
            ),
        )


@pytest.mark.parametrize("anticipation_time", [*np.arange(0.1, 5, 0.5)])
def test_anticipation_velocity_model_can_set_anticipation_time(
    square_room_100x100_avm, anticipation_time
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, anticipation_time=anticipation_time
        ),
    )


def test_anticipation_velocity_model_can_not_set_anticipation_time_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: anticipationTime -0.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, anticipation_time=-0.1
            ),
        )


def test_anticipation_velocity_model_can_not_set_anticipation_time_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: anticipationTime 5.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, anticipation_time=5.1
            ),
        )


@pytest.mark.parametrize("reaction_time", [*np.arange(0.1, 1, 0.2)])
def test_anticipation_velocity_model_can_set_reaction_time(
    square_room_100x100_avm, reaction_time
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, reaction_time=reaction_time
        ),
    )


def test_anticipation_velocity_model_can_not_set_reaction_time_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: reactionTime 0 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, reaction_time=0
            ),
        )


def test_anticipation_velocity_model_can_not_set_reaction_time_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: reactionTime 2.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, reaction_time=2.1
            ),
        )


@pytest.mark.parametrize("time_gap", [*np.arange(0.1, 10, 1)])
def test_anticipation_velocity_model_can_set_time_gap(
    square_room_100x100_avm, time_gap
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, time_gap=time_gap
        ),
    )


def test_anticipation_velocity_model_can_not_set_time_gap_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: timeGap 0 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, time_gap=0
            ),
        )


def test_anticipation_velocity_model_can_not_set_time_gap_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: timeGap 10.1 .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, time_gap=10.1
            ),
        )


@pytest.mark.parametrize("desired_speed", [*np.arange(0.1, 10, 1)])
def test_anticipation_velocity_model_can_set_desired_speed(
    square_room_100x100_avm, desired_speed
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, desired_speed=desired_speed
        ),
    )


def test_anticipation_velocity_model_can_not_set_desired_speed_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: v0 -0.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, desired_speed=-0.1
            ),
        )


def test_anticipation_velocity_model_can_not_set_desired_speed_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: v0 10.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, desired_speed=10.1
            ),
        )


@pytest.mark.parametrize("radius", [*np.arange(0.1, 2, 0.2)])
def test_anticipation_velocity_model_can_set_radius(
    square_room_100x100_avm, radius
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=jps.AnticipationVelocityModelState(
            position=agent_position, radius=radius
        ),
    )


def test_anticipation_velocity_model_can_not_set_radius_too_small(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: radius 0 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, radius=0
            ),
        )


def test_anticipation_velocity_model_can_not_set_radius_too_large(
    square_room_100x100_avm,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm
    with pytest.raises(
        jps.SimulationError, match=r"Model constraint violation: radius 2.1 .*"
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=agent_position, radius=2.1
            ),
        )


@pytest.mark.parametrize("radius", [*np.arange(0.1, 0.5, 0.1)])
def test_anticipation_velocity_model_can_not_add_agent_too_close_to_wall(
    square_room_100x100_avm,
    radius,
):
    simulation, journey_id, exit_id, agent_position = square_room_100x100_avm

    with pytest.raises(
        jps.SimulationError,
        match=r"Model constraint violation: Agent (.+) too close to geometry boundaries, distance .*",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.AnticipationVelocityModelState(
                position=(50 - (0.99 * radius), 0), radius=radius
            ),
        )


@dataclasses.dataclass(frozen=True)
class _MinimalCustomState:
    position: tuple[float, float]


class _MinimalCustomModel(jps.CustomOperationalModel):
    def compute_next_state(
        self, dt, state, destination, geometry, neighborhood_search
    ):
        return _MinimalCustomState(position=state.position)


def test_add_agent_rejects_state_of_different_model_on_custom_simulation():
    simulation = jps.Simulation(
        model=_MinimalCustomModel(),
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )
    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))

    with pytest.raises(
        jps.SimulationError,
        match="does not match the simulation's operational model",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=jps.SocialForceModelState(position=(0, 0)),
        )


def test_add_agent_rejects_custom_state_on_builtin_model_simulation():
    simulation = jps.Simulation(
        model=jps.SocialForceModel(),
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )
    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))

    with pytest.raises(
        jps.SimulationError,
        match="does not match the simulation's operational model",
    ):
        simulation.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=_MinimalCustomState(position=(0, 0)),
        )


def test_add_agent_rejects_object_without_position():
    simulation = jps.Simulation(
        model=jps.SocialForceModel(),
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )
    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))

    with pytest.raises(TypeError, match="CustomModelAgentState"):
        simulation.add_agent(
            journey_id=journey_id, stage_id=exit_id, state=object()
        )


class _NoPositionState:
    pass


class _NoPositionModel(jps.CustomOperationalModel):
    def compute_next_state(
        self, dt, state, destination, geometry, neighborhood_search
    ):
        return _NoPositionState()


class _WrongPositionTypeModel(jps.CustomOperationalModel):
    def compute_next_state(
        self, dt, state, destination, geometry, neighborhood_search
    ):
        return _MinimalCustomState(position="not-a-tuple")


def _custom_model_simulation_with_agent(model):
    simulation = jps.Simulation(
        model=model,
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )
    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))
    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=_MinimalCustomState(position=(0, 0)),
    )
    return simulation


def test_custom_model_update_missing_position_names_source():
    simulation = _custom_model_simulation_with_agent(_NoPositionModel())

    with pytest.raises(
        jps.SimulationError,
        match=r"State returned by compute_next_state\(\) is missing the 'position' attribute",
    ):
        simulation.iterate()


def test_custom_model_update_wrong_position_type_names_source():
    simulation = _custom_model_simulation_with_agent(_WrongPositionTypeModel())

    with pytest.raises(
        jps.SimulationError,
        match=r"State returned by compute_next_state\(\) has attribute 'position' of wrong type",
    ):
        simulation.iterate()


class _AddAgentDuringIterateModel(jps.CustomOperationalModel):
    """Calls add_agent from within the compute callback."""

    def __init__(self):
        self.simulation = None
        self.journey_id = None
        self.stage_id = None
        self.error = None

    def compute_next_state(
        self, dt, state, destination, geometry, neighborhood_search
    ):
        try:
            self.simulation.add_agent(
                journey_id=self.journey_id,
                stage_id=self.stage_id,
                state=_MinimalCustomState(position=(5, 5)),
            )
        except Exception as e:
            self.error = e
        return _MinimalCustomState(position=state.position)


def test_add_agent_during_iterate_raises_simulation_error():
    model = _AddAgentDuringIterateModel()
    simulation = jps.Simulation(
        model=model,
        geometry=[(-50, -50), (50, -50), (50, 50), (-50, 50)],
    )
    exit_id = simulation.add_exit_stage([(49, -3), (49, 3), (50, 3), (50, -3)])
    journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))
    model.simulation = simulation
    model.journey_id = journey_id
    model.stage_id = exit_id
    simulation.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        state=_MinimalCustomState(position=(0, 0)),
    )

    simulation.iterate()

    assert isinstance(model.error, jps.SimulationError)
    assert "AddAgent is not allowed during iteration" in str(model.error)
    # The re-entrant call must not have added an agent.
    assert simulation.agent_count() == 1
