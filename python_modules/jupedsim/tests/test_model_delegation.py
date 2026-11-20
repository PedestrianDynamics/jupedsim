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

    def seed_agent_state(self, state):
        return replace(state, sub=self._inner.seed_state(state.sub))


class _NonSeedingDelegatingModel(_DelegatingModel):
    """Forgets to forward the seeding to its inner model."""

    def seed_agent_state(self, state):
        return state


def _open_room():
    return shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])


def _make_sim(model):
    surface = jps.GeometryBuilder()
    room = surface.add_region(polygon=_open_room(), height=0.0)
    sim = jps.Simulation(model=model, geometry=surface.build(), dt=0.05)
    exit_ = sim.add_target(
        jps.Exit([([(19, 9), (19, 11), (20, 11), (20, 9)], room)])
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, exit_)])
    return sim, route, room


def _run(sim, room, route, state, steps=40):
    agent = sim.add_agent(
        room.at(1.0, 10.0),
        route=route,
        state=state,
    )
    trajectory = [agent.location.xy]
    for _ in range(steps):
        sim.iterate()
        trajectory.append(agent.location.xy)
    return trajectory


def test_delegated_model_walks_towards_the_exit():
    sim, route, room = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    trajectory = _run(
        sim,
        room,
        route,
        _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert trajectory[-1][0] > trajectory[0][0] + 1.0


def test_delegation_matches_the_native_model_exactly():
    """The delegated run must reproduce the built-in model step for step."""
    native_sim, native_route, native_room = _make_sim(
        jps.CollisionFreeSpeedModelV3()
    )
    native = _run(
        native_sim,
        native_room,
        native_route,
        jps.CollisionFreeSpeedModelV3State(),
    )

    delegating_sim, delegating_route, delegating_room = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    delegated = _run(
        delegating_sim,
        delegating_room,
        delegating_route,
        _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert delegated == native


def test_delegated_state_is_advanced():
    """The sub-state returned by the built-in model is a new, updated object."""
    initial = jps.CollisionFreeSpeedModelV3State()
    sim, route, room = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    # Off the exit axis, so that the model has to turn and the change is visible.
    agent = sim.add_agent(
        room.at(1.0, 3.0),
        route=route,
        state=_Wrapped(sub=initial),
    )
    sim.iterate()

    sub = agent.state.sub
    assert sub is not initial
    assert sub.orientation != initial.orientation


def test_wrong_state_type_is_reported_with_both_names():
    sim, route, room = _make_sim(
        _NonSeedingDelegatingModel(
            jps.SocialForceModel(body_force=0.0, friction=0.0)
        )
    )
    sim.add_agent(
        room.at(1.0, 10.0),
        route=route,
        state=_Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    with pytest.raises(
        jps.SimulationError, match="SocialForceModel.*CollisionFreeSpeedModelV3"
    ):
        sim.iterate()


def _run_pair(sim, room, route, state_of, steps=40):
    """Two agents on a collision course, so that neighbor states are read."""
    left = sim.add_agent(
        room.at(1.0, 10.0),
        route=route,
        state=state_of(),
    )
    ahead = sim.add_agent(
        room.at(2.2, 10.0),
        route=route,
        state=state_of(),
    )
    trajectory = []
    for _ in range(steps):
        sim.iterate()
        trajectory.append((left.location.xy, ahead.location.xy))
    return trajectory


def test_neighbors_are_unreadable_without_the_hook():
    """The failure the hook exists for, and the message that points at it."""
    sim, route, room = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3())
    )
    with pytest.raises(
        jps.SimulationError, match="with_neighbor_state_mapping"
    ):
        _run_pair(
            sim,
            room,
            route,
            lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
            steps=1,
        )


def test_repacked_neighbors_match_the_native_model_exactly():
    """Two interacting agents, mapped 1:1 -- must reproduce the built-in model."""
    native_sim, native_route, native_room = _make_sim(
        jps.CollisionFreeSpeedModelV3()
    )
    native = _run_pair(
        native_sim,
        native_room,
        native_route,
        jps.CollisionFreeSpeedModelV3State,
    )

    sim, route, room = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3(), repack_neighbors=True)
    )
    delegated = _run_pair(
        sim,
        room,
        route,
        lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    assert delegated == native


def test_repacked_neighbors_are_actually_seen():
    """A mapping that inflates neighbors must change the outcome."""

    def _fat(_):
        return jps.CollisionFreeSpeedModelV3State(radius=0.5)

    sim, route, room = _make_sim(
        _DelegatingModel(jps.CollisionFreeSpeedModelV3(), repack_neighbors=True)
    )
    lean = _run_pair(
        sim,
        room,
        route,
        lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
    )

    class _FatNeighbors(_DelegatingModel):
        def compute_next_state(self, state, step):
            step = step.with_neighbor_state_mapping(_fat)
            sub, movement = self._inner.compute_next_state(state.sub, step)
            return replace(state, sub=sub), movement

    fat_sim, fat_route, fat_room = _make_sim(
        _FatNeighbors(jps.CollisionFreeSpeedModelV3())
    )
    fat = _run_pair(
        fat_sim,
        fat_room,
        fat_route,
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

    sim, route, room = _make_sim(_Broken(jps.CollisionFreeSpeedModelV3()))
    with pytest.raises(
        jps.SimulationError,
        match="mapping passed to with_neighbor_state_mapping.. has to return",
    ):
        _run_pair(
            sim,
            room,
            route,
            lambda: _Wrapped(sub=jps.CollisionFreeSpeedModelV3State()),
            steps=1,
        )


def test_step_argument_must_be_a_step():
    model = jps.CollisionFreeSpeedModelV3()
    with pytest.raises(jps.SimulationError, match="expects the step"):
        model.compute_next_state(jps.CollisionFreeSpeedModelV3State(), 42)


def test_delegated_random_streams_match_the_native_model_exactly():
    """WarpDriver draws every step, so any stream mixup shows in the trajectories."""
    native_sim, native_route, native_room = _make_sim(
        jps.WarpDriverModel(rng_seed=7)
    )
    native = _run_pair(
        native_sim,
        native_room,
        native_route,
        jps.WarpDriverModelState,
    )

    sim, route, room = _make_sim(
        _DelegatingModel(jps.WarpDriverModel(rng_seed=7), repack_neighbors=True)
    )
    delegated = _run_pair(
        sim,
        room,
        route,
        lambda: _Wrapped(sub=jps.WarpDriverModelState()),
    )

    assert delegated == native


def test_the_inner_seed_reaches_delegated_agents():
    # That the same seed gives the same run follows from the exact match with the native model.
    def run(seed):
        sim, route, room = _make_sim(
            _DelegatingModel(
                jps.WarpDriverModel(rng_seed=seed), repack_neighbors=True
            )
        )
        return _run_pair(
            sim,
            room,
            route,
            lambda: _Wrapped(sub=jps.WarpDriverModelState()),
        )

    assert run(7) != run(8)


@pytest.mark.parametrize(
    "inner, state",
    [
        (jps.WarpDriverModel, jps.WarpDriverModelState),
        (jps.AnticipationVelocityModel, jps.AnticipationVelocityModelState),
    ],
)
def test_a_forgotten_seed_forward_is_reported(inner, state):
    sim, route, room = _make_sim(_NonSeedingDelegatingModel(inner()))
    with pytest.raises(jps.SimulationError, match="never seeded"):
        _run(sim, room, route, _Wrapped(sub=state()), steps=1)


def test_seed_state_rejects_a_foreign_state():
    with pytest.raises(
        jps.SimulationError, match="WarpDriverModel cannot seed a state"
    ):
        jps.WarpDriverModel().seed_state(jps.CollisionFreeSpeedModelV3State())


@pytest.mark.parametrize(
    "seed_agent_state, returned",
    [
        (lambda model, state: None, "None"),
        (
            lambda model, state: model._inner.seed_state(state.sub),
            "WarpDriverModelState",
        ),
    ],
    ids=["forgotten_return", "inner_state"],
)
def test_seed_agent_state_must_return_the_agents_state(
    seed_agent_state, returned
):
    class _Broken(_DelegatingModel):
        def seed_agent_state(self, state):
            return seed_agent_state(self, state)

    sim, route, room = _make_sim(_Broken(jps.WarpDriverModel()))
    with pytest.raises(
        jps.SimulationError,
        match=f"must return a state of type _Wrapped, got .*{returned}",
    ):
        sim.add_agent(
            room.at(1.0, 10.0),
            route=route,
            state=_Wrapped(sub=jps.WarpDriverModelState()),
        )
    assert sim.agent_count() == 0
