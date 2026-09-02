# SPDX-License-Identifier: LGPL-3.0-or-later
"""A Python model delegating one step to a built-in C++ model.

Step 1 of the multi-model prototype: only the call surface is exercised, with a
single agent so that no neighbor state is ever looked up.
"""

from dataclasses import dataclass, replace

import jupedsim as jps
import pytest
import shapely


@dataclass(kw_only=True, frozen=True)
class _Wrapped:
    """Custom state that carries a built-in model state as its payload."""

    sub: object


class _DelegatingModel(jps.CustomOperationalModel):
    """Hands the whole step over to a built-in model and passes on its result.

    With ``repack_neighbors``, neighbors are mapped to the state type the inner
    model expects; without it, the inner model sees the stored custom states.
    """

    def __init__(self, inner, *, repack_neighbors=False):
        super().__init__()
        self._inner = inner
        self._repack_neighbors = repack_neighbors

    def compute_next_state(self, state, step):
        if self._repack_neighbors:
            step = step.with_neighbor_state_mapping(
                lambda neighbor: neighbor.sub
            )
        sub, movement = self._inner.compute_next_state(state.sub, step)
        return replace(state, sub=sub), movement


def _open_room():
    return shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])


def _make_sim(model):
    sim = jps.Simulation(model=model, geometry=_open_room(), dt=0.05)
    exit_id = sim.add_exit_stage([(19, 9), (19, 11), (20, 11), (20, 9)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return sim, exit_id, journey_id


def _run(sim, journey_id, stage_id, state, steps=40):
    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=(1.0, 10.0),
        state=state,
    )
    trajectory = [sim.agent(agent_id).position]
    for _ in range(steps):
        sim.iterate()
        trajectory.append(sim.agent(agent_id).position)
    return trajectory


def test_delegated_model_walks_towards_the_exit():
    sim, exit_id, journey_id = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    trajectory = _run(
        sim,
        journey_id,
        exit_id,
        _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert trajectory[-1][0] > trajectory[0][0] + 1.0


def test_delegation_matches_the_native_model_exactly():
    """The delegated run must reproduce the built-in model step for step."""
    native_sim, native_exit, native_journey = _make_sim(
        jps.CollisionFreeSpeedModelV3()
    )
    native = _run(
        native_sim,
        native_journey,
        native_exit,
        jps.CollisionFreeSpeedModelV3State(),
    )

    delegating_sim, delegating_exit, delegating_journey = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    delegated = _run(
        delegating_sim,
        delegating_journey,
        delegating_exit,
        _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert delegated == native


def test_delegated_state_is_advanced():
    """The sub-state returned by the built-in model is a new, updated object."""
    initial = jps.CollisionFreeSpeedModelV3State()
    sim, exit_id, journey_id = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    # Off the exit axis, so that the model has to turn and the change is visible.
    agent_id = sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(1.0, 3.0),
        state=_Wrapped(sub=initial),
    )
    sim.iterate()

    sub = sim.agent(agent_id).state.sub
    assert sub is not initial
    assert sub.orientation != initial.orientation


def test_wrong_state_type_is_reported_with_both_names():
    sim, exit_id, journey_id = _make_sim(
        _DelegatingModel(jps.SocialForceModel(body_force=0.0, friction=0.0))
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=(1.0, 10.0),
        state=_Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    with pytest.raises(
        jps.SimulationError, match="SocialForceModel.*CollisionFreeSpeedModelV3"
    ):
        sim.iterate()


def _run_pair(sim, journey_id, stage_id, state_of, steps=40):
    """Two agents on a collision course, so that neighbor states are read."""
    left = sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=(1.0, 10.0),
        state=state_of(),
    )
    ahead = sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=(2.2, 10.0),
        state=state_of(),
    )
    trajectory = []
    for _ in range(steps):
        sim.iterate()
        trajectory.append((sim.agent(left).position, sim.agent(ahead).position))
    return trajectory


def test_neighbors_are_unreadable_without_the_hook():
    """The failure the hook exists for, and the message that points at it."""
    sim, exit_id, journey_id = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    with pytest.raises(jps.SimulationError, match="with_neighbor_states"):
        _run_pair(
            sim,
            journey_id,
            exit_id,
            lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
            steps=1,
        )


def test_repacked_neighbors_match_the_native_model_exactly():
    """Two interacting agents, mapped 1:1 -- must reproduce the built-in model."""
    native_sim, native_exit, native_journey = _make_sim(
        jps.CollisionFreeSpeedModelV3()
    )
    native = _run_pair(
        native_sim,
        native_journey,
        native_exit,
        jps.CollisionFreeSpeedModelV3State,
    )

    sim, exit_id, journey_id = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3(), repack_neighbors=True)
    )
    delegated = _run_pair(
        sim,
        journey_id,
        exit_id,
        lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert delegated == native


def test_repacked_neighbors_are_actually_seen():
    """A mapping that inflates neighbors must change the outcome."""

    def _fat(_):
        return jps.CollisionFreeSpeedModelV3State(radius=0.5)

    sim, exit_id, journey_id = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3(), repack_neighbors=True)
    )
    lean = _run_pair(
        sim,
        journey_id,
        exit_id,
        lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    class _FatNeighbors(_DelegatingModel):
        def compute_next_state(self, state, step):
            step = step.with_neighbor_state_mapping(_fat)
            sub, movement = self._inner.compute_next_state(state.sub, step)
            return replace(state, sub=sub), movement

    fat_sim, fat_exit, fat_journey = _make_sim(
        _FatNeighbors(jps.CollisionFreeSpeedModelV3())
    )
    fat = _run_pair(
        fat_sim,
        fat_journey,
        fat_exit,
        lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert fat != lean


def test_a_wrong_mapping_blames_the_mapping():
    """With a mapping in place, the error must point at it, not at its absence."""

    class _Broken(_DelegatingModel):
        def compute_next_state(self, state, step):
            step = step.with_neighbor_state_mapping(lambda _: "not a state")
            sub, movement = self._inner.compute_next_state(state.sub, step)
            return replace(state, sub=sub), movement

    sim, exit_id, journey_id = _make_sim(
        _Broken(jps.CollisionFreeSpeedModelV3())
    )
    with pytest.raises(
        jps.SimulationError,
        match="mapping passed to with_neighbor_states.. has to return",
    ):
        _run_pair(
            sim,
            journey_id,
            exit_id,
            lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
            steps=1,
        )


def test_step_argument_must_be_a_step():
    model = jps.CollisionFreeSpeedModelV3()
    with pytest.raises(jps.SimulationError, match="expects the step"):
        model.compute_next_state(jps.CollisionFreeSpeedModelV3State(), 42)
