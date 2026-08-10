# SPDX-License-Identifier: LGPL-3.0-or-later
"""Two operational models side by side in one simulation.

The simulation runs a single model, a dispatcher, whose agents carry the state
of the sub-model they belong to. Each sub-model is handed a step in which the
neighbors carry the state type that sub-model expects.
"""

from dataclasses import dataclass, replace

import jupedsim as jps
import pytest
import shapely
from jupedsim_examples.models.pysocial_force import (
    PythonSocialForceModel,
    PythonSocialForceModelState,
)


@dataclass(kw_only=True, frozen=True)
class MultiModelState:
    """Per-agent state of the dispatcher: which sub-model, and its state."""

    kind: str
    sub: object


class MultiModel(jps.CustomOperationalModel):
    """Dispatches each agent to its sub-model.

    A sub-model must never see a ``MultiModelState``, so every neighbor is
    mapped to the state type the sub-model reads. The mappings are the
    modelling decision this class makes: they say what an agent of one model
    looks like to an agent of the other.
    """

    def __init__(self):
        super().__init__()
        self._social_force = PythonSocialForceModel()
        self._collision_free = jps.CollisionFreeSpeedModelV3()

    @staticmethod
    def _as_social_force_state(neighbor):
        """How a neighbor is seen by the social force model.

        It reads ``radius`` and ``velocity``. CollisionFreeSpeedModelV3 has no
        velocity -- it stores a heading only -- so the neighbor's speed has to
        be reconstructed from its orientation and desired speed.
        """
        if neighbor.kind == "social_force":
            return neighbor.sub
        sub = neighbor.sub
        return PythonSocialForceModelState(
            velocity=(
                sub.orientation[0] * sub.desired_speed,
                sub.orientation[1] * sub.desired_speed,
            ),
            radius=sub.radius,
        )

    @staticmethod
    def _as_collision_free_state(neighbor):
        """How a neighbor is seen by the collision free speed model.

        It reads ``radius`` of a neighbor and nothing else.
        """
        if neighbor.kind == "collision_free":
            return neighbor.sub
        return jps.CollisionFreeSpeedModelV3State(radius=neighbor.sub.radius)

    def compute_next_state(self, state, step):
        if state.kind == "social_force":
            sub_step = step.with_neighbor_state_mapping(
                self._as_social_force_state
            )
            sub, movement = self._social_force.compute_next_state(
                state.sub, sub_step
            )
        else:
            sub_step = step.with_neighbor_state_mapping(
                self._as_collision_free_state
            )
            sub, movement = self._collision_free.compute_next_state(
                state.sub, sub_step
            )
        return replace(state, sub=sub), movement


def _corridor():
    return shapely.Polygon([(0, 0), (30, 0), (30, 8), (0, 8)])


def _make_sim(model):
    sim = jps.Simulation(model=model, geometry=_corridor(), dt=0.01)
    exit_id = sim.add_exit_stage([(29, 3), (29, 5), (30, 5), (30, 3)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return sim, exit_id, journey_id


def _social_force_agent():
    return MultiModelState(
        kind="social_force",
        sub=PythonSocialForceModelState(velocity=(0.0, 0.0)),
    )


def _collision_free_agent():
    return MultiModelState(
        kind="collision_free",
        sub=jps.CollisionFreeSpeedModelV3State(),
    )


def _add(sim, journey_id, stage_id, position, state):
    return sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=position,
        state=state,
    )


def test_both_models_advance_in_one_simulation():
    sim, exit_id, journey_id = _make_sim(MultiModel())
    left = _add(sim, journey_id, exit_id, (2.0, 3.0), _social_force_agent())
    right = _add(sim, journey_id, exit_id, (2.0, 5.0), _collision_free_agent())

    start = (sim.agent(left).position, sim.agent(right).position)
    for _ in range(200):
        sim.iterate()
    end = (sim.agent(left).position, sim.agent(right).position)

    assert end[0][0] > start[0][0] + 0.5
    assert end[1][0] > start[1][0] + 0.5


def test_each_agent_keeps_its_own_state_type():
    sim, exit_id, journey_id = _make_sim(MultiModel())
    left = _add(sim, journey_id, exit_id, (2.0, 3.0), _social_force_agent())
    right = _add(sim, journey_id, exit_id, (2.0, 5.0), _collision_free_agent())

    for _ in range(10):
        sim.iterate()

    assert isinstance(sim.agent(left).state.sub, PythonSocialForceModelState)
    assert isinstance(
        sim.agent(right).state.sub, jps.CollisionFreeSpeedModelV3State
    )


def test_the_models_see_each_other():
    """A collision free agent must react to a social force agent in its way."""
    sim, exit_id, journey_id = _make_sim(MultiModel())
    blocked = _add(
        sim, journey_id, exit_id, (2.0, 4.0), _collision_free_agent()
    )
    _add(sim, journey_id, exit_id, (2.6, 4.0), _social_force_agent())
    for _ in range(50):
        sim.iterate()
    with_neighbor = sim.agent(blocked).position

    free_sim, free_exit, free_journey = _make_sim(MultiModel())
    alone = _add(
        free_sim, free_journey, free_exit, (2.0, 4.0), _collision_free_agent()
    )
    for _ in range(50):
        free_sim.iterate()
    without_neighbor = free_sim.agent(alone).position

    assert with_neighbor != without_neighbor
    assert with_neighbor[0] < without_neighbor[0]


def test_an_unmapped_neighbor_is_reported():
    """Forgetting the mapping must fail loudly, not silently misbehave."""

    class _Forgetful(MultiModel):
        def compute_next_state(self, state, step):
            if state.kind == "social_force":
                return super().compute_next_state(state, step)
            sub, movement = self._collision_free.compute_next_state(
                state.sub, step
            )
            return replace(state, sub=sub), movement

    sim, exit_id, journey_id = _make_sim(_Forgetful())
    _add(sim, journey_id, exit_id, (2.0, 4.0), _collision_free_agent())
    _add(sim, journey_id, exit_id, (2.6, 4.0), _social_force_agent())

    with pytest.raises(jps.SimulationError, match="with_neighbor_states"):
        sim.iterate()
