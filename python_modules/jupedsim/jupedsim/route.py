# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from collections import Counter
from collections.abc import Iterable, Mapping, Sequence
from typing import TYPE_CHECKING, Any

from jupedsim.pyjupedsim import Rule as _NativeRule
from jupedsim.stages import Exit, _Stage

if TYPE_CHECKING:
    from jupedsim.agent import Agent


def _label(stage: _Stage) -> str:
    return stage.name if stage.name is not None else repr(stage)


def _require_stage(item: Any) -> _Stage:
    if isinstance(item, _Stage):
        return item
    raise TypeError(
        f"{item!r} is not a stage; use sim.add_waypoint({item!r}, ...)"
    )


def _labels(stages: Iterable[_Stage]) -> str:
    return ", ".join(_label(stage) for stage in stages)


class _Stay:
    """Terminal action: the agent stays in the arrival region of its stage."""

    def __repr__(self) -> str:
        return "STAY"


STAY = _Stay()


# The native rule's limit; keeps its running sums far from overflow.
MAX_WEIGHT = 1_000_000_000


def _check_weight(weight: Any) -> None:
    if isinstance(weight, bool) or not isinstance(weight, int):
        raise ValueError("weights must be integers")
    if weight < 0:
        raise ValueError("weights must be >= 0")
    if weight > MAX_WEIGHT:
        raise ValueError(f"weights must be <= {MAX_WEIGHT}")


class RoundRobin:
    """Choose the next stage by smooth weighted round-robin.

    Each decision adds every target's weight to its current weight, takes
    the target with the largest current weight (ties: the earlier one) and
    takes the sum of the weights off it. Only the ratio matters: weights
    3 and 2 give the same sequence as 60 and 40. Targets of weight 0 are
    never chosen. Weights are integers from 0 to 1000000000.
    """

    def __init__(self, weights: Mapping[_Stage, int]) -> None:
        for stage, weight in weights.items():
            _require_stage(stage)
            _check_weight(weight)
        if not any(weight > 0 for weight in weights.values()):
            raise ValueError("at least one weight must be > 0")
        self._weights = dict(weights)

    @property
    def weights(self) -> dict[_Stage, int]:
        return dict(self._weights)

    def _targets(self) -> list[_Stage]:
        return list(self._weights)

    def _live_targets(self) -> list[_Stage]:
        return [s for s, weight in self._weights.items() if weight > 0]

    def _native(self) -> _NativeRule:
        pairs = [(s.id, w) for s, w in self._weights.items()]
        return _NativeRule.create_round_robin_transition(pairs)

    def _describe(self) -> str:
        items = ", ".join(f"{_label(s)}: {w}" for s, w in self._weights.items())
        return f"RoundRobin({items})"

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, RoundRobin):
            return NotImplemented
        return self._weights == other._weights

    def __hash__(self) -> int:
        return hash(tuple(self._weights.items()))

    def __repr__(self) -> str:
        items = ", ".join(f"{_label(s)}: {w}" for s, w in self._weights.items())
        return f"RoundRobin({{{items}}})"


class _ChoiceRule:
    """Base of the rules that choose among a list of stages."""

    def __init__(self, stages: Sequence[_Stage]) -> None:
        self._stages = tuple(_require_stage(stage) for stage in stages)
        if not self._stages:
            raise ValueError(f"{type(self).__name__} needs at least one stage")

    @property
    def stages(self) -> tuple[_Stage, ...]:
        return self._stages

    def _targets(self) -> list[_Stage]:
        return list(self._stages)

    def _live_targets(self) -> list[_Stage]:
        return list(self._stages)

    def _describe(self) -> str:
        return repr(self)

    def __eq__(self, other: object) -> bool:
        if type(other) is not type(self):
            return NotImplemented
        return self._stages == other._stages

    def __hash__(self) -> int:
        return hash((type(self).__name__, self._stages))

    def __repr__(self) -> str:
        return f"{type(self).__name__}([{_labels(self._stages)}])"


class LeastTargeted(_ChoiceRule):
    """Choose the stage that the fewest agents are currently heading for."""

    def _native(self) -> _NativeRule:
        ids = [stage.id for stage in self._stages]
        return _NativeRule.create_least_targeted_transition(ids)


class LeastTravelTime(_ChoiceRule):
    """Choose the stage with the least travel time from the agent.

    Read from the stages' floor fields when the agent completes the stage;
    ties go to the earlier stage.
    """

    def _native(self) -> _NativeRule:
        ids = [stage.id for stage in self._stages]
        return _NativeRule.create_least_travel_time_transition(ids)


_RULES = (RoundRobin, LeastTargeted, LeastTravelTime)


def _is_step_pair(item: Any) -> bool:
    return isinstance(item, (tuple, list))


def _closes_loop(items: Sequence[Any], endless: bool) -> bool:
    return endless and len(items) > 1 and items[-1] == items[0]


def _linear_pairs(items: Sequence[Any], endless: bool) -> list[tuple]:
    if items[0] is STAY:
        raise ValueError("STAY must follow a stage")
    if STAY in items[:-1]:
        raise ValueError("STAY must be the last element")
    stages = [_require_stage(item) for item in items if item is not STAY]
    compared = stages[:-1] if _closes_loop(items, endless) else stages
    for stage, count in Counter(compared).items():
        if count > 1:
            raise ValueError(
                f"{_label(stage)} appears twice; only repeating the first "
                "stage at the end closes a loop"
            )
    return list(zip(items, items[1:]))


def _split_steps(
    steps: Sequence[Any], endless: bool
) -> tuple[list[tuple], list[Any], Any]:
    """Return the ``(from, to)`` pairs, the stages without a pair and the
    default start, which only a linear list has."""
    if len(steps) == 0:
        raise ValueError("route is empty")
    if not any(_is_step_pair(item) for item in steps):
        pairs = _linear_pairs(steps, endless)
        return pairs, list(steps) if len(steps) == 1 else [], steps[0]
    if not all(_is_step_pair(item) and len(item) == 2 for item in steps):
        raise ValueError(
            "steps must be either all stages or all (from, to) pairs"
        )
    return [tuple(item) for item in steps], [], None


def _check_pair_types(pairs: Sequence[tuple]) -> None:
    for source, target in pairs:
        _require_stage(source)
        if target is STAY or isinstance(target, _RULES):
            continue
        _require_stage(target)


def _describe_target(target: Any) -> str:
    if isinstance(target, _Stage):
        return _label(target)
    return target._describe() if hasattr(target, "_describe") else repr(target)


def _targets_of(target: Any) -> list[_Stage]:
    if isinstance(target, _Stage):
        return [target]
    return target._targets() if hasattr(target, "_targets") else []


def _order(pairs: Sequence[tuple], singles: Sequence[Any]) -> list[_Stage]:
    order: dict[_Stage, None] = {}
    for source, target in pairs:
        order[source] = None
        order.update(dict.fromkeys(_targets_of(target)))
    order.update(dict.fromkeys(singles))
    return list(order)


def _transitions(pairs: Sequence[tuple]) -> dict[_Stage, Any]:
    counts = Counter(source for source, _ in pairs)
    for source, target in pairs:
        if isinstance(source, Exit):
            raise ValueError(
                f"exits are terminal; remove "
                f"({_label(source)}, {_describe_target(target)})"
            )
        if counts[source] > 1:
            raise ValueError(
                f"{_label(source)} has {counts[source]} outgoing "
                "transitions; combine them into one rule"
            )
    return dict(pairs)


def _common_simulation(stages: Sequence[_Stage]) -> Any:
    simulation = stages[0]._simulation
    if any(stage._simulation is not simulation for stage in stages):
        raise ValueError("all stages must belong to the same simulation")
    return simulation


def _check_next_stages(
    order: Sequence[_Stage], transitions: Mapping[_Stage, Any]
) -> None:
    for stage in order:
        if isinstance(stage, Exit) or stage in transitions:
            continue
        raise ValueError(
            f"{_label(stage)} has no next stage; end with an exit or add "
            f"({_label(stage)}, jps.STAY)"
        )


def _is_terminal(stage: _Stage, transitions: Mapping[_Stage, Any]) -> bool:
    return isinstance(stage, Exit) or transitions.get(stage) is STAY


def _successors(target: Any) -> list[_Stage]:
    if isinstance(target, _Stage):
        return [target]
    return target._live_targets() if hasattr(target, "_live_targets") else []


def _stuck_stages(
    order: Sequence[_Stage], transitions: Mapping[_Stage, Any]
) -> list[_Stage]:
    """Return the stages, in route order, from which no exit or STAY can be
    reached through weights > 0."""
    done = {s for s in order if _is_terminal(s, transitions)}
    changed = True
    while changed:
        changed = False
        for stage in order:
            reaches = done.intersection(_successors(transitions.get(stage)))
            if stage not in done and reaches:
                done.add(stage)
                changed = True
    return [s for s in order if s not in done]


def _check_ending(
    order: Sequence[_Stage], transitions: Mapping[_Stage, Any], endless: bool
) -> None:
    terminals = [s for s in order if _is_terminal(s, transitions)]
    if endless and terminals:
        raise ValueError(
            f"route can end at {_label(terminals[0])}; drop endless=True"
        )
    if endless:
        return
    if _stuck_stages(order, transitions):
        raise ValueError(
            "no exit or STAY reachable; to loop on purpose, use endless=True"
        )


def _check_supported(transitions: Mapping[_Stage, Any]) -> None:
    for target in transitions.values():
        if target is STAY:
            raise NotImplementedError("STAY is not supported yet")


def _native_rule(stage: _Stage, transitions: Mapping[_Stage, Any]) -> Any:
    if stage not in transitions:
        return _NativeRule.create_none_transition()
    target = transitions[stage]
    if isinstance(target, _Stage):
        return _NativeRule.create_fixed_transition(target.id)
    return target._native()


def _patched(
    label: str, current: Mapping[_Stage, int], patch: Mapping[_Stage, int]
) -> dict[_Stage, int]:
    """Return ``current`` updated by ``patch``, checked as weights of the
    rule at the stage ``label``."""
    for target, weight in patch.items():
        if _require_stage(target) not in current:
            raise ValueError(
                f"{_label(target)} is not a target of {label}; targets are "
                f"{_labels(current)} (see route.weights({label}))"
            )
        _check_weight(weight)
    proposed = {**current, **patch}
    if not any(weight > 0 for weight in proposed.values()):
        raise ValueError(
            f"at least one target at {label} needs weight > 0; "
            f"to stop agents, use route.hold({label})"
        )
    return proposed


class Route:
    """The order in which an agent visits stages.

    A route is validated at construction and bound to the simulation that
    owns its stages. ``steps`` is a list of stages, walked in order, or a
    list of ``(from, to)`` pairs where ``to`` is a stage or a rule.
    """

    def __init__(
        self,
        steps: Sequence[Any],
        *,
        start: _Stage | None = None,
        endless: bool = False,
    ) -> None:
        pairs, singles, default_start = _split_steps(steps, endless)
        _check_pair_types(pairs)
        order = _order(pairs, singles)
        simulation = _common_simulation(order)
        transitions = _transitions(pairs)
        _check_next_stages(order, transitions)
        start = default_start if start is None else start
        if start is not None and start not in order:
            raise ValueError(f"{_label(start)} is not part of this route")
        _check_ending(order, transitions, endless)
        _check_supported(transitions)
        self._simulation = simulation
        self._stages = tuple(order)
        self._transitions = transitions
        self._start = start
        self._endless = endless
        native = {s.id: _native_rule(s, transitions) for s in order}
        self._native_id = simulation._obj.add_route(native)
        simulation._routes[self._native_id] = self

    @property
    def stages(self) -> tuple[_Stage, ...]:
        """The stages of the route in order of first appearance."""
        return self._stages

    @property
    def start(self) -> _Stage | None:
        """The default start stage, or None if the route has none."""
        return self._start

    def next(self, stage: _Stage) -> Any:
        """Return the stage or rule that follows ``stage``, None at exits."""
        self._check_member(stage)
        return self._transitions.get(stage)

    def weights(self, stage: _Stage) -> dict[_Stage, int]:
        """Return a copy of the weights of the rule at ``stage``."""
        label = _label(stage)
        fixed = f"{label} has a fixed transition; use route.next({label})"
        return self._round_robin(stage, fixed).weights

    def update_weights(
        self, stage: _Stage, weights: Mapping[_Stage, int]
    ) -> None:
        """Change some weights of the round-robin rule at ``stage``.

        A patch, like ``dict.update``: targets not named keep their weight.
        The new weights are checked against the whole route first; on error
        nothing changes. On success the rule's sequence starts afresh. Takes
        effect at the next decision: agents already assigned keep their
        target, and stranded agents decide again when an exit of their rule
        opens.

        Raises:
            ValueError: If ``stage`` has no round-robin rule, a key is not
                one of its targets, a weight is not a non-negative integer,
                no weight would be > 0, or with these weights no exit or
                STAY would be reachable from some stage.
        """
        label = _label(stage)
        fixed = f"{label} has a fixed transition, not a weighted rule"
        current = self._round_robin(stage, fixed).weights
        proposed = _patched(label, current, weights)
        transitions = {**self._transitions, stage: RoundRobin(proposed)}
        self._check_reachable(transitions)
        self._simulation._obj.set_round_robin_weights(
            route_id=self._native_id,
            stage_id=stage.id,
            weights={target.id: weight for target, weight in proposed.items()},
        )
        self._transitions = transitions

    def hold(self, stage: _Stage) -> None:
        """Stop agents of this route at ``stage`` until :meth:`release`.

        Every agent on this route whose target is ``stage``, now or later,
        walks on to it and stops where it enters it (``agent.held``).
        Agents on other routes and agents that completed ``stage`` before
        the call are not affected. Weights are kept. A repeated call does
        nothing.

        Raises:
            ValueError: If ``stage`` is not part of this route or is an
                exit.
        """
        self._check_member(stage)
        if isinstance(stage, Exit):
            raise ValueError(
                f"exits cannot be held; use {_label(stage)}.close()"
            )
        self._simulation._obj.hold(route_id=self._native_id, stage_id=stage.id)

    def release(self, stage: _Stage) -> None:
        """Lift the hold at ``stage``.

        The held agents go on in arrival order (ties by agent id), each
        sent on by the current rule at ``stage``; an agent whose rule has
        no open target is stranded inside ``stage``. Agents still walking
        to ``stage`` pass it without stopping.

        Raises:
            ValueError: If ``stage`` is not part of this route or not held.
        """
        if not self.is_held(stage):
            raise ValueError(f"{_label(stage)} is not held")
        self._simulation._obj.release(
            route_id=self._native_id, stage_id=stage.id
        )

    def is_held(self, stage: _Stage) -> bool:
        """True between :meth:`hold` and :meth:`release` of ``stage``."""
        self._check_member(stage)
        return self._simulation._obj.is_held(
            route_id=self._native_id, stage_id=stage.id
        )

    def held_at(self, stage: _Stage) -> tuple[Agent, ...]:
        """The agents of this route stopped at ``stage``, in arrival order.

        Agents still walking to a held ``stage`` are not included.
        """
        self._check_member(stage)
        ids = self._simulation._obj.held_agents(
            route_id=self._native_id, stage_id=stage.id
        )
        return tuple(self._simulation.agent(i) for i in ids)

    def _round_robin(self, stage: _Stage, fixed: str) -> RoundRobin:
        """Return the round-robin rule at ``stage``; raise with ``fixed`` for
        a fixed transition."""
        self._check_member(stage)
        rule = self._transitions.get(stage)
        label = _label(stage)
        if isinstance(rule, RoundRobin):
            return rule
        if isinstance(rule, _ChoiceRule):
            raise ValueError(
                f"{label} uses {type(rule).__name__}, which has no weights"
            )
        if isinstance(stage, Exit):
            raise ValueError(f"{label} is an exit; it has no weights")
        raise ValueError(fixed)

    def _check_reachable(self, transitions: Mapping[_Stage, Any]) -> None:
        if self._endless:
            return
        stuck = _stuck_stages(self._stages, transitions)
        if stuck:
            raise ValueError(
                f"with these weights no exit or STAY is reachable from "
                f"{_labels(stuck)}; weights unchanged"
            )

    def _check_member(self, stage: _Stage) -> None:
        if stage in self._stages:
            return
        raise ValueError(
            f"{_label(stage)} is not part of this route; "
            f"stages are {_labels(self._stages)}"
        )

    def _check_simulation(self, simulation: Any) -> None:
        if simulation is not self._simulation:
            raise ValueError("route belongs to another simulation")

    def __repr__(self) -> str:
        start = _label(self._start) if self._start is not None else None
        lines = [f"Route(start={start})"]
        for stage in self._stages:
            if stage in self._transitions:
                target = _describe_target(self._transitions[stage])
                lines.append(f"  {_label(stage)} -> {target}")
        return "\n".join(lines)
