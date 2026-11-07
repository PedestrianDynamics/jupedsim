# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

import numbers
from collections.abc import Sequence
from dataclasses import dataclass
from typing import TYPE_CHECKING, Any

import shapely

from jupedsim.convenience import circle_to_polygon
from jupedsim.surface import Region

if TYPE_CHECKING:
    from jupedsim.agent import Agent


class _Stage:
    """Base of the stage handles; equality and hash by (simulation, id)."""

    def __init__(self, simulation: Any, id: int, name: str | None) -> None:
        self._simulation = simulation
        self._id = id
        self._name = name

    @property
    def id(self) -> int:
        return self._id

    @property
    def name(self) -> str | None:
        return self._name

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, _Stage):
            return NotImplemented
        return self._simulation is other._simulation and self._id == other._id

    def __hash__(self) -> int:
        return hash((id(self._simulation), self._id))

    def __repr__(self) -> str:
        parts = [f"id={self._id}"]
        if self._name is not None:
            parts.append(f'name="{self._name}"')
        return f"{type(self).__name__}({', '.join(parts)})"


class Waypoint(_Stage):
    """A waypoint stage, created by :meth:`Simulation.add_waypoint`."""


class Exit(_Stage):
    """An exit stage, created by :meth:`Simulation.add_exit`."""

    @property
    def closed(self) -> bool:
        """Whether the exit is closed, read-only; change it with :meth:`close`."""
        return self._simulation._obj.stage_closed(self._id)

    @closed.setter
    def closed(self, value: Any) -> None:
        raise AttributeError(f"use {self._name or 'exit'}.close()")

    def close(self, *, informed: bool) -> tuple[Agent, ...]:
        """Close the exit for all routes; rules skip it as a target.

        Args:
            informed: If True, agents assigned to the exit by a rule are
                reassigned by that rule now; if False, they walk to the exit
                and are reassigned there.

        Returns:
            The agents stranded now, in the order the simulation reports
            them. Closing a closed exit does nothing and returns ``()``.
        """
        ids = self._simulation._obj.close_exit(self._id, informed)
        return tuple(self._simulation.agent(i) for i in ids)

    def open(self) -> tuple[Agent, ...]:
        """Open the exit again; stranded agents whose rule finds it resume.

        Returns:
            The agents that resumed. Opening an open exit does nothing and
            returns ``()``.
        """
        ids = self._simulation._obj.open_exit(self._id)
        return tuple(self._simulation.agent(i) for i in ids)


@dataclass(frozen=True)
class Removal:
    """An agent's removal, see :meth:`Simulation.removed_agents`."""

    id: int
    """Id of the removed agent."""
    exit: Exit | None
    """The exit the agent left through, None for ``agent.remove()``."""
    time: float
    """Simulation time in seconds at which the agent reached the exit, or at
    which ``agent.remove()`` was called."""


def _is_pair_list(area: Any) -> bool:
    return (
        isinstance(area, Sequence)
        and len(area) > 0
        and all(
            isinstance(item, Sequence)
            and len(item) == 2
            and isinstance(item[1], Region)
            for item in area
        )
    )


def _is_point(area: Any) -> bool:
    return (
        isinstance(area, Sequence)
        and len(area) == 2
        and all(isinstance(c, numbers.Real) for c in area)
    )


def _polygon(area: Any, method: str) -> shapely.Polygon:
    polygon = area if isinstance(area, shapely.Polygon) else None
    if polygon is None:
        polygon = shapely.Polygon(list(area))
    if polygon.interiors or not polygon.is_valid or polygon.is_empty:
        kind = "exit" if method == "add_exit" else "waypoint"
        raise ValueError(f"{kind} polygons must be simple and without holes")
    return polygon


def _check_region(
    region: Any, method: str, surface: Any, region_count: int
) -> Region:
    if region is None:
        raise TypeError(
            f"region= is required, e.g. sim.{method}(area, region=floor0)"
        )
    if not isinstance(region, Region):
        raise TypeError(
            'region must be a Region handle, e.g. surface.region("floor1")'
        )
    if surface is None or region._surface is not surface:
        raise ValueError(f"{region!r} belongs to another surface")
    if region.id >= region_count:
        raise ValueError(
            f"{region.label} is not part of this simulation's geometry"
        )
    return region


def _check_overlap(polygon: shapely.Polygon, region: Region) -> None:
    if region._polygon is None:
        return
    if shapely.intersection(polygon, region._polygon).area <= 0:
        raise ValueError(f"area does not overlap {region.label}")


def _single_area(
    area: Any, region: Region, radius: float | None, method: str
) -> list[tuple[shapely.Polygon, Region]]:
    if radius is not None and not _is_point(area):
        raise ValueError("radius applies to point waypoints only")
    if _is_point(area):
        if method == "add_exit":
            raise ValueError("add_exit takes an area, not a point")
        if radius is None:
            raise ValueError("a point waypoint needs radius=")
        if radius <= 0:
            raise ValueError("radius must be > 0")
        polygon = circle_to_polygon(center=tuple(area), radius=radius)
    else:
        polygon = _polygon(area, method)
    _check_overlap(polygon, region)
    return [(polygon, region)]


def build_areas(
    area: Any,
    *,
    region: Any,
    radius: float | None,
    method: str,
    surface: Any,
    region_count: int,
) -> list[tuple[shapely.Polygon, int]]:
    """Return the ``(polygon, region id)`` pairs of a stage, validated."""
    if isinstance(area, Sequence) and len(area) == 0:
        raise ValueError("no areas given")
    if _is_pair_list(area):
        if radius is not None:
            raise ValueError("radius applies to point waypoints only")
        if region is not None:
            raise ValueError(
                "region= applies to a single area; a list of pairs "
                "carries its regions"
            )
        pairs = [
            (
                _polygon(a, method),
                _check_region(r, method, surface, region_count),
            )
            for a, r in area
        ]
        for polygon, pair_region in pairs:
            _check_overlap(polygon, pair_region)
        return [(p, r.id) for p, r in pairs]
    checked = _check_region(region, method, surface, region_count)
    return [(p, r.id) for p, r in _single_area(area, checked, radius, method)]
