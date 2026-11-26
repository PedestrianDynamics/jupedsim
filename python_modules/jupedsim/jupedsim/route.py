# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from collections.abc import Iterable, Mapping
from types import MappingProxyType
from typing import TYPE_CHECKING, Final

from jupedsim.internal.messages import id_and_name
from jupedsim.pyjupedsim import (
    _FixedTransitionDescription,
    _LeastTargetedTransitionDescription,
    _RoundRobinTransitionDescription,
)
from jupedsim.target import RouteTarget

if TYPE_CHECKING:
    from jupedsim.simulation import Simulation


class _AgentLocation:
    __slots__ = ()

    def __repr__(self) -> str:
        return "AGENT_LOCATION"


AGENT_LOCATION: Final = _AgentLocation()
"""Source of the transition that chooses an agent's first target.

The location of the agent when it joins the route, in
:meth:`~jupedsim.Simulation.add_agent` or :meth:`~jupedsim.Agent.reroute`. It is not
a target, and no transition can lead to it.
"""

Source = RouteTarget | _AgentLocation


def _checked_source(source: object) -> Source:
    if not isinstance(source, RouteTarget | _AgentLocation):
        raise TypeError(
            "source must be a RouteTarget or jupedsim.AGENT_LOCATION, got "
            f"{type(source).__name__}"
        )
    return source


def _checked_target(target: object) -> RouteTarget:
    if isinstance(target, _AgentLocation):
        raise ValueError(
            "AGENT_LOCATION cannot be the target of a transition; it is where an "
            "agent joins the route"
        )
    if not isinstance(target, RouteTarget):
        raise TypeError(
            f"targets must be RouteTargets, got {type(target).__name__}"
        )
    return target


class FixedTransition:
    """Leads every agent reaching ``source`` to ``target``.

    Args:
        source: The target the transition leaves, or :data:`AGENT_LOCATION`.
        target: The target that follows.
    """

    __slots__ = ("_source", "_target")

    def __init__(self, source: Source, target: RouteTarget) -> None:
        self._source = _checked_source(source)
        self._target = _checked_target(target)

    @property
    def source(self) -> Source:
        return self._source

    @property
    def target(self) -> RouteTarget:
        return self._target

    def _description(self) -> _FixedTransitionDescription:
        return _FixedTransitionDescription(self._target.id)

    def _targets(self) -> list[RouteTarget]:
        return [self._target]

    def __repr__(self) -> str:
        return f"FixedTransition({self._source!r}, {self._target!r})"


class RoundRobinTransition:
    """Distributes the agents reaching ``source`` over targets by weight.

    The targets take turns in proportion to their weights, mixed as evenly as
    possible: with weights ``{a: 2, b: 1}`` the sequence is a, b, a, a, b, a,
    ... Only the ratio matters, ``{a: 60, b: 40}`` gives the same sequence as
    ``{a: 3, b: 2}``. The sequence is counted per route and source.

    Args:
        source: The target the transition leaves, or :data:`AGENT_LOCATION`.
        weights: Integer weight from 1 to 1000000000 of every target that can
            follow.
    """

    __slots__ = ("_source", "_weights")

    def __init__(
        self, source: Source, weights: Mapping[RouteTarget, int]
    ) -> None:
        self._source = _checked_source(source)
        if not weights:
            raise ValueError(
                "a round robin transition needs at least one target"
            )
        for target, weight in weights.items():
            _checked_target(target)
            if (
                isinstance(weight, bool)
                or not isinstance(weight, int)
                or not 1 <= weight <= 1_000_000_000
            ):
                raise ValueError(
                    "weights must be integers from 1 to 1000000000, got "
                    f"{weight!r} for target {id_and_name(target)}"
                )
        self._weights = dict(weights)

    @property
    def source(self) -> Source:
        return self._source

    @property
    def weights(self) -> Mapping[RouteTarget, int]:
        return MappingProxyType(self._weights)

    def _description(self) -> _RoundRobinTransitionDescription:
        return _RoundRobinTransitionDescription(
            [(target.id, weight) for target, weight in self._weights.items()]
        )

    def _targets(self) -> list[RouteTarget]:
        return list(self._weights)

    def __repr__(self) -> str:
        return f"RoundRobinTransition({self._source!r}, {self._weights!r})"


class LeastTargetedTransition:
    """Leads an agent reaching ``source`` to the target fewest agents head for.

    Agents are counted over all routes of the simulation, at the moment of the
    decision. On a tie, the earlier target in ``targets`` is chosen.

    Args:
        source: The target the transition leaves, or :data:`AGENT_LOCATION`.
        targets: The targets that can follow.
    """

    __slots__ = ("_source", "_candidates")

    def __init__(self, source: Source, targets: Iterable[RouteTarget]) -> None:
        self._source = _checked_source(source)
        self._candidates = tuple(_checked_target(t) for t in targets)
        if not self._candidates:
            raise ValueError(
                "a least targeted transition needs at least one target"
            )

    @property
    def source(self) -> Source:
        return self._source

    @property
    def targets(self) -> tuple[RouteTarget, ...]:
        return self._candidates

    def _description(self) -> _LeastTargetedTransitionDescription:
        return _LeastTargetedTransitionDescription(
            [target.id for target in self._candidates]
        )

    def _targets(self) -> list[RouteTarget]:
        return list(self._candidates)

    def __repr__(self) -> str:
        return f"LeastTargetedTransition({self._source!r}, {list(self._candidates)!r})"


Transition = FixedTransition | RoundRobinTransition | LeastTargetedTransition


class Route:
    """Handle to a route in a :class:`~jupedsim.Simulation`.

    Returned by :meth:`~jupedsim.Simulation.add_route` and the lookups of the
    simulation; never created directly. Handles are equal if they refer to the same
    route.
    """

    __slots__ = ("_simulation", "_id")

    def __init__(self, simulation: Simulation, route_id: int) -> None:
        """Do not use.

        Retrieve route handles from the simulation.
        """
        self._simulation = simulation
        self._id = route_id

    @property
    def id(self) -> int:
        """Numeric id of the route in its simulation."""
        return self._id

    @property
    def name(self) -> str | None:
        """Name of the route or ``None``."""
        return self._simulation._obj.route_name(self._id)

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, Route)
            and other._simulation is self._simulation
            and other._id == self._id
        )

    def __hash__(self) -> int:
        return hash((id(self._simulation), self._id))

    def __repr__(self) -> str:
        name = self.name
        if name is None:
            return f"Route(id={self._id})"
        return f"Route(id={self._id}, name={name!r})"
