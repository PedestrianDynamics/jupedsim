# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Any

import shapely

import jupedsim.native as py_jps
from jupedsim.geometry import Geometry
from jupedsim.geometry_utils import build_geometry

# Alias RoutingEngine to the native one
RoutingEngine = py_jps.RoutingEngine


class DirectPathRoutingEngine(RoutingEngine):
    """Trivial routing engine that returns a straight line ignoring geometry.

    Useful as a reference implementation and for open-space scenarios where
    agents should walk directly towards their destination.
    """

    def name(self) -> str:
        return "DirectPath"

    def set_geometry(self, _) -> None:
        pass

    def compute_waypoints(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
    ) -> list[tuple[float, float]]:
        return [frm, to]

    def is_routable(self, _: tuple[float, float]) -> bool:
        return True


class AStarRoutingEngine(py_jps.AStarRoutingEngine):
    """A* shortest-path routing on a navigation mesh.

    Default-construct, then call :meth:`set_geometry` (or hand the engine to
    a :class:`Simulation`, which calls ``set_geometry`` automatically) before
    any routing query.

    The only Python-side behaviour added on top of the native binding is the
    accepted input types of :meth:`set_geometry`. All other methods come
    straight from the C++ binding.

    Note:
        Overriding methods of this class in further Python subclasses will
        *not* affect the routing decisions made by a :class:`Simulation`:
        the simulation calls the native C++ implementations directly. To
        implement custom routing in Python, subclass
        :class:`RoutingEngine` instead.
    """

    def set_geometry(
        self,
        geometry: (
            Geometry
            | str
            | shapely.GeometryCollection
            | shapely.Polygon
            | shapely.MultiPolygon
            | shapely.MultiPoint
            | list[tuple[float, float]]
        ),
        **kwargs: Any,
    ) -> None:
        """Bind this engine to *geometry*.

        Accepts a :class:`~jupedsim.Geometry`, a WKT string, a shapely
        geometry, or a list of points. Forwards to the native engine after
        converting to the internal geometry representation.

        Keyword Arguments:
            excluded_areas: forwarded to :func:`~jupedsim.build_geometry`
                when *geometry* is a list of points.
        """
        native = (
            geometry._obj
            if isinstance(geometry, Geometry)
            else build_geometry(geometry, **kwargs)._obj
        )
        super().set_geometry(native)
