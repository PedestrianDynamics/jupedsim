# SPDX-License-Identifier: LGPL-3.0-or-later

import abc
from typing import Any

import shapely

import jupedsim.native as py_jps
from jupedsim.geometry_utils import build_geometry


class RoutingEngine(abc.ABC):
    """Abstract base class for Python-implemented routing engines.

    Subclass this to prototype a new routing algorithm in Python.  Once the
    algorithm is proven correct it can be ported to C++ for production
    performance.

    All methods that accept or return points use ``(x, y)`` tuples.
    """

    @abc.abstractmethod
    def name(self) -> str:
        """Short identifier returned by :attr:`~jupedsim.Simulation.routing_engine_name`."""

    @abc.abstractmethod
    def set_geometry(self, geometry) -> None:
        """Called when the simulation geometry changes.

        Arguments:
            geometry: A :class:`~jupedsim.native.Geometry` instance describing
                the updated walkable area.
        """

    @abc.abstractmethod
    def compute_waypoints(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
    ) -> list[tuple[float, float]]:
        """Return the full path from *frm* to *to* as a sequence of waypoints.

        The returned list must include both *frm* and *to*.
        """

    @abc.abstractmethod
    def is_routable(self, p: tuple[float, float]) -> bool:
        """Return ``True`` if *p* lies inside the walkable area."""

    @abc.abstractmethod
    def clone(self) -> "RoutingEngine":
        """Return an independent copy of this engine."""


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

    def clone(self) -> "DirectPathRoutingEngine":
        return DirectPathRoutingEngine()

    @staticmethod
    def factory() -> py_jps.RoutingEngineFactory:
        """Return a factory that installs this engine in a simulation.

        Pass the returned object to :meth:`~jupedsim.Simulation.switch_routing_engine`.
        """
        return py_jps.python_routing_factory(
            lambda _: DirectPathRoutingEngine()
        )


class AStarRoutingEngine:
    """AStarRoutingEngine to compute the shortest paths with navigation meshes."""

    def __init__(
        self,
        geometry: (
            str
            | shapely.GeometryCollection
            | shapely.Polygon
            | shapely.MultiPolygon
            | shapely.MultiPoint
            | list[tuple[float, float]]
        ),
        **kwargs: Any,
    ) -> None:
        self._obj = py_jps.AStarRoutingEngine(
            build_geometry(geometry, **kwargs)._obj
        )

    @property
    def id(self) -> int:
        """Unique identifier of this routing engine instance."""
        return self._obj.id

    @property
    def name(self) -> str:
        """Name of the routing engine implementation."""
        return self._obj.name

    def compute_waypoints(
        self, frm: tuple[float, float], to: tuple[float, float]
    ) -> list[tuple[float, float]]:
        """Computes shortest path between specified points.

        Arguments:
            geometry: Data to create the geometry out of. Data may be supplied as:

                * list of 2d points describing the outer boundary, holes may be added with use of `excluded_areas` kw-argument

                * :class:`~shapely.GeometryCollection` consisting only out of :class:`Polygons <shapely.Polygon>`, :class:`MultiPolygons <shapely.MultiPolygon>` and :class:`MultiPoints <shapely.MultiPoint>`

                * :class:`~shapely.MultiPolygon`

                * :class:`~shapely.Polygon`

                * :class:`~shapely.MultiPoint` forming a "simple" polygon when points are interpreted as linear ring without repetition of the start/end point.

                * str with a valid Well Known Text. In this format the same WKT types as mentioned for the shapely types are supported: GEOMETRYCOLLETION, MULTIPOLYGON, POLYGON, MULTIPOINT. The same restrictions as mentioned for the shapely types apply.

            frm: point from which to find the shortest path
            to: point to which to find the shortest path

        Keyword Arguments:
            excluded_areas: describes exclusions
                from the walkable area. Only use this argument if `geometry` was
                provided as list[tuple[float, float]].

        Returns:
            List of points (path) from 'frm' to 'to' including from and to.

        """
        return self._obj.compute_waypoints(frm, to)

    def is_routable(self, p: tuple[float, float]) -> bool:
        """Tests if the supplied point is inside the underlying geometry.

        Returns:
            If the point is inside the geometry.

        """
        return self._obj.is_routable(p)

    def mesh(
        self,
    ) -> tuple[list[tuple[float, float]], list[list[int]]]:
        """Access the navigation mesh geometry.

        The navigation mesh is store as a collection of convex polygons in CCW order.

        The returned data is to be interpreted as:

        .. code::

            tuple[
                list[tuple[float, float]], # All vertices in this mesh.
                list[ # List of polygons
                    list[int] # List of indices into the vertices that compose this polygon in CCW order
                ]
            ]

        Returns:
            A tuple of vertices and list of polygons which in turn are a list of indices
            tuple[list[tuple[float, float]],list[list[int]]]
        """
        return self._obj.mesh()

    def edges_for(self, vertex_id: int):
        return self._obj.edges_for(vertex_id)

    @staticmethod
    def factory() -> py_jps.RoutingEngineFactory:
        """Return a factory that installs this engine in a simulation.

        Pass the returned object to :meth:`~jupedsim.Simulation.switch_routing_engine`.
        """
        return py_jps.astar_routing_factory()
