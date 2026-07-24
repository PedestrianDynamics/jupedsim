# SPDX-License-Identifier: LGPL-3.0-or-later
from dataclasses import dataclass, replace

import jupedsim as jps
import shapely
from jupedsim.models.custom_model import (
    CustomModelAgentState,
    CustomOperationalModel,
)


@dataclass(kw_only=True, frozen=True)
class _State(CustomModelAgentState):
    position: tuple[float, float]
    group: int = 0


def _open_room():
    return shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])


def _make_sim(model, geometry=None):
    if geometry is None:
        geometry = _open_room()
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage([(19, 9), (19, 11), (20, 11), (20, 9)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return sim, exit_id, journey_id


def _add_agent(sim, journey_id, stage_id, position, group=0):
    return sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=_State(position=position, group=group),
    )


def _walled_room():
    """20x20 room with a thin vertical wall from y=0 to y=15 at x=10.
    Agents on the left (x<9.9) cannot see across to the right (x>10.1)
    below y=15; above y=15 the gap gives free sight lines."""
    outer = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    wall = shapely.Polygon([(9.9, 0), (10.1, 0), (10.1, 15), (9.9, 15)])
    return outer.difference(wall)


class _CapturingModel(CustomOperationalModel):
    """Calls agents_in_range for one designated probe agent and records results."""

    def __init__(self, probe_position, *, radius=5.0, predicate=None):
        super().__init__()
        self._probe_pos = probe_position
        self._radius = radius
        self._predicate = predicate
        self.neighbor_positions: list[tuple] = []
        self.predicate_positions: list[tuple] = []

    def _is_probe(self, ped):
        px, py = ped.position
        ppx, ppy = self._probe_pos
        return abs(px - ppx) < 1e-4 and abs(py - ppy) < 1e-4

    def compute_next_state(self, dt, ped, env_query):
        if self._is_probe(ped):
            self.predicate_positions.clear()

            def _tracking(neighbor):
                self.predicate_positions.append(tuple(neighbor.position))
                return self._predicate(neighbor) if self._predicate else True

            neighbors = env_query.agents_in_range(ped, self._radius, _tracking)
            self.neighbor_positions = [tuple(n.position) for n in neighbors]
        return replace(ped.model, position=ped.model.position)


# ---------------------------------------------------------------------------
# Basic filter tests
# ---------------------------------------------------------------------------


def test_no_predicate_returns_all_in_radius():
    model = _CapturingModel((2.0, 10.0), radius=5.0)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0))  # probe
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))  # 1.0 m away — in range
    _add_agent(sim, journey_id, exit_id, (4.0, 10.0))  # 2.0 m away — in range
    _add_agent(
        sim, journey_id, exit_id, (15.0, 10.0)
    )  # 13.0 m away — out of range

    sim.iterate()

    assert len(model.neighbor_positions) == 2
    assert (3.0, 10.0) in model.neighbor_positions
    assert (4.0, 10.0) in model.neighbor_positions


def test_reject_all_predicate_returns_empty():
    model = _CapturingModel((2.0, 10.0), radius=5.0, predicate=lambda _: False)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0))
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))
    _add_agent(sim, journey_id, exit_id, (4.0, 10.0))

    sim.iterate()

    assert model.neighbor_positions == []


def test_group_filter_returns_only_matching_group():
    def same_group(neighbor):
        return neighbor.model.group == 1

    model = _CapturingModel((2.0, 10.0), radius=5.0, predicate=same_group)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(
        sim, journey_id, exit_id, (2.0, 10.0), group=1
    )  # probe — not a neighbor of itself
    _add_agent(
        sim, journey_id, exit_id, (3.0, 10.0), group=1
    )  # same group → kept
    _add_agent(
        sim, journey_id, exit_id, (4.0, 10.0), group=0
    )  # different group → filtered out
    _add_agent(
        sim, journey_id, exit_id, (2.0, 11.0), group=1
    )  # same group → kept

    sim.iterate()

    assert len(model.neighbor_positions) == 2
    assert (3.0, 10.0) in model.neighbor_positions
    assert (2.0, 11.0) in model.neighbor_positions


def test_predicate_never_called_with_self():
    probe_pos = (2.0, 10.0)
    # accept-all predicate — only here to trigger tracking
    model = _CapturingModel(probe_pos, radius=100.0, predicate=lambda _: True)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, probe_pos)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))

    sim.iterate()

    assert probe_pos not in model.predicate_positions


# ---------------------------------------------------------------------------
# no_wall_between tests
# ---------------------------------------------------------------------------


def test_no_wall_between_filters_occluded_agents():
    """Agents behind the wall should be excluded by no_wall_between."""

    class _VisModel(CustomOperationalModel):
        def __init__(self):
            super().__init__()
            self.neighbor_positions: list[tuple] = []

        def compute_next_state(self, dt, ped, env_query):
            px, py = ped.position
            if abs(px - 5.0) < 1e-4 and abs(py - 10.0) < 1e-4:
                self.neighbor_positions = [
                    tuple(n.position)
                    for n in env_query.agents_in_range(
                        ped,
                        20.0,
                        lambda n: env_query.no_wall_between(
                            ped.position, n.position
                        ),
                    )
                ]
            return replace(ped.model, position=ped.model.position)

    model = _VisModel()
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (5.0, 10.0))  # probe
    _add_agent(sim, journey_id, exit_id, (5.0, 5.0))  # same side, visible
    _add_agent(sim, journey_id, exit_id, (15.0, 10.0))  # behind wall, occluded

    sim.iterate()

    assert (5.0, 5.0) in model.neighbor_positions
    assert (15.0, 10.0) not in model.neighbor_positions


def test_no_wall_between_agent_above_wall_is_seen():
    """Agents whose connecting line stays above the wall top (y>15) are visible.

    Probe is at (5, 18) and target at (15, 18): the horizontal path y=18 never
    crosses the wall (which ends at y=15), so the target must be returned.
    """

    class _VisModel(CustomOperationalModel):
        def __init__(self):
            super().__init__()
            self.neighbor_positions: list[tuple] = []

        def compute_next_state(self, _dt, ped, env_query):
            px, py = ped.position
            if abs(px - 5.0) < 1e-4 and abs(py - 18.0) < 1e-4:
                self.neighbor_positions = [
                    tuple(n.position)
                    for n in env_query.agents_in_range(
                        ped,
                        20.0,
                        lambda n: env_query.no_wall_between(
                            ped.position, n.position
                        ),
                    )
                ]
            return replace(ped.model, position=ped.model.position)

    model = _VisModel()
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (5.0, 18.0))  # probe — above wall top
    _add_agent(
        sim, journey_id, exit_id, (15.0, 18.0)
    )  # far side, same height → visible

    sim.iterate()

    assert (15.0, 18.0) in model.neighbor_positions


def test_composed_no_wall_between_and_group_filter():
    """Combining no_wall_between with a group predicate using a lambda."""

    class _ComposedModel(CustomOperationalModel):
        def __init__(self):
            super().__init__()
            self.neighbor_positions: list[tuple] = []

        def compute_next_state(self, _dt, ped, env_query):
            px, py = ped.position
            if abs(px - 5.0) < 1e-4 and abs(py - 10.0) < 1e-4:
                self.neighbor_positions = [
                    tuple(n.position)
                    for n in env_query.agents_in_range(
                        ped,
                        20.0,
                        lambda neighbor: (
                            env_query.no_wall_between(
                                ped.position, neighbor.position
                            )
                            and neighbor.model.group == 1
                        ),
                    )
                ]
            return replace(ped.model, position=ped.model.position)

    model = _ComposedModel()
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (5.0, 10.0), group=0)  # probe
    _add_agent(
        sim, journey_id, exit_id, (5.0, 15.0), group=1
    )  # visible, group 1 → kept
    _add_agent(
        sim, journey_id, exit_id, (5.0, 5.0), group=0
    )  # visible, group 0 → filtered
    _add_agent(
        sim, journey_id, exit_id, (15.0, 10.0), group=1
    )  # occluded, group 1 → filtered

    sim.iterate()

    assert model.neighbor_positions == [(5.0, 15.0)]
