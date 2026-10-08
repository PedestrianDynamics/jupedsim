# SPDX-License-Identifier: LGPL-3.0-or-later
import math
from dataclasses import dataclass, replace

import jupedsim as jps
import pytest
import shapely

_STAY_PUT = (0.0, 0.0)


@dataclass(kw_only=True, frozen=True)
class _State:
    group: int = 0
    probe: bool = False


def _open_room():
    return shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])


def _make_sim(model, geometry=None):
    if geometry is None:
        geometry = _open_room()
    sim = jps.Simulation(model=model, geometry=geometry, dt=0.05)
    exit_id = sim.add_exit_stage(
        [(19, 9), (19, 11), (20, 11), (20, 9)], region_id=0
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    return sim, exit_id, journey_id


def _add_agent(sim, journey_id, stage_id, position, group=0, probe=False):
    return sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=position,
        state=_State(group=group, probe=probe),
        region_id=0,
    )


def _walled_room():
    """20x20 room with a thin vertical wall from y=0 to y=15 at x=10.
    Agents on the left (x<9.9) cannot see across to the right (x>10.1)
    below y=15; above y=15 the gap gives free sight lines."""
    outer = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    wall = shapely.Polygon([(9.9, 0), (10.1, 0), (10.1, 15), (9.9, 15)])
    return outer.difference(wall)


class _CapturingModel(jps.CustomOperationalModel):
    """Calls other_agents_in_range for the probe agent and records results."""

    def __init__(self, *, radius=5.0, predicate=None):
        """predicate is called as ``(step, neighbor) -> bool``."""
        super().__init__()
        self._radius = radius
        self._predicate = predicate
        self.neighbors: list[tuple] = []
        self.predicate_neighbors: list[tuple] = []

    def compute_next_state(self, state, step):
        if state.probe:
            self.predicate_neighbors.clear()

            def _tracking(neighbor):
                self.predicate_neighbors.append(neighbor.relative_position)
                if self._predicate is None:
                    return True
                return self._predicate(step, neighbor)

            neighbors = step.other_agents_in_range(self._radius, _tracking)
            self.neighbors = [n.relative_position for n in neighbors]
        return replace(state), _STAY_PUT


# ---------------------------------------------------------------------------
# Basic filter tests
# ---------------------------------------------------------------------------


def test_no_predicate_returns_all_in_radius():
    model = _CapturingModel(radius=5.0)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))  # 1.0 m away — in range
    _add_agent(sim, journey_id, exit_id, (4.0, 10.0))  # 2.0 m away — in range
    _add_agent(
        sim, journey_id, exit_id, (15.0, 10.0)
    )  # 13.0 m away — out of range

    sim.iterate()

    assert len(model.neighbors) == 2
    assert (1.0, 0.0) in model.neighbors
    assert (2.0, 0.0) in model.neighbors


def test_reject_all_predicate_returns_empty():
    model = _CapturingModel(radius=5.0, predicate=lambda step, n: False)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))
    _add_agent(sim, journey_id, exit_id, (4.0, 10.0))

    sim.iterate()

    assert model.neighbors == []


def test_group_filter_returns_only_matching_group():
    model = _CapturingModel(
        radius=5.0, predicate=lambda step, n: n.state.group == 1
    )
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(
        sim, journey_id, exit_id, (2.0, 10.0), group=1, probe=True
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

    assert len(model.neighbors) == 2
    assert (1.0, 0.0) in model.neighbors
    assert (0.0, 1.0) in model.neighbors


def test_predicate_never_called_with_self():
    # accept-all predicate — only here to trigger tracking
    model = _CapturingModel(radius=100.0, predicate=lambda step, n: True)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))

    sim.iterate()

    assert (0.0, 0.0) not in model.predicate_neighbors


class _NeighborTypeModel(jps.CustomOperationalModel):
    """Records type and repr of the probe agent's neighbors."""

    def __init__(self):
        super().__init__()
        self.seen: list[tuple] = []

    def compute_next_state(self, state, step):
        if state.probe:
            self.seen = [
                (type(n), repr(n), step.no_geometry_between(n))
                for n in step.other_agents_in_range(5.0)
            ]
        return replace(state), _STAY_PUT


def test_neighbors_are_the_native_type():
    model = _NeighborTypeModel()
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))

    sim.iterate()

    [(kind, text, visible)] = model.seen
    assert kind is jps.NeighborView
    assert jps.NeighborView.__module__ == "jupedsim.py_jupedsim"
    assert text == "NeighborView(relative_position=(1.0, 0.0))"
    assert visible
    assert "A neighboring agent as seen from" in jps.NeighborView.__doc__
    assert "Vector from the querying agent" in (
        jps.NeighborView.relative_position.__doc__
    )


# ---------------------------------------------------------------------------
# no_geometry_between tests
# ---------------------------------------------------------------------------


def _visible(step, neighbor):
    return step.no_geometry_between(neighbor)


def test_no_geometry_between_filters_occluded_agents():
    """Agents behind the wall should be excluded by no_geometry_between."""
    model = _CapturingModel(radius=20.0, predicate=_visible)
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (5.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (5.0, 5.0))  # same side, visible
    _add_agent(sim, journey_id, exit_id, (15.0, 10.0))  # behind wall, occluded

    sim.iterate()

    assert (0.0, -5.0) in model.neighbors
    assert (10.0, 0.0) not in model.neighbors


def test_no_geometry_between_agent_above_wall_is_seen():
    """Agents whose connecting line stays above the wall top (y>15) are visible.

    Probe is at (5, 18) and target at (15, 18): the horizontal path y=18 never
    crosses the wall (which ends at y=15), so the target must be returned.
    """
    model = _CapturingModel(radius=20.0, predicate=_visible)
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(
        sim, journey_id, exit_id, (5.0, 18.0), probe=True
    )  # probe — above wall top
    _add_agent(
        sim, journey_id, exit_id, (15.0, 18.0)
    )  # far side, same height → visible

    sim.iterate()

    assert (10.0, 0.0) in model.neighbors


def test_composed_no_geometry_between_and_group_filter():
    """Combining no_geometry_between with a group predicate using a lambda."""
    model = _CapturingModel(
        radius=20.0,
        predicate=lambda step, n: _visible(step, n) and n.state.group == 1,
    )
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(
        sim, journey_id, exit_id, (5.0, 10.0), group=0, probe=True
    )  # probe
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

    assert model.neighbors == [(0.0, 5.0)]


# ---------------------------------------------------------------------------
# Wall views
# ---------------------------------------------------------------------------


class _WallCapturingModel(jps.CustomOperationalModel):
    """Records the walls the probe agent sees."""

    def __init__(self, *, distance=3.0):
        super().__init__()
        self._distance = distance
        self.walls: list = []

    def compute_next_state(self, state, step):
        if state.probe:
            self.walls = step.walls_in_range(self._distance)
        return replace(state), _STAY_PUT


def test_wall_view_carries_the_projection_onto_the_wall():
    model = _WallCapturingModel(distance=3.0)
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (8.0, 5.0), probe=True)

    sim.iterate()

    assert model.walls
    nearest = min(model.walls, key=lambda w: w.distance)

    # The near face of the wall sits at x = 9.9, so 1.9 to the agent's right.
    assert nearest.distance == pytest.approx(1.9)
    assert nearest.closest_point == pytest.approx((1.9, 0.0))
    # The normal points from the wall back at the agent, i.e. against +x here.
    assert nearest.normal == pytest.approx((-1.0, 0.0))
    # The segment is relative to the agent too: a vertical face at x = 1.9.
    assert nearest.segment[0][0] == pytest.approx(1.9)
    assert nearest.segment[1][0] == pytest.approx(1.9)


def test_walls_are_the_native_type():
    model = _WallCapturingModel(distance=3.0)
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (8.0, 5.0), probe=True)

    sim.iterate()

    nearest = min(model.walls, key=lambda w: w.distance)
    assert type(nearest) is jps.WallView
    assert jps.WallView.__module__ == "jupedsim.py_jupedsim"
    assert repr(nearest) == (
        f"WallView(distance={nearest.distance!r}, normal={nearest.normal!r})"
    )
    assert "A wall segment as seen from the agent" in jps.WallView.__doc__
    assert "Unit vector pointing from the wall" in jps.WallView.normal.__doc__


def test_wall_segment_is_clipped_to_the_queried_distance():
    model = _WallCapturingModel(distance=3.0)
    sim, exit_id, journey_id = _make_sim(model, geometry=_walled_room())
    _add_agent(sim, journey_id, exit_id, (8.0, 5.0), probe=True)

    sim.iterate()

    # The wall face runs from y = 0 to y = 15, but only the part within
    # 3.0 of the agent is handed out.
    nearest = min(model.walls, key=lambda w: w.distance)
    for x, y in nearest.segment:
        assert math.hypot(x, y) == pytest.approx(3.0)
    assert "within the queried distance" in jps.WallView.segment.__doc__


# ---------------------------------------------------------------------------
# View lifetime
# ---------------------------------------------------------------------------


class _StoringModel(jps.CustomOperationalModel):
    """Keeps every view it is handed, plus one derived from each step."""

    def __init__(self):
        super().__init__()
        self.views: list = []
        self.types: list = []

    def compute_next_state(self, state, step):
        self.types.append(
            (type(step), isinstance(step, jps.AgentView), step.dt > 0)
        )
        self.views.append(step)
        self.views.append(step.with_neighbor_state_mapping(lambda s: s))
        return replace(state), _STAY_PUT

    def check_model_constraint(self, state, view):
        self.types.append((type(view), True, True))
        self.views.append(view)


def test_views_are_the_native_types():
    model = _StoringModel()
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    sim.iterate()
    assert (jps.AgentView, True, True) in model.types  # check_model_constraint
    assert (jps.AgentStep, True, True) in model.types  # compute_next_state
    assert jps.AgentStep.__module__ == "jupedsim.py_jupedsim"
    assert issubclass(jps.AgentStep, jps.AgentView)
    assert "What an agent perceives" in jps.AgentView.__doc__
    assert "Duration of this simulation step" in jps.AgentStep.dt.__doc__


def test_views_cannot_be_used_after_the_callback():
    model = _StoringModel()
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    sim.iterate()
    assert len(model.views) >= 3
    for view in model.views:
        with pytest.raises(jps.SimulationError, match="only valid during"):
            view.walls_in_range(1.0)
        with pytest.raises(jps.SimulationError, match="only valid during"):
            view.other_agents_in_range(1.0)
    step = next(v for v in model.views if isinstance(v, jps.AgentStep))
    with pytest.raises(jps.SimulationError, match="only valid during"):
        step.dt


def test_a_raising_predicate_propagates():
    model = _CapturingModel(radius=5.0, predicate=lambda step, n: 1 / 0)
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))
    with pytest.raises(ZeroDivisionError):
        sim.iterate()


def test_predicate_results_are_used_by_truthiness():
    # The relative x of the neighbor straight above is 0.0 -> falsy -> dropped.
    model = _CapturingModel(
        radius=5.0, predicate=lambda step, n: n.relative_position[0]
    )
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0))
    _add_agent(sim, journey_id, exit_id, (2.0, 11.0))
    sim.iterate()
    assert model.neighbors == [(1.0, 0.0)]


def test_views_from_a_raising_callback_expire_too():
    class _RaisingModel(_StoringModel):
        def compute_next_state(self, state, step):
            self.views.append(step)
            raise RuntimeError("boom")

    model = _RaisingModel()
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    with pytest.raises(RuntimeError, match="boom"):
        sim.iterate()
    step = next(v for v in model.views if isinstance(v, jps.AgentStep))
    with pytest.raises(jps.SimulationError, match="only valid during"):
        step.route_orientation


class _NeighborStoringModel(jps.CustomOperationalModel):
    """Keeps the probe agent's neighbors, plain and seen through a mapping."""

    def __init__(self):
        super().__init__()
        self.neighbors: list = []
        self.mapped_groups: list = []

    def compute_next_state(self, state, step):
        if state.probe:
            self.neighbors = step.other_agents_in_range(5.0)
            # The derived step is a temporary: its neighbors must keep its
            # mapping alive.
            self.mapped_groups = [
                n.state.group
                for n in step.with_neighbor_state_mapping(
                    lambda s: replace(s, group=42)
                ).other_agents_in_range(5.0)
            ]
        return replace(state), _STAY_PUT


def test_neighbors_of_a_temporary_derived_view_see_the_mapping():
    model = _NeighborStoringModel()
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0), group=7)
    sim.iterate()
    assert model.mapped_groups == [42]


def test_neighbors_cannot_be_used_after_the_callback():
    model = _NeighborStoringModel()
    sim, exit_id, journey_id = _make_sim(model)
    _add_agent(sim, journey_id, exit_id, (2.0, 10.0), probe=True)
    _add_agent(sim, journey_id, exit_id, (3.0, 10.0), group=7)
    sim.iterate()
    [neighbor] = model.neighbors
    assert neighbor.relative_position == (1.0, 0.0)
    assert repr(neighbor) == "NeighborView(relative_position=(1.0, 0.0))"
    with pytest.raises(jps.SimulationError, match="only valid during"):
        neighbor.state
    del sim
    with pytest.raises(jps.SimulationError, match="only valid during"):
        neighbor.state
