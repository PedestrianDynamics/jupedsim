# SPDX-License-Identifier: LGPL-3.0-or-later

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Any

import shapely

from jupedsim.pyjupedsim import SimulationError
from jupedsim.pyjupedsim import WalkableSurface as _NativeWalkableSurface


@dataclass(frozen=True)
class Location:
    """A point on a region: coordinates plus the region's identity.

    Create locations with :meth:`Region.at`. Equality and hash are by
    ``(x, y, region)``.
    """

    x: float
    y: float
    region: Region

    @property
    def xy(self) -> tuple[float, float]:
        return (self.x, self.y)

    def __repr__(self) -> str:
        return f"Location({self.x}, {self.y}, region={self.region.label})"


class Region:
    """A handle to a region of a :class:`WalkableSurface`.

    Equality and hash are by ``(surface, id)``.
    """

    def __init__(
        self,
        surface: WalkableSurface,
        id: int,
        name: str | None,
        height: float | None,
        polygon: shapely.Polygon | None = None,
        footprint: shapely.Polygon | None = None,
    ) -> None:
        self._surface = surface
        self._polygon = polygon
        # Where at() places points; a connector has a footprint only.
        self._footprint = polygon if footprint is None else footprint
        self._id = id
        self._name = name
        self._height = height

    @property
    def id(self) -> int:
        return self._id

    @property
    def name(self) -> str | None:
        return self._name

    @property
    def height(self) -> float | None:
        """Height of the region; ``None`` for connectors."""
        return self._height

    @property
    def label(self) -> str:
        return self._name if self._name is not None else f"Region({self._id})"

    def at(self, x: float, y: float) -> Location:
        """Return the location ``(x, y)`` in this region.

        Raises
        ------
        ValueError
            If the point is not inside this region.
        """
        if not self._surface._covers(self, x, y):
            raise ValueError(f"({x}, {y}) is not in {self.label}")
        return Location(float(x), float(y), self)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Region):
            return NotImplemented
        return self._surface is other._surface and self._id == other._id

    def __hash__(self) -> int:
        return hash((id(self._surface), self._id))

    def __repr__(self) -> str:
        parts = [f"id={self._id}"]
        if self._name is not None:
            parts.append(f'name="{self._name}"')
        if self._height is not None:
            parts.append(f"height={self._height}")
        return f"Region({', '.join(parts)})"


class WalkableSurface:
    """The walkable area, built from regions joined by connectors."""

    def __init__(self) -> None:
        self._obj = _NativeWalkableSurface()
        self._regions: list[Region] = []

    def add_region(
        self,
        *,
        polygon: shapely.Polygon,
        height: float = 0.0,
        name: str | None = None,
    ) -> Region:
        """Add a region at ``height`` and return its handle."""
        self._check_region_name(name)
        region_id = self._obj.add_region(polygon=polygon, height=height)
        return self._register(region_id, name, height, polygon)

    def connect_regions(
        self,
        *,
        from_region: Region,
        from_edge: Any,
        to_region: Region,
        to_edge: Any,
        name: str | None = None,
    ) -> Region:
        """Join an edge of ``from_region`` to an edge of ``to_region``."""
        for region in (from_region, to_region):
            if region._surface is not self:
                raise ValueError(f"{region.label} belongs to another surface")
            if region.height is None:
                raise ValueError(
                    f"{region.label} is a connector and cannot be connected "
                    "again; add a landing region between two connectors"
                )
        self._check_region_name(name)
        try:
            connector_id = self._obj.connect_regions(
                from_region=from_region.id,
                from_edge=from_edge,
                to_region=to_region.id,
                to_edge=to_edge,
            )
        except SimulationError as error:
            match = re.search(r"is not an edge of region (\d+)", str(error))
            if match is None:
                raise
            region = self._region_by_id(int(match.group(1)))
            edge = "from_edge" if region == from_region else "to_edge"
            raise ValueError(
                f"{edge} is not an edge of {region.label}'s boundary"
            ) from error
        # The corners native built from the regions' vertices, not the input.
        footprint = shapely.Polygon(self._obj.connector_corners(connector_id))
        return self._register(connector_id, name, None, footprint=footprint)

    @property
    def regions(self) -> tuple[Region, ...]:
        return tuple(self._regions)

    def region(
        self, name: str | None = None, *, id: int | None = None
    ) -> Region:
        """Look a region up by name or by id."""
        if (name is None) == (id is None):
            raise TypeError("give a name or an id, not both or neither")
        if id is not None:
            return self._region_by_id(id)
        for region in self._regions:
            if region.name == name:
                return region
        known = ", ".join(r.label for r in self._regions)
        raise KeyError(f"no region named {name}; regions are {known}")

    def regions_at(self, point: tuple[float, float]) -> tuple[Region, ...]:
        """Return every region covering ``point``, by ascending id."""
        x, y = point
        return tuple(r for r in self._regions if self._covers(r, x, y))

    def _region_by_id(self, id: int) -> Region:
        if not 0 <= id < len(self._regions):
            raise KeyError(f"no region with id {id}")
        return self._regions[id]

    def _check_region_name(self, name: str | None) -> None:
        if name is None:
            return
        for region in self._regions:
            if region.name == name:
                raise ValueError(
                    f"region name '{name}' is already used by {region.label}"
                )

    def _register(
        self,
        region_id: int,
        name: str | None,
        height: float | None,
        polygon: shapely.Polygon | None = None,
        footprint: shapely.Polygon | None = None,
    ) -> Region:
        region = Region(self, region_id, name, height, polygon, footprint)
        self._regions.append(region)
        return region

    def _covers(self, region: Region, x: float, y: float) -> bool:
        # A region's own geometry decides, so later, unconnected regions do
        # not matter; the native surface checks connectivity in Simulation().
        return region._footprint.covers(shapely.Point(x, y))
