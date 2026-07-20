# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import shapely
from jupedsim_examples.models.py_wrapped_social_force import (
    WrappedSocialForceModel,
    WrappedSocialForceModelState,
)

# A cluster of interacting agents: pairwise distances inside the 2.5 m
# social-force cutoff and close enough (0.7-1.0 m) for non-trivial forces, but
# outside the contact zone (0.6 m) so the dynamics stay bounded.
_AGENT_POSITIONS = [
    (2.0, 9.6),
    (2.0, 10.4),
    (2.9, 10.0),
    (3.7, 9.65),
    (3.7, 10.35),
]


def _build_simulation(model, make_state):
    geometry = shapely.Polygon([(0, 0), (40, 0), (40, 20), (0, 20)])
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage([(39, 9), (39, 11), (40, 11), (40, 9)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    agent_ids = [
        sim.add_agent(
            journey_id=journey_id, stage_id=exit_id, state=make_state(position)
        )
        for position in _AGENT_POSITIONS
    ]
    return sim, agent_ids


def test_wrapped_social_force_is_bit_identical_to_native():
    """The forwarding wrapper must reproduce the native model exactly.

    Both simulations share geometry, journey, dt, agent parameters, and
    insertion order; positions and velocities are compared with exact float
    equality every iteration. Any mismatch indicates a defect in the model
    exposure machinery, not an acceptable tolerance.
    """
    native_sim, native_ids = _build_simulation(
        jps.SocialForceModel(),
        lambda position: jps.SocialForceModelState(position=position),
    )
    wrapped_sim, wrapped_ids = _build_simulation(
        WrappedSocialForceModel(),
        lambda position: WrappedSocialForceModelState.create(position=position),
    )

    for iteration in range(100):
        native_sim.iterate()
        wrapped_sim.iterate()
        assert native_sim.agent_count() == wrapped_sim.agent_count()
        for native_id, wrapped_id in zip(native_ids, wrapped_ids):
            native_agent = native_sim.agent(native_id)
            wrapped_agent = wrapped_sim.agent(wrapped_id)
            assert native_agent.position == wrapped_agent.position, (
                f"position diverged at iteration {iteration}: "
                f"{native_agent.position} != {wrapped_agent.position}"
            )
            wrapped_sfm_state = wrapped_agent.model.sfm_state
            assert wrapped_agent.position == wrapped_sfm_state.position
            assert native_agent.model.velocity == wrapped_sfm_state.velocity, (
                f"velocity diverged at iteration {iteration}"
            )


def test_wrapped_social_force_forwards_constraint_checks():
    """Adding an overlapping agent must raise through the wrapped model."""
    sim, _ = _build_simulation(
        WrappedSocialForceModel(),
        lambda position: WrappedSocialForceModelState.create(position=position),
    )
    exit_id = sim.add_exit_stage([(39, 1), (39, 3), (40, 3), (40, 1)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    import pytest

    with pytest.raises(jps.SimulationError, match="too close to agent"):
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            state=WrappedSocialForceModelState.create(
                position=_AGENT_POSITIONS[0]
            ),
        )
