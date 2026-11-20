# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

from collections.abc import Sequence
from typing import TYPE_CHECKING

import shapely

from jupedsim.pyjupedsim import Region

if TYPE_CHECKING:
    from jupedsim.simulation import Simulation

Area = shapely.Polygon | Sequence[tuple[float, float]]


class _TargetDescription:
    __slots__ = ("_areas",)

    def __init__(self, areas: Sequence[tuple[Area, Region]]) -> None:
        object.__setattr__(self, "_areas", tuple(tuple(a) for a in areas))

    @property
    def areas(self) -> tuple[tuple[Area, Region], ...]:
        """Pairs of an area and the region it lies in, as given."""
        return self._areas

    def __setattr__(self, name, value):
        raise AttributeError(f"{type(self).__name__} is immutable")

    def __repr__(self) -> str:
        return f"{type(self).__name__}({list(self._areas)!r})"


class Waypoint(_TargetDescription):
    """Describes a waypoint, to be added with :meth:`~jupedsim.Simulation.add_target`.

    An agent reaches the waypoint when it stands in any one of its areas.

    Args:
        areas: Pairs of an area and the region it lies in. An area is a
            :class:`~shapely.Polygon` without holes or a sequence of at least three
            ``(x, y)`` points. Only the part of an area inside its region belongs to
            the waypoint. Areas may lie in different regions.
    """

    __slots__ = ()


class Exit(_TargetDescription):
    """Describes an exit, to be added with :meth:`~jupedsim.Simulation.add_target`.

    Agents reaching an exit leave the simulation at the beginning of the next
    iteration.

    Args:
        areas: Pairs of an area and the region it lies in, as for
            :class:`Waypoint`.
    """

    __slots__ = ()


class RouteTarget:
    """Handle to a waypoint or an exit in a :class:`~jupedsim.Simulation`.

    Returned by :meth:`~jupedsim.Simulation.add_target` and the lookups of the
    simulation; never created directly. Handles are equal if they refer to the same
    target.
    """

    __slots__ = ("_simulation", "_id")

    def __init__(self, simulation: Simulation, target_id: int) -> None:
        """Do not use.

        Retrieve target handles from the simulation.
        """
        self._simulation = simulation
        self._id = target_id

    @property
    def id(self) -> int:
        """Numeric id of the target in its simulation."""
        return self._id

    @property
    def name(self) -> str | None:
        """Name of the target, if it was given one."""
        return self._simulation._obj.stage_name(self._id)

    @property
    def description(self) -> Waypoint | Exit:
        """The :class:`Waypoint` or :class:`Exit` the target was created from.

        Describes the input: not the part of an area clipped to its region, nor the
        floor field cells on which arrival is decided.
        """
        return self._simulation._target_descriptions[self._id]

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, RouteTarget)
            and other._simulation is self._simulation
            and other._id == self._id
        )

    def __hash__(self) -> int:
        return hash((id(self._simulation), self._id))

    def __repr__(self) -> str:
        name = self.name
        if name is None:
            return f"RouteTarget(id={self._id})"
        return f"RouteTarget(id={self._id}, name={name!r})"
