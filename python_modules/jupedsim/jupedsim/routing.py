# SPDX-License-Identifier: LGPL-3.0-or-later

from jupedsim.pyjupedsim import Geometry, SurfaceMeshShortestPathRoutingEngine


class RoutingEngine:
    """RoutingEngine to compute the shortest paths with navigation meshes."""

    def __init__(self, geometry: Geometry) -> None:
        """Creates a RoutingEngine.

        Args:
            geometry: The walkable area, see
                :meth:`~jupedsim.WalkableSurface.create_geometry`.
        """
        self._geometry = geometry
        self._obj = SurfaceMeshShortestPathRoutingEngine(self._geometry)

    def compute_waypoints(
        self, frm: tuple[float, float], to: tuple[float, float]
    ) -> list[tuple[float, float]]:
        """Computes shortest path between specified points.

        Args:
            frm: point from which to find the shortest path
            to: point to which to find the shortest path

        Returns:
            List of points (path) from 'frm' to 'to' including from and to.
            Intermediate points may be collinear: the path is reported wherever
            it passes a triangle edge, not only where it turns.

        """
        path = self._obj.get_shortest_path(
            (frm[0], frm[1], 0.0), (to[0], to[1], 0.0)
        )
        return [(x, y) for x, y, _ in path]

    def is_routable(self, p: tuple[float, float]) -> bool:
        """Tests if the supplied point is inside the underlying geometry.

        Returns:
            If the point is inside the geometry.

        """
        return self._obj.is_valid_location((p[0], p[1], 0.0))

    def mesh(
        self,
    ) -> tuple[list[tuple[float, float]], list[list[int]]]:
        """Access the navigation mesh geometry.

        The navigation mesh is stored as a collection of triangles in CCW order.

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
        vertices = [(x, y) for x, y, _ in self._geometry._obj.vertices()]
        return vertices, self._geometry._obj.triangles()
